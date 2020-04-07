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
Parsed_Jump cpp_parse_jump_location(String_Const_u8 line) { return parse_jump_location(line); }