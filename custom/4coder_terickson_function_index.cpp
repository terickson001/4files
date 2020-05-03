#define EXT_FUNCTION_INDEX SCu8("Function Index")

struct Function_Parameter
{
    String_Const_u8 prefix;
    String_Const_u8 type;
    String_Const_u8 postfix;
    String_Const_u8 name;
    String_Const_u8 default_value;
    
    Function_Parameter *next;
};

struct Function_Parameter_List
{
    Function_Parameter *first;
    Function_Parameter *last;
    
    u32 count;
};

struct Language_Function_Indexer;
struct Function_Index
{
    Code_Index_Note *note;
    String_Const_u8 name;
    Function_Parameter_List parameters;
    Language_Function_Indexer *indexer;
};

typedef Table_Data_Data Function_Index_Table;


struct Function_Index_Preview
{
    Function_Index *index;
    Buffer_ID buffer;
    Managed_Object start_marker;
    Managed_Object end_marker;
    i64 pos;
    List_String_Const_u8 params;
};

// global Function_Index_Table global_indexed_functions;
global Function_Index_Preview current_function_preview = {};


struct Language_Function_Delimiters // Token Kinds
{
    u64 open;
    String_Const_u8 open_str;
    u64 close;
    String_Const_u8 close_str;
    u64 parameter;
    String_Const_u8 parameter_str;
};

struct Language_Function_Indexer
{
    Function_Index *(*parse_function)(Application_Links *app, Code_Index_Note *note, Arena *arena);
    List_String_Const_u8 (*parameter_strings)(Function_Index *index, Arena *arena);
    Language_Function_Delimiters delims;
};

Function_Index *function_index_from_token(Application_Links *app, Arena *arena, Buffer_ID buffer, Token token, Language_Function_Indexer *indexer)
{
    String_Const_u8 ident = push_buffer_range(app, arena, buffer, Ii64(&token));
    
    for (Buffer_ID buf = get_buffer_next(app, 0, Access_Always);
         buf != 0;
         buf = get_buffer_next(app, buf, Access_Always))
    {
        Code_Index_File *file = code_index_get_file(buf);
        if (file == 0)
            continue;
        
        for (i32 i = 0; i < file->note_array.count; i++)
        {
            Code_Index_Note *note = file->note_array.ptrs[i];
            
            if (note->note_kind == CodeIndexNote_Function &&
                string_match(note->text, ident, StringMatch_Exact))
            {
                Function_Index *index = indexer->parse_function(app, note, arena);
                index->indexer = indexer;
                return index;
            }
        }
    }
    
    return 0;
}

function void function_index_render_preview(Application_Links *app, Buffer_ID buffer, View_ID view, Text_Layout_ID text_layout_id)
{
    if (!current_function_preview.index)
        return; // No preview to show
    if (current_function_preview.buffer != buffer)
        return; // Wrong Buffer
    
    i64 cursor = view_get_cursor_pos(app, view);
    i64 cursor_line = get_line_number_from_pos(app, buffer, cursor);
    i64 cursor_col = get_column_from_pos(app, buffer, cursor);
    Marker *start_marker = (Marker *)managed_object_get_pointer(app, current_function_preview.start_marker);
    Marker *end_marker = (Marker *)managed_object_get_pointer(app, current_function_preview.end_marker);
    Code_Index_File *code_index = code_index_get_file(buffer);
    Code_Index_Nest *start_nest = code_index_get_nest(code_index, start_marker->pos);
    
    // @todo(tyler): Handle multi-line parameters (Check Scopes?)
    if (cursor_line != get_line_number_from_pos(app, buffer, start_marker->pos))
    {
        current_function_preview = {0};
        return; // Cancelled
    }
    if (cursor < start_marker->pos)
    {
        current_function_preview = {0};
        return; // Cancelled
    }
    
    
    Scratch_Block scratch(app);
    
    if (cursor > end_marker->pos)
        end_marker->pos = cursor;
    Language_Function_Delimiters delims = current_function_preview.index->indexer->delims;
    Range_i64 written_range = Ii64(start_marker->pos, end_marker->pos);
    String_Const_u8 written = push_buffer_range(app, scratch, buffer, written_range);
    u64 close_idx = 0;
    close_idx = string_find_first(written, delims.close_str, StringMatch_Exact);
    if (close_idx != written.size)
    {
        Code_Index_Nest *close_nest = code_index_get_nest(code_index, written_range.min + close_idx);
        if (close_nest != start_nest)
        {
            current_function_preview = {0};
            return; // Finished
        }
    }
    u64 param_idx = 0;
    u32 curr_param = 0;
    while (written.size)
    {
        param_idx = string_find_first(written, delims.parameter_str, StringMatch_Exact);
        if (param_idx == written.size) break;
        written = string_skip(written, param_idx+1);
        Code_Index_Nest *delim_nest = code_index_get_nest(code_index, written_range.min + param_idx);
        if (delim_nest != start_nest) continue;
        print_message(app, push_stringf(scratch, "(%d:%d) -- (%d:%d)\n",
                                        start_nest->open.max, start_nest->close.min,
                                        delim_nest->open.max, delim_nest->close.min)
                      );
        curr_param++;
    }
    
    
    List_String_Const_u8 indexed_params = current_function_preview.params;
    for (int i = 0; i < curr_param; i++)
    {
        if (!indexed_params.first)
            break;
        
        indexed_params.node_count -= 1;
        indexed_params.total_size -= indexed_params.first->string.size;
        indexed_params.first = indexed_params.first->next;
    }
    if (indexed_params.node_count == 0)
    {
        current_function_preview = {0};
        return; // Out of params
    }
    
    String_Const_u8 param_delim_space = push_stringf(scratch, "%.*s ", string_expand(delims.parameter_str));
    String_Const_u8 flattened = string_list_flatten(scratch, indexed_params, param_delim_space, 0, 0);
    String_Const_u8 parameters = push_stringf(scratch, "%.*s%.*s",
                                              string_expand(flattened),
                                              string_expand(delims.close_str)
                                              );
    
    Face_ID face_id = get_face_id(app, buffer);
    Face_Metrics metrics = get_face_metrics(app, face_id);
    
    Rect_f32 start_rect = text_layout_character_on_screen(app, text_layout_id, end_marker->pos);
    
    u32 color = finalize_color(defcolor_comment, 0);
    color &= 0x00ffffff;
    color |= 0x80000000;
    Vec2_f32 draw_pos =
    {
        start_rect.x0 + metrics.space_advance,
        start_rect.y0,
    };
    
    Rect_f32 debug_rect = text_layout_character_on_screen(app, text_layout_id, start_marker->pos);
    draw_rectangle(app, debug_rect, 0.1, finalize_color(defcolor_comment, 0));
    draw_string(app, face_id, parameters, draw_pos, color);
    
}

CUSTOM_COMMAND_SIG(preview_function_parameters)
CUSTOM_DOC("Shows a preview of the written function's parameter list")
{
    Scratch_Block scratch(app);
    
    View_ID view = get_active_view(app, Access_Always);
    i64 cursor = view_get_cursor_pos(app, view);
    Buffer_ID buffer = view_get_buffer(app, view, Access_Always);
    Managed_Scope scope = buffer_get_managed_scope(app, buffer);
    Token_Array *tokens = scope_attachment(app, scope, attachment_tokens, Token_Array);
    
    Language *language = *buffer_get_language(app, buffer);
    Extension_Support *ext_support = language_get_extension(language, EXT_FUNCTION_INDEX);
    Language_Function_Indexer *lang_indexer;
    
    if (!ext_support)
        return;
    
    lang_indexer = (Language_Function_Indexer *)ext_support->ext_interface.data;
    
    if (!lang_indexer)
        return;
    
    i64 token_index = token_index_from_pos(tokens, cursor);
    while (token_index &&
           (tokens->tokens[token_index].kind == TokenBaseKind_Whitespace ||
            tokens->tokens[token_index].kind == TokenBaseKind_EOF ||
            tokens->tokens[token_index].kind == TokenBaseKind_Comment))
        token_index--;
    
    Function_Index *index;
    i64 token_start = token_index;
    b32 is_open = false;
    if (tokens->tokens[token_index].kind == TokenBaseKind_Identifier)
    {
        if (!(index = function_index_from_token(app, scratch, buffer, tokens->tokens[token_index], lang_indexer)))
            return;
        
    }
    else if (tokens->tokens[token_index].sub_kind == lang_indexer->delims.open)
    {
        is_open = true;
        while (token_index &&
               (tokens->tokens[token_index].kind == TokenBaseKind_Whitespace ||
                tokens->tokens[token_index].kind == TokenBaseKind_EOF ||
                tokens->tokens[token_index].kind == TokenBaseKind_Comment))
            token_index--;
        
        if (tokens->tokens[token_index].kind != TokenBaseKind_Identifier)
            return;
        
        if (!(index = function_index_from_token(app, scratch, buffer, tokens->tokens[token_index], lang_indexer)))
            return;
    }
    
    if (current_function_preview.index && string_match(index->name, current_function_preview.index->name))
    {
        current_function_preview = {0};
        return;
    }
    
    current_function_preview.params = lang_indexer->parameter_strings(index, tc_global_arena);
    current_function_preview.index = push_array(tc_global_arena, Function_Index, 1);
    *current_function_preview.index = *index;
    current_function_preview.start_marker = alloc_buffer_markers_on_buffer(app, buffer, 1, 0);
    current_function_preview.end_marker = alloc_buffer_markers_on_buffer(app, buffer, 1, 0);
    Marker marker = {Ii64(&tokens->tokens[token_start]).max, false};
    managed_object_store_data(app, current_function_preview.start_marker, 0, 1, &marker);
    managed_object_store_data(app, current_function_preview.end_marker, 0, 1, &marker);
    
    current_function_preview.buffer = buffer;
}