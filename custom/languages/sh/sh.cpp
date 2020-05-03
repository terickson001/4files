#include "generated/lexer_sh.h"
#include "generated/lexer_sh.cpp"

// Common

// Index
#include "languages/sh/index.cpp"

// Highlight
static FColor sh_get_token_color(Token token)
{
    Managed_ID color = defcolor_text_default;
    switch (token.kind){
        case TokenBaseKind_Preprocessor:
        {
            color = defcolor_preproc;
        } break;
        case TokenBaseKind_Keyword:
        {
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
        case TokenBaseKind_Identifier:
        {
            color = defcolor_text_default;
        }break;
    }
    return(fcolor_id(color));
}

// Jump
function Parsed_Jump sh_parse_jump_location(String_Const_u8 line)
{
    return (Parsed_Jump){0};
}

String_Const_u8 sh_comment_delims[3] = {SCu8("#"), string_u8_empty, string_u8_empty};