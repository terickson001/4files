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

    Function_Index *next;
};

struct Function_Index_List
{
    Function_Index *first;
    Function_Index *last;

    u32 count;
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
    Language *lang;
    Function_Index *(*parse_function)(Application_Links *app, Code_Index_Note *note, Arena *arena);
    List_String_Const_u8 (*parameter_strings)(Function_Index *index, Arena *arena);
    Language_Function_Delimiters delims;
};

struct Function_Index_Menu
{
    Function_Index_List indices;
    Render_Caller_Function *prev_render_caller;
};

Function_Index_List function_indices_from_token(Application_Links *app, Arena *arena, Buffer_ID buffer, Token token, Language_Function_Indexer *indexer)
{
    Function_Index_List indices = {0};
    String_Const_u8 ident = push_buffer_range(app, arena, buffer, Ii64(&token));
    Data str_data = *(Data *)&ident;

    for (Buffer_ID buf = get_buffer_next(app, 0, Access_Always);
         buf != 0;
         buf = get_buffer_next(app, buf, Access_Always))
    {
        if (*buffer_get_language(app, buf) != *buffer_get_language(app, buffer)) continue;
        Code_Index_Table *code_index = get_code_index_table(&code_index_tables, buf);
        if (code_index == 0)
            continue;

        // Table_Lookup lookup = table_lookup(&code_index->notes, str_data);
        Code_Index_Note_List **list_ref = index_map_get(&code_index->notes, hash_crc64(ident.str, ident.size));
        // b32 res = table_read(&code_index->notes, lookup, (u64 *)&list);
        if (!list_ref) continue;
        Code_Index_Note_List *list = *list_ref;
        for (Code_Index_Note *note = list->first; note; note = note->next)
        {
            if (note->note_kind == CodeIndexNote_Function &&
                string_match(note->text, ident, StringMatch_Exact))
            {
                Function_Index *index = indexer->parse_function(app, note, arena);
                if (!index) continue;
                index->indexer = indexer;
                sll_queue_push(indices.first, indices.last, index);
                indices.count++;
            }
        }
    }
    return indices;
}

function void function_index_render_preview(Application_Links *app, Buffer_ID buffer, View_ID view, Text_Layout_ID text_layout_id)
{
    if (!current_function_preview.index)
        return; // No preview to show
    if (current_function_preview.buffer != buffer)
        return; // Wrong Buffer

    i64 cursor = view_get_cursor_pos(app, view);
    //     i64 cursor_line = get_line_number_from_pos(app, buffer, cursor);
    //     i64 cursor_col = get_column_from_pos(app, buffer, cursor);
    Marker *start_marker = (Marker *)managed_object_get_pointer(app, current_function_preview.start_marker);
    Marker *end_marker = (Marker *)managed_object_get_pointer(app, current_function_preview.end_marker);
    Code_Index_File *code_index = code_index_get_file(buffer);
    Code_Index_Nest *start_nest = code_index_get_nest(code_index, start_marker->pos);
    Code_Index_Nest *cursor_nest = code_index_get_nest(code_index, cursor);

    if (!start_nest) return;
    if (cursor_nest != start_nest)
    {
        if (!cursor_nest)
        {
            current_function_preview = {0};
            return; // Cancelled
        }
        Code_Index_Nest *parent = cursor_nest->parent;
        while (parent)
        {
            if (parent == start_nest)
                break;
            parent = parent->parent;
        }
        if (!parent)
        {
            current_function_preview = {0};
            return; // Cancelled
        }
    }
    if (start_nest->is_closed)
    {
        current_function_preview = {0};
        return; // Parameters are closed
    }

    Scratch_Block scratch(app);

    if (cursor > end_marker->pos)
        end_marker->pos = cursor;
    Language_Function_Delimiters delims = current_function_preview.index->indexer->delims;
    Range_i64 written_range = Ii64(start_marker->pos, end_marker->pos);
    String_Const_u8 written = push_buffer_range(app, scratch, buffer, written_range);

    u64 param_idx = 0;
    i32 curr_param = 0;
    while (written.size)
    {
        param_idx = string_find_first(written, delims.parameter_str, StringMatch_Exact);
        if (param_idx == written.size) break;
        Code_Index_Nest *delim_nest = code_index_get_nest(code_index, written_range.max-written.size + param_idx);
        written = string_skip(written, param_idx+1);
        if (delim_nest != start_nest) continue;
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

    // pop first parameter
    String_Const_u8 active_param = indexed_params.first->string;
    indexed_params.first = indexed_params.first->next;
    indexed_params.node_count -= 1;
    indexed_params.total_size -= active_param.size;

    Face_ID face_id = get_face_id(app, buffer);
    Face_Metrics metrics = get_face_metrics(app, face_id);

    Rect_f32 start_rect = text_layout_character_on_screen(app, text_layout_id, end_marker->pos);

    u32 color = finalize_color(defcolor_comment, 0);

    Vec2_f32 draw_pos =
    {
        start_rect.x0 + metrics.space_advance,
        start_rect.y0,
    };

    draw_pos = draw_string(app, face_id, active_param, draw_pos, color);

    color &= 0x00ffffff;
    color |= 0x80000000;
    if (indexed_params.node_count > 0)
    {
        String_Const_u8 param_delim_space = push_stringf(scratch, "%.*s ", string_expand(delims.parameter_str));
        String_Const_u8 flattened = string_list_flatten(scratch, indexed_params, param_delim_space, 0, 0);
        String_Const_u8 parameters = push_stringf(scratch, "%.*s%.*s",
                                                  string_expand(param_delim_space),
                                                  string_expand(flattened)
                                                  );

        draw_pos = draw_string(app, face_id, parameters, draw_pos, color);
    }

    draw_pos = draw_string(app, face_id, delims.close_str, draw_pos, color);
}

function void function_index_menu_render(Application_Links *app, Frame_Info frame_info, View_ID view)
{
    Managed_Scope scope = view_get_managed_scope(app, view);
    Function_Index_Menu **menu_ptr = scope_attachment(app, scope, view_function_index_menu, Function_Index_Menu*);
    Function_Index_Menu *menu = *menu_ptr;

    if (!menu) return;
    menu->prev_render_caller(app, frame_info, view);
    if (menu->indices.count == 0) return;

    Buffer_ID buffer = view_get_buffer(app, view, Access_Always);
    Face_ID face = get_face_id(app, buffer);

    Scratch_Block scratch(app);

    Language_Function_Indexer *indexer = menu->indices.first->indexer;
    Language_Function_Delimiters delims = indexer->delims;
    String_Const_u8 param_delim_space = push_stringf(scratch, "%.*s ", string_expand(delims.parameter_str));

    Fancy_Block block = {};
    int i = 0;
    for (Function_Index *index = menu->indices.first;
         index != 0;
         index = index->next, i++)
    {
        List_String_Const_u8 parameters = indexer->parameter_strings(index, scratch);
        String_Const_u8 flattened = string_list_flatten(scratch, parameters, param_delim_space, 0, 0);
        String_Const_u8 param_str = push_stringf(scratch, "%.*s%.*s%.*s",
                                                 string_expand(delims.open_str),
                                                 string_expand(flattened),
                                                 string_expand(delims.close_str)
                                                 );

        Fancy_Line *line = push_fancy_line(scratch, &block, face);
        push_fancy_stringf(scratch, line, fcolor_id(defcolor_pop1), "F%d:", i+1);
        push_fancy_string(scratch, line, fcolor_id(defcolor_text_default), param_str);
    }


    Rect_f32 region = view_get_buffer_region(app, view);
    Buffer_Scroll scroll = view_get_buffer_scroll(app, view);
    Buffer_Point buffer_point = scroll.position;
    i64 pos = view_get_cursor_pos(app, view);
    Vec2_f32 cursor_p = view_relative_xy_of_pos(app, view, buffer_point.line_number, pos);
    cursor_p -= buffer_point.pixel_shift;
    cursor_p += region.p0;

    Face_Metrics metrics = get_face_metrics(app, face);
    f32 x_padding = metrics.normal_advance;
    f32 x_half_padding = x_padding*0.5f;

    draw_drop_down(app, face, &block, cursor_p, region, x_padding, x_half_padding,
                   fcolor_id(defcolor_margin_hover), fcolor_id(defcolor_back));
}

function Function_Index *get_function_index_from_user_drop_down(Application_Links *app, View_ID view, Buffer_ID buffer, i64 pos, Function_Index_List indices)
{
    View_Context ctx = view_current_context(app, view);
    Render_Caller_Function *prev_render_caller = ctx.render_caller;
    Function_Index_Menu menu = {indices, prev_render_caller};

    ctx.render_caller = function_index_menu_render;
    View_Context_Block ctx_block(app, view, &ctx);

    Managed_Scope scope = view_get_managed_scope(app, view);
    Function_Index_Menu **menu_ptr = scope_attachment(app, scope, view_function_index_menu, Function_Index_Menu*);
    *menu_ptr = &menu;

    Function_Index *index = 0;
    b32 keep_looping_menu = true;
    while (keep_looping_menu)
    {
        User_Input in = get_next_input(app, EventPropertyGroup_Any, EventProperty_Escape);

        if (in.abort) break;

        b32 handled = false;
        switch (in.event.kind)
        {
            case InputEventKind_KeyStroke:
            {
                switch(in.event.key.code)
                {
                    case KeyCode_F1:
                    case KeyCode_F2:
                    case KeyCode_F3:
                    case KeyCode_F4:
                    case KeyCode_F5:
                    case KeyCode_F6:
                    case KeyCode_F7:
                    case KeyCode_F8:
                    {
                        handled = true;
                        index = menu.indices.first;
                        for (int idx = in.event.key.code; idx > KeyCode_F1; idx--)
                        {
                            if (!index)
                                break;
                            index = index->next;
                        }
                        if (index)
                            keep_looping_menu = false;
                    }break;

                    default:
                    {
                        keep_looping_menu = false;
                    } break;
                }
            }
            case InputEventKind_TextInsert:
            {
                keep_looping_menu = false;
            } break;
        }
        if (!handled)
        {
            leave_current_input_unhandled(app);
        }
    }
    scope = view_get_managed_scope(app, view);
    menu_ptr = scope_attachment(app, scope, view_function_index_menu, Function_Index_Menu*);
    *menu_ptr = 0;

    return index;
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

    lang_indexer = (Language_Function_Indexer *)ext_support->ext_interface.str;
    Language_Function_Delimiters delims = lang_indexer->delims;

    if (!lang_indexer)
        return;

    i64 token_index = token_index_from_pos(tokens, cursor);
    while (token_index &&
           (tokens->tokens[token_index].kind == TokenBaseKind_Whitespace ||
            tokens->tokens[token_index].kind == TokenBaseKind_EOF ||
            tokens->tokens[token_index].kind == TokenBaseKind_Comment))
        token_index--;

    Function_Index *index = 0;
    Function_Index_List indices = {0};
    i64 token_start = token_index;
    b32 is_open = false;
    i64 open_pos = 0;
    if (tokens->tokens[token_index].kind == TokenBaseKind_Identifier)
    {
        Token token = tokens->tokens[token_index];
        indices = function_indices_from_token(app, scratch, buffer,
                                              token, lang_indexer);

        if (indices.count == 0)
            return;

        i64 insert_point = Ii64(&token).max;
        i64 peek_token_index = token_index+1;
        while (peek_token_index &&
               (tokens->tokens[peek_token_index].kind == TokenBaseKind_Whitespace ||
                tokens->tokens[peek_token_index].kind == TokenBaseKind_Comment))
            peek_token_index++;
        if (tokens->tokens[peek_token_index].sub_kind == lang_indexer->delims.open)
        {
            open_pos = Ii64(&tokens->tokens[peek_token_index]).max;
            view_set_cursor(app, view, seek_pos(open_pos));
        }
        else
        {
            Buffer_Insertion insert = begin_buffer_insertion_at(app, buffer, insert_point);
            insert_string(&insert, delims.open_str);
            end_buffer_insertion(&insert);

            open_pos = insert_point+delims.open_str.size;
            view_set_cursor(app, view, seek_pos(open_pos));
        }

        is_open = true;

    }
    else if (tokens->tokens[token_index].sub_kind == lang_indexer->delims.open)
    {
        is_open = true;
        token_index--;
        while (token_index &&
               (tokens->tokens[token_index].kind == TokenBaseKind_Whitespace ||
                tokens->tokens[token_index].kind == TokenBaseKind_EOF ||
                tokens->tokens[token_index].kind == TokenBaseKind_Comment))
            token_index--;

        if (tokens->tokens[token_index].kind != TokenBaseKind_Identifier)
            return;

        indices = function_indices_from_token(app, scratch, buffer,
                                              tokens->tokens[token_index], lang_indexer);
        open_pos = Ii64(&tokens->tokens[token_start]).max;
        if (indices.count == 0)
            return;
    }

    if (indices.count > 1)
    {
        index = get_function_index_from_user_drop_down(app, view, buffer, cursor, indices);
        if (!index)
            return;
    }
    else
    {
        index = indices.first;
    }

    if (!index) // No Index Found
        return;

    if (current_function_preview.index && string_match(index->name, current_function_preview.index->name))
    {
        current_function_preview = {0};
        return; // Toggle
    }

    current_function_preview.params = lang_indexer->parameter_strings(index, &tc_global_arena);
    current_function_preview.index = push_array(&tc_global_arena, Function_Index, 1);
    *current_function_preview.index = *index;
    current_function_preview.start_marker = alloc_buffer_markers_on_buffer(app, buffer, 1, 0);
    current_function_preview.end_marker = alloc_buffer_markers_on_buffer(app, buffer, 1, 0);
    Marker marker = {open_pos, false};
    managed_object_store_data(app, current_function_preview.start_marker, 0, 1, &marker);
    managed_object_store_data(app, current_function_preview.end_marker, 0, 1, &marker);

    current_function_preview.buffer = buffer;
}
