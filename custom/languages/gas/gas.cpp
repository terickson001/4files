#include "generated/lexer_gas.h"
#include "generated/lexer_gas.cpp"

// Common
function b32 gas_is_register(Token *token)
{
     return TokenGasKind_rax <= token->sub_kind &&
     token->sub_kind <= TokenGasKind_xmm15;
}

// Index
#include "languages/gas/index.cpp"

// Highlight
static FColor gas_get_token_color(Token token)
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
     }
     return(fcolor_id(color));
}

// Jump
function Parsed_Jump gas_parse_jump_location(String_Const_u8 line)
{
     return cpp_parse_jump_location(line);
}

String_Const_u8 gas_comment_delims[3] = {SCu8("#"), SCu8("/*"), SCu8("*/")};