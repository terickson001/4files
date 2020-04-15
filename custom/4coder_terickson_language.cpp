
struct Language
{
     String_Const_u8 name;
     String_Const_u8 ext_string;
     char **token_kind_names;
     String_Const_u8 *comment_delims;
     // @note(tyler): These are required for a cancellable lexer,
     //               but the `Lex_State_{}` is specific to each language
     /*
     void (*lex_init)(Lex_State_{} *state_ptr, String_Const_u8 input);
     void (*lex_breaks)(Arena *arena, Token_List *list, Lex_State_{} *state_ptr, u64 max);
     */
     Token_List (*lex_full_input)(Arena *arena, String_Const_u8 input);
     b32 (*try_index)(Code_Index_File *index, Generic_Parse_State *state);
     FColor (*get_token_color)(Token token);
     Parsed_Jump (*parse_jump_location)(String_Const_u8 line);
     
     String_Const_u8_Array extensions;
};

#define LANG(PRETTY, NAME, EXT) \
{ \
SCu8(PRETTY), \
SCu8(EXT), \
token_##NAME##_kind_names, \
&NAME##_comment_delims[0], \
lex_full_input_##NAME, \
NAME##_try_index, \
NAME##_get_token_color, \
NAME##_parse_jump_location \
}

global Language *last_compiled_language = 0;
global Language languages[] = {
     LANG("CPP",  cpp,  ".c.cpp.h.hpp.cc"),
     LANG("Odin", odin, ".odin"),
     LANG("GLSL", glsl, ".glsl.vert.frag.geom.tess.vs.fs.gs.ts.compute"),
     LANG("GAS",  gas,  ".S"),
     LANG("NASM", nasm, ".asm")
};

#undef LANG

global Language *default_language = &languages[0];

#define LANG_COUNT (sizeof(languages)/sizeof(*languages))

function void init_languages(Application_Links *app, Arena *arena)
{
     for (int l = 0; l < LANG_COUNT; l++)
     {
         languages[l].extensions = parse_extension_line_to_extension_list(app, arena, languages[l].ext_string);
     }
}

// Helper Functions
function Language **buffer_get_language(Application_Links *app, Buffer_ID buffer)
{
     Managed_Scope scope = buffer_get_managed_scope(app, buffer);
     return scope_attachment(app, scope, buffer_language, Language*);
}

function void buffer_set_language(Application_Links *app, Buffer_ID buffer, Language *language)
{
     Managed_Scope scope = buffer_get_managed_scope(app, buffer);
     Language **lang_ptr = scope_attachment(app, scope, buffer_language, Language*);
     *lang_ptr = language;
}

function Language *language_from_extension(String_Const_u8 ext)
{
     for (i32 l = 0; l < LANG_COUNT; l++)
     {
         for (i32 e = 0; e < languages[l].extensions.count; e++)
         {
             if (string_match(ext, languages[l].extensions.strings[e]))
             {
                 return &languages[l];
             }
         }
     }
     return 0;
}

function Language *language_from_name(String_Const_u8 name)
{
     for (i32 l = 0; l < LANG_COUNT; l++)
     {
         if (string_match_insensitive(name, languages[l].name))
         {
             return &languages[l];
         }
     }
     return 0;
}


function void init_buffer(Application_Links *app, Buffer_ID buffer_id);
CUSTOM_COMMAND_SIG(set_language)
CUSTOM_DOC("Set the language for the current buffer.")
{
     View_ID view = get_active_view(app, Access_Always);
     i64 cursor = view_get_cursor_pos(app, view);
     Buffer_ID buffer = view_get_buffer(app, view, Access_Always);
     
     u8 language_buff[1024];
     
     Query_Bar_Group bar_group(app);
     Query_Bar language_bar = {};
     language_bar.prompt = string_u8_litexpr("Language: ");
     language_bar.string = SCu8(language_buff, (u64)0);
     language_bar.string_capacity = 1024;
     if (!query_user_string(app, &language_bar)) return;
     if (language_bar.string.size == 0) return;
     
     Scratch_Block scratch(app);
     
     Language *language = language_from_name(language_bar.string);
     print_message(app, push_stringf(scratch, "Setting language to %.*s\n", string_expand(language->name)));
     buffer_set_language(app, buffer, language_from_name(language_bar.string));
     init_buffer(app, buffer); // @note(tyler): Must re-init with new language
}

CUSTOM_COMMAND_SIG(print_language)
CUSTOM_DOC("Print the language for the current buffer.")
{
     View_ID view = get_active_view(app, Access_Always);
     i64 cursor = view_get_cursor_pos(app, view);
     Buffer_ID buffer = view_get_buffer(app, view, Access_Always);
     
     Scratch_Block scratch(app);
     
     Language *lang = *buffer_get_language(app, buffer);
     print_message(app, push_stringf(scratch, "Language is set to %.*s\n", string_expand(lang->name)));
}

function b32 language_line_comment_starts_at_position(Application_Links *app, Buffer_ID buffer, i64 pos, Language *lang)
{
     b32 already_has_comment = false;
     u8 check_buffer[64] = {0};
     if (buffer_read_range(app, buffer, Ii64(pos, pos + lang->comment_delims[0].size), check_buffer)) {
         print_message(app, SCu8(check_buffer));
         if (string_match(SCu8(check_buffer), lang->comment_delims[0]))
         already_has_comment = true;
     }
     return(already_has_comment);
}

CUSTOM_COMMAND_SIG(language_comment_line)
CUSTOM_DOC("Comment the current line with the current language's delimeters.")
{
     View_ID view = get_active_view(app, Access_ReadWriteVisible);
     Buffer_ID buffer = view_get_buffer(app, view, Access_ReadWriteVisible);
     i64 pos = get_start_of_line_at_cursor(app, view, buffer);
     Language *lang = *buffer_get_language(app, buffer);
     b32 already_has_comment = language_line_comment_starts_at_position(app, buffer, pos, lang);
     if (!already_has_comment)
     buffer_replace_range(app, buffer, Ii64(pos), lang->comment_delims[0]);
}


CUSTOM_COMMAND_SIG(language_uncomment_line)
CUSTOM_DOC("Comment the current line with the current language's delimeters.")
{
     View_ID view = get_active_view(app, Access_ReadWriteVisible);
     Buffer_ID buffer = view_get_buffer(app, view, Access_ReadWriteVisible);
     i64 pos = get_start_of_line_at_cursor(app, view, buffer);
     Language *lang = *buffer_get_language(app, buffer);
     b32 already_has_comment = language_line_comment_starts_at_position(app, buffer, pos, lang);
     if (already_has_comment)
     buffer_replace_range(app, buffer, Ii64(pos, pos + lang->comment_delims[0].size), string_u8_empty);
}

CUSTOM_COMMAND_SIG(language_comment_line_toggle)
CUSTOM_DOC("Comment the current line with the current language's delimeters.")
{
     View_ID view = get_active_view(app, Access_ReadWriteVisible);
     Buffer_ID buffer = view_get_buffer(app, view, Access_ReadWriteVisible);
     i64 pos = get_start_of_line_at_cursor(app, view, buffer);
     Language *lang = *buffer_get_language(app, buffer);
     b32 already_has_comment = language_line_comment_starts_at_position(app, buffer, pos, lang);
     if (already_has_comment)
     buffer_replace_range(app, buffer, Ii64(pos, pos + lang->comment_delims[0].size), string_u8_empty);
     else
     buffer_replace_range(app, buffer, Ii64(pos), lang->comment_delims[0]);
}

CUSTOM_COMMAND_SIG(language_comment_range)
CUSTOM_DOC("Comment the current range according the current language's block comment delimiters.")
{
     View_ID view = get_active_view(app, Access_ReadWriteVisible);
     Buffer_ID buffer = view_get_buffer(app, view, Access_ReadWriteVisible);
     Language *lang = *buffer_get_language(app, buffer);
     
     String_Const_u8 begin = lang->comment_delims[1];
     String_Const_u8 end   = lang->comment_delims[2];
     
     Range_i64 range = get_view_range(app, view);
     Range_i64 lines = get_line_range_from_pos_range(app, buffer, range);
     range = get_pos_range_from_line_range(app, buffer, lines);
     
     Scratch_Block scratch(app);
     
     b32 min_line_blank = line_is_valid_and_blank(app, buffer, lines.min);
     b32 max_line_blank = line_is_valid_and_blank(app, buffer, lines.max);
     
     if ((lines.min < lines.max) || (!min_line_blank)){
         String_Const_u8 begin_str = {};
         String_Const_u8 end_str = {};
         
         i64 min_adjustment = 0;
         i64 max_adjustment = 0;
         
         if (min_line_blank){
             begin_str = push_u8_stringf(scratch, "\n%.*s", string_expand(begin));
             min_adjustment += 1;
         }
         else{
             begin_str = push_u8_stringf(scratch, "%.*s\n", string_expand(begin));
         }
         if (max_line_blank){
             end_str = push_u8_stringf(scratch, "%.*s\n", string_expand(end));
         }
         else{
             end_str = push_u8_stringf(scratch, "\n%.*s", string_expand(end));
             max_adjustment += 1;
         }
         
         max_adjustment += begin_str.size;
         Range_i64 new_pos = Ii64(range.min + min_adjustment, range.max + max_adjustment);
         
         History_Group group = history_group_begin(app, buffer);
         buffer_replace_range(app, buffer, Ii64(range.min), begin_str);
         buffer_replace_range(app, buffer, Ii64(range.max + begin_str.size), end_str);
         history_group_end(group);
         
         set_view_range(app, view, new_pos);
     }
     else{
         String_Const_u8 str = push_u8_stringf(scratch, "%.*s\n\n%.*s", string_expand(begin), string_expand(end));
         buffer_replace_range(app, buffer, range, str);
         i64 center_pos = range.min + begin.size + 1;
         view_set_cursor_and_preferred_x(app, view, seek_pos(center_pos));
         view_set_mark(app, view, seek_pos(center_pos));
     }
}

function i64 get_column_from_pos(Application_Links *app, Buffer_ID buffer, i64 pos)
{
     Buffer_Cursor cursor = buffer_compute_cursor(app, buffer, seek_pos(pos));
     return(cursor.col);
}

function void indent_nest_list(Application_Links *app, Buffer_ID buffer, Code_Index_Nest_List nests, i64 *indentations, i32 base_indent, i32 tab_width)
{
     for (Code_Index_Nest *current_nest = nests.first;
          current_nest != 0 ;
     current_nest = current_nest->next)
     {
         switch (current_nest->kind)
         {
             case CodeIndexNest_Scope: {
                 i64 start = get_line_number_from_pos(app, buffer, current_nest->open.min);
                 i64 end   = get_line_number_from_pos(app, buffer, current_nest->close.min);
                 i64 count = end - start - 1;
                 
                 i32 new_indent = base_indent + tab_width;
                 block_fill_u64(indentations+start, sizeof(*indentations)*count, (u64)(new_indent));
                 indent_nest_list(app, buffer, current_nest->nest_list, indentations, new_indent, tab_width);
             } break;
             
             case CodeIndexNest_Paren: {
                 i64 start = get_line_number_from_pos(app, buffer, current_nest->open.min);
                 i64 end   = get_line_number_from_pos(app, buffer, current_nest->close.min);
                 i64 count = end - start - 1;
                 
                 i64 column = Max(0, get_column_from_pos(app, buffer, current_nest->open.max)-1);// + indentations[start-1];
                 block_fill_u64(indentations+start, sizeof(*indentations)*count, (u64)(column));
                 indent_nest_list(app, buffer, current_nest->nest_list, indentations, column, tab_width);
             } break;
             
             case CodeIndexNest_Statement: {
                 indent_nest_list(app, buffer, current_nest->nest_list, indentations, base_indent, tab_width);
             } break;
         }
         
     }
}

function i64 *get_indentation_array_from_index(Application_Links *app, Arena *arena, Buffer_ID buffer, Range_i64 lines, Indent_Flag flags, i32 tab_width, i32 indent_width)
{
     i64 count = lines.max - lines.min + 1;
     i64 *indentations = push_array(arena, i64, count);
     i64 *shifted_indentations = indentations - lines.first;
     block_fill_u64(indentations, sizeof(*indentations)*count, (u64)(0));
     
     Code_Index_File *file = code_index_get_file(buffer);
     if (file == 0)
     return indentations;
     
     Code_Index_Nest_List nests = file->nest_list;
     indent_nest_list(app, buffer, nests, indentations, 1, tab_width);
     return indentations;
}