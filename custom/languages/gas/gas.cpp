#include "generated/lexer_gas.h"
#include "generated/lexer_gas.cpp"
#include "4coder_terickson_language.h"

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

Comment_Delimiters gas_comment_delims = {SCu8("#"), SCu8("/*"), SCu8("*/")};

LEX_INIT_DEF(gas, Lex_State_Gas);
LEX_BREAKS_DEF(gas, Lex_State_Gas);

static Language language_def_gas = LANG_DEF("GAS", gas, ".s.S");

function void init_language_gas()
{
     push_language(&language_def_gas);
}