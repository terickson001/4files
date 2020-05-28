#include "generated/lexer_glsl.h"
#include "generated/lexer_glsl.cpp"
#include "4coder_terickson_language.h"

// Common
function b32 glsl_is_builtin_type(Token *token)
{
     return TokenGlslKind_float <= token->sub_kind &&
         token->sub_kind <= TokenGlslKind_sampler3DRect;
}

function b32 glsl_is_builtin_proc(Token *token)
{
     return TokenGlslKind_abs <= token->sub_kind &&
         token->sub_kind <= TokenGlslKind_textureCubeLod;
}

// Index
#include "languages/glsl/index.cpp"

// Highlight
static FColor glsl_get_token_color(Token token)
{
     Managed_ID color = defcolor_text_default;
     switch (token.kind){
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
         case TokenBaseKind_Identifier:
         {
             if (glsl_is_builtin_type(&token))
                 color = defcolor_type_name;
             else if (glsl_is_builtin_proc(&token))
                 color = defcolor_function_name;
         } break;
     }
     return(fcolor_id(color));
}

// Jump
function Parsed_Jump glsl_parse_jump_location(String_Const_u8 line)
{
     return {.success = false};
}

Comment_Delimiters glsl_comment_delims = {SCu8("//"), SCu8("/*"), SCu8("*/")};


LEX_INIT_DEF(glsl, Lex_State_Glsl);
LEX_BREAKS_DEF(glsl, Lex_State_Glsl);

static Language language_def_glsl = LANG_DEF("GLSL", glsl, ".glsl.vert.frag.geom.tess.vs.fs.gs.ts.compute");

function void init_language_glsl()
{
     push_language(&language_def_glsl);
}