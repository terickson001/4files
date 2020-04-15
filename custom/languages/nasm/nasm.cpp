#include "generated/lexer_nasm.h"
#include "generated/lexer_nasm.cpp"

// Common
function b32 nasm_is_register(Token *token)
{
     return TokenGasKind_rax <= token->sub_kind &&
     token->sub_kind <= TokenGasKind_xmm15;
}

// Index
#include "languages/nasm/index.cpp"

// Highlight
static FColor nasm_get_token_color(Token token)
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
function Parsed_Jump nasm_parse_jump_location(String_Const_u8 line)
{
     return cpp_parse_jump_location(line);
}

String_Const_u8 nasm_comment_delims[3] = {SCu8(";"), SCu8("%startcomment"), SCu8("%endcomment")};