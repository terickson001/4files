#include "languages/cpp/index.cpp"

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
            color = defcolor_keyword; break;
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
            String_Const_u8 msg = string_skip(line, colon_pos3 + 2);
            if (file_name.size > 0 && line_number.size > 0 && column_number.size > 0){
                jump.location.file = file_name;
                jump.location.line = (i32)string_to_integer(line_number, 10);
                jump.location.column = (i32)string_to_integer(column_number, 10);
                jump.colon_position = (i32)(colon_pos3 + whitespace_length);
                jump.msg = msg;
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
                
                if (string_is_integer(line_number, 10)){
                    if (file_name.size > 0 && line_number.size > 0){
                        jump.location.file = file_name;
                        jump.location.line = (i32)string_to_integer(line_number, 10);
                        jump.location.column = 0;
                        jump.colon_position = (i32)(colon_pos3 + whitespace_length);
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

String_Const_u8 cpp_comment_delims[3] = {SCu8("//"), SCu8("/*"), SCu8("*/")};