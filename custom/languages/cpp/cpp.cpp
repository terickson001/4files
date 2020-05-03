#include "languages/cpp/index.cpp"
#include "4coder_terickson_language.h"

// Common
function b32 cpp_is_builtin_type(Token *token)
{
    return TokenCppKind_Void <= token->sub_kind
        && token->sub_kind <= TokenCppKind_Signed;
}

// Highlight
FColor cpp_get_token_color(Token token)
{
    Managed_ID color = defcolor_text_default;
    switch (token.kind) {
        case TokenBaseKind_Preprocessor:
        {
            color = defcolor_preproc;
        } break;
        case TokenBaseKind_Keyword:
        {
            if (cpp_is_builtin_type(&token))
                color = defcolor_type_name;
            else
                color = defcolor_keyword;
        } break;
        case TokenBaseKind_Comment:
        {
            color = defcolor_comment;
        } break;
        case TokenBaseKind_LiteralString:
        {
            color = defcolor_str_constant;
        } break;
        case TokenBaseKind_LiteralInteger:
        {
            color = defcolor_int_constant;
        } break;
        case TokenBaseKind_LiteralFloat:
        {
            color = defcolor_float_constant;
        } break;
        default:
        {
            switch (token.sub_kind){
                case TokenCppKind_LiteralTrue:
                case TokenCppKind_LiteralFalse:
                {
                    color = defcolor_bool_constant;
                } break;
                
                case TokenCppKind_PPIncludeFile:
                {
                    color = defcolor_include;
                } break;
            }
        } break;
    }
    return(fcolor_id(color));
}

// Jump
Parsed_Jump cpp_parse_jump_location(String_Const_u8 line)
{
    Parsed_Jump jump = {};
    jump.sub_jump_indented = (string_get_character(line, 0) == ' ');
    
    String_Const_u8 reduced_line = string_skip_chop_whitespace(line);
    u64 whitespace_length = (u64)(reduced_line.str - line.str);
    line = reduced_line;
    
    u64 left_paren_pos = string_find_first(line, '(');
    u64 right_paren_pos = left_paren_pos + string_find_first(string_skip(line, left_paren_pos), ')');
    for (;!jump.is_ms_style && right_paren_pos < line.size;){
        if (ms_style_verify(line, left_paren_pos, right_paren_pos)){
            jump.is_ms_style = true;
            jump.colon_position = (i32)(right_paren_pos + string_find_first(string_skip(line, right_paren_pos), ':'));
            if (jump.colon_position < line.size){
                if (check_is_note(line, jump.colon_position)){
                    jump.sub_jump_note = true;
                }
                
                String_Const_u8 location_str = string_prefix(line, jump.colon_position);
                location_str = string_skip_chop_whitespace(location_str);
                
                i32 close_pos = (i32)right_paren_pos;
                i32 open_pos = (i32)left_paren_pos;
                
                if (0 < open_pos && open_pos < location_str.size){
                    String_Const_u8 file = SCu8(location_str.str, open_pos);
                    file = string_skip_chop_whitespace(file);
                    
                    if (file.size > 0){
                        String_Const_u8 line_number = string_skip(string_prefix(location_str, close_pos), open_pos + 1);
                        line_number = string_skip_chop_whitespace(line_number);
                        
                        if (line_number.size > 0){
                            u64 comma_pos = string_find_first(line_number, ',');
                            if (comma_pos < line_number.size){
                                String_Const_u8 column_number = string_skip(line_number, comma_pos + 1);
                                
                                line_number = string_prefix(line_number, comma_pos);
                                jump.location.line = (i32)string_to_integer(line_number, 10);
                                jump.location.column = (i32)string_to_integer(column_number, 10);
                            }
                            else{
                                jump.location.line = (i32)string_to_integer(line_number, 10);
                                jump.location.column = 0;
                            }
                            jump.location.file = file;
                            jump.colon_position = jump.colon_position + (i32)whitespace_length;
                            jump.success = true;
                        }
                    }
                }
            }
        }
        else{
            left_paren_pos = string_find_first(string_skip(line, left_paren_pos + 1), '(') + left_paren_pos + 1;
            right_paren_pos = string_find_first(string_skip(line, left_paren_pos), ')') + left_paren_pos;
        }
    }
    
    if (!jump.is_ms_style){
        i32 start = (i32)try_skip_rust_arrow(line);
        if (start != 0){
            jump.has_rust_arrow = true;
        }
        
        u64 colon_pos1 = string_find_first(string_skip(line, start), ':') + start;
        if (line.size > colon_pos1 + 1){
            if (character_is_slash(string_get_character(line, colon_pos1 + 1))){
                colon_pos1 = string_find_first(string_skip(line, colon_pos1 + 1), ':') + colon_pos1 + 1;
            }
        }
        
        u64 colon_pos2 = string_find_first(string_skip(line, colon_pos1 + 1), ':') + colon_pos1 + 1;
        u64 colon_pos3 = string_find_first(string_skip(line, colon_pos2 + 1), ':') + colon_pos2 + 1;
        
        if (colon_pos3 < line.size){
            if (check_is_note(line, colon_pos3)){
                jump.sub_jump_note = true;
            }
            
            String_Const_u8 file_name = string_skip(string_prefix(line, colon_pos1), start);
            String_Const_u8 line_number = string_skip(string_prefix(line, colon_pos2), colon_pos1 + 1);
            String_Const_u8 column_number = string_skip(string_prefix(line, colon_pos3), colon_pos2 + 1);
            String_Const_u8 message = string_skip(line, colon_pos3 + 2);
            if (file_name.size > 0 && line_number.size > 0 && column_number.size > 0){
                jump.location.file = file_name;
                jump.location.line = (i32)string_to_integer(line_number, 10);
                jump.location.column = (i32)string_to_integer(column_number, 10);
                jump.colon_position = (i32)(colon_pos3 + whitespace_length);
                jump.msg = message;
                jump.success = true;
            }
        }
        else{
            if (colon_pos2 < line.size){
                if (check_is_note(line, colon_pos2)){
                    jump.sub_jump_note = true;
                }
                
                String_Const_u8 file_name = string_prefix(line, colon_pos1);
                String_Const_u8 line_number = string_skip(string_prefix(line, colon_pos2), colon_pos1 + 1);
                String_Const_u8 message = string_skip(line, colon_pos2 + 2);
                
                if (string_is_integer(line_number, 10)){
                    if (file_name.size > 0 && line_number.size > 0){
                        jump.location.file = file_name;
                        jump.location.line = (i32)string_to_integer(line_number, 10);
                        jump.location.column = 0;
                        jump.colon_position = (i32)(colon_pos3 + whitespace_length);
                        jump.msg = message;
                        jump.success = true;
                    }
                }
            }
        }
    }
    
    if (!jump.success){
        block_zero_struct(&jump);
    }
    else{
        jump.is_sub_jump = (jump.sub_jump_indented || jump.sub_jump_note);
    }
    return(jump);
}

Comment_Delimiters cpp_comment_delims = {SCu8("//"), SCu8("/*"), SCu8("*/")};

static Language language_def_cpp = LANG_DEF("CPP", cpp, ".c.cpp.h.hpp.cc");

#ifdef EXT_FUNCTION_INDEX
function Function_Index *cpp_parse_function__findexer(Application_Links *app, Code_Index_Note *note, Arena *arena)
{
    Code_Index_Nest *nest = code_index_get_nest(note->file, note->pos.max+1);
    if (!nest || nest->kind != CodeIndexNest_Paren)
        return 0;
    
    Buffer_ID buffer = note->file->buffer;
    Range_i64 param_range = Ii64(nest->open.max, nest->close.min);
    Token_Array tokens = get_token_array_from_buffer(app, buffer);
    
    Function_Index *index = push_array_zero(arena, Function_Index, 1);
    index->note = note;
    index->name = note->text;
    index->parameters = {0};
    
    i64 idx = token_index_from_pos(&tokens, param_range.min);
    String_Const_u8 param_string = push_buffer_range(app, arena, buffer, param_range);
    Function_Parameter *param = 0;
    
    while (tokens.tokens[idx].pos < param_range.max)
    {
        String_Const_u8 prefix = {};
        String_Const_u8 type = {};
        String_Const_u8 postfix = {};
        String_Const_u8 name = {};
        
        //// TYPE PREFIX
        type_prefix:
        switch (tokens.tokens[idx].sub_kind)
        {
            case TokenCppKind_Static:
            case TokenCppKind_Volatile:
            case TokenCppKind_Const:
            if (prefix.str == 0)
                prefix.str = &param_string.str[tokens.tokens[idx].pos - param_range.min];
            idx++;
            goto type_prefix;
            
            default:
            if (prefix.str != 0)
                prefix.size = &param_string.str[Ii64(&tokens.tokens[idx-1]).max-param_range.min] - prefix.str;
            break;
        }
        
        while (tokens.tokens[idx].kind == TokenBaseKind_Whitespace ||
               tokens.tokens[idx].kind == TokenBaseKind_Comment) idx++;
        
        //// TYPE NAME
        type_name:
        switch (tokens.tokens[idx].sub_kind)
        {
            case TokenCppKind_Identifier:
            case TokenCppKind_Struct:
            case TokenCppKind_Enum:
            case TokenCppKind_Union:
            if (type.str == 0)
                type.str = &param_string.str[tokens.tokens[idx].pos - param_range.min];
            idx++;
            goto type_name;
            
            default:
            if (cpp_is_builtin_type(&tokens.tokens[idx]))
            {
                if (type.str == 0)
                    type.str = &param_string.str[tokens.tokens[idx].pos - param_range.min];
                idx++;
                goto type_name;
            }
            else
            {
                if (type.str != 0)
                    type.size = &param_string.str[Ii64(&tokens.tokens[idx-1]).max-param_range.min] - type.str;
                break;
            }
        }
        
        while (tokens.tokens[idx].kind == TokenBaseKind_Whitespace ||
               tokens.tokens[idx].kind == TokenBaseKind_Comment) idx++;
        
        //// TYPE POSTFIX
        type_postfix:
        switch (tokens.tokens[idx].sub_kind)
        {
            case TokenCppKind_Star:
            if (postfix.str == 0)
                postfix.str = &param_string.str[tokens.tokens[idx].pos - param_range.min];
            idx++;
            goto type_postfix;
            
            default:
            if (postfix.str != 0)
                postfix.size = &param_string.str[Ii64(&tokens.tokens[idx-1]).max-param_range.min] - postfix.str;
            break;
        }
        
        while (tokens.tokens[idx].kind == TokenBaseKind_Whitespace ||
               tokens.tokens[idx].kind == TokenBaseKind_Comment) idx++;
        //// PARAM NAME
        switch (tokens.tokens[idx].sub_kind)
        {
            case TokenCppKind_Identifier:
            name.str = &param_string.str[tokens.tokens[idx].pos - param_range.min];
            name.size = tokens.tokens[idx].size;
            idx++;
            break;
            
            default:
            break;
        }
        
        //// SKIP TO COMMA or PAREN
        skip_rest:
        switch (tokens.tokens[idx].sub_kind)
        {
            case TokenCppKind_Comma:
            case TokenCppKind_ParenCl:
            idx++;
            break;
            
            default:
            idx++;
            goto skip_rest;
        }
        
        param = push_array_zero(arena, Function_Parameter, 1);
        param->prefix = push_string_copy(arena, prefix);
        param->name = push_string_copy(arena, name);
        param->postfix = push_string_copy(arena, postfix);
        param->type = push_string_copy(arena, type);
        sll_queue_push(index->parameters.first, index->parameters.last, param);
    }
    
    return index;
}

function List_String_Const_u8 cpp_parameter_strings(Function_Index *index, Arena *arena)
{
    List_String_Const_u8 param_strings = {};//push_array(arena, List_String_Const_u8, 1);
    for (Function_Parameter *param = index->parameters.first;
         param != 0;
         param = param->next)
    {
        if(param->type.size == 0)
            continue;
        string_list_pushf(arena, &param_strings, "%.*s%c%.*s%.*s",
                          string_expand(param->type),
                          param->name.size!=0 || param->postfix.size!=0?' ':0,
                          string_expand(param->postfix),
                          string_expand(param->name)
                          );
    }
    
    return param_strings;
}

static Language_Function_Indexer cpp_function_indexer =
{
    cpp_parse_function__findexer,
    cpp_parameter_strings,
    .delims = {
        TokenCppKind_ParenOp, SCu8("("),
        TokenCppKind_ParenCl, SCu8(")"),
        TokenCppKind_Comma, SCu8(",")
    },
};
#endif

function void init_language_cpp()
{
    push_language(&language_def_cpp);
#ifdef EXT_FUNCTION_INDEX
    Extension_Support findex_support = {EXT_FUNCTION_INDEX, make_data_struct(&cpp_function_indexer)};
    language_add_extension(&language_def_cpp, findex_support);
#endif
}