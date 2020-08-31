#include "4coder_terickson_language.h"

// @todo(tyler): Use wildcard string instead of extensions
function void push_language(Language *lang)
{
    if (!languages.last)
    {
        languages.first = languages.last = lang;
    }
    else
    {
        languages.last->next = lang;
        languages.last = lang;
    }
    languages.count++;
}

function void init_languages(Application_Links *app, Arena *arena)
{
    for (Language *lang = languages.first;
         lang != 0;
         lang = lang->next)
    {
        lang->file_extensions = parse_extension_line_to_extension_list(app, arena, lang->ext_string);
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
    for (Language *lang = languages.first;
         lang != 0;
         lang = lang->next)
    {
        for (i32 e = 0; e < lang->file_extensions.count; e++)
        {
            if (string_match(ext, lang->file_extensions.strings[e]))
            {
                return lang;
            }
        }
    }
    return 0;
}

function Language *language_from_name(String_Const_u8 name)
{
    for (Language *lang = languages.first;
         lang != 0;
         lang = lang->next)
    {
        if (string_match_insensitive(name, lang->name))
        {
            return lang;
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
    if (buffer_read_range(app, buffer, Ii64(pos, pos + lang->comment_delims.line.size), check_buffer)) {
        print_message(app, SCu8(check_buffer));
        if (string_match(SCu8(check_buffer), lang->comment_delims.line))
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
        buffer_replace_range(app, buffer, Ii64(pos), lang->comment_delims.line);
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
        buffer_replace_range(app, buffer, Ii64(pos, pos + lang->comment_delims.line.size), string_u8_empty);
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
        buffer_replace_range(app, buffer, Ii64(pos, pos + lang->comment_delims.line.size), string_u8_empty);
    else
        buffer_replace_range(app, buffer, Ii64(pos), lang->comment_delims.line);
}

CUSTOM_COMMAND_SIG(language_comment_range)
CUSTOM_DOC("Comment the current range according the current language's block comment delimiters.")
{
    View_ID view = get_active_view(app, Access_ReadWriteVisible);
    Buffer_ID buffer = view_get_buffer(app, view, Access_ReadWriteVisible);
    Language *lang = *buffer_get_language(app, buffer);
    
    String_Const_u8 begin = lang->comment_delims.block_start;
    String_Const_u8 end   = lang->comment_delims.block_end;
    
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
                i64 start = get_line_number_from_pos(app, buffer, current_nest->open.min);
                i64 end   = get_line_number_from_pos(app, buffer, current_nest->close.min);
                i64 count = end - start;
                
                i32 new_indent = base_indent + tab_width;
                block_fill_u64(indentations+start, sizeof(*indentations)*count, (u64)(new_indent));
                indent_nest_list(app, buffer, current_nest->nest_list, indentations, new_indent, tab_width);
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

function void language_add_extension(Language *lang, Extension_Support ext)
{
    Data str_data = *(Data *)&ext.ext_name;
    Data ext_data = push_data_copy(&tc_global_arena, make_data_struct(&ext));
    if (!lang->extension_support.allocator)
        lang->extension_support = make_table_Data_Data(tc_global_arena.base_allocator, 32);
    table_insert(&lang->extension_support, str_data, ext_data);
}

function void language_add_extension(String_Const_u8 name, Extension_Support ext)
{
    language_add_extension(language_from_name(name), ext);
}

function Extension_Support *language_get_extension(Language *lang, String_Const_u8 ext_name)
{
    Data str_data = *(Data *)&ext_name;
    Data ext_data;
    Table_Lookup lookup = table_lookup(&lang->extension_support, str_data);
    b32 res = table_read(&lang->extension_support, lookup, &ext_data);
    if (res)
        return (Extension_Support*)ext_data.data;
    return 0;
}

function Extension_Support *language_get_extension(String_Const_u8 lang_name, String_Const_u8 ext_name)
{
    return language_get_extension(language_from_name(lang_name), ext_name);
}