#include "generated/lexer_odin.h"
#include "generated/lexer_odin.cpp"
#include "4coder_terickson_language.h"

// Common
function b32 odin_is_builtin_type(Token *token)
{
    return TokenOdinKind_byte <= token->sub_kind &&
        token->sub_kind <= TokenOdinKind_u128be;
}

function b32 odin_is_builtin_proc(Token *token)
{
    return TokenOdinKind_len <= token->sub_kind &&
        token->sub_kind <= TokenOdinKind_card;
}

function b32 odin_is_directive(Token *token)
{
    return TokenOdinKind_align <= token->sub_kind &&
        token->sub_kind <= TokenOdinKind_partial;
}

function b32 odin_is_attribute(Token *token)
{
    return TokenOdinKind_builtin <= token->sub_kind &&
        token->sub_kind <= TokenOdinKind_thread_local;
}
// Index
#include "languages/odin/index.cpp"

// Highlight
static FColor odin_get_token_color(Token token)
{
    Managed_ID color = defcolor_text_default;
    switch (token.kind){
        case TokenBaseKind_Preprocessor:
        {
            color = defcolor_preproc;
        } break;
        case TokenBaseKind_Keyword:
        {
            if (odin_is_directive(&token) || odin_is_attribute(&token))
                color = defcolor_preproc;
            else if (odin_is_builtin_type(&token))
                color = defcolor_type_name;
            else if (odin_is_builtin_proc(&token))
                color = defcolor_function_name;
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
        case TokenBaseKind_Identifier:
        {
            if (odin_is_builtin_type(&token))
                color = defcolor_type_name;
            else if (odin_is_builtin_proc(&token))
                color = defcolor_function_name;
        }break;
    }
    return(fcolor_id(color));
}

// Jump
function Parsed_Jump odin_parse_jump_location(String_Const_u8 line)
{
    Parsed_Jump jump = {};
    
    line = string_skip_chop_whitespace(line);
    u64 lparen_pos = string_find_first(line, '(');
    u64 colon_pos  = string_find_first(string_skip(line, lparen_pos), ':')+lparen_pos;
    u64 rparen_pos = string_find_first(string_skip(line, colon_pos), ')')+colon_pos;
    
    String_Const_u8 file_name     = string_prefix(line, lparen_pos);
    String_Const_u8 line_number   = string_skip(string_prefix(line, colon_pos), lparen_pos+1);
    String_Const_u8 column_number = string_skip(string_prefix(line, rparen_pos), colon_pos+1);
    String_Const_u8 message = string_skip(line, rparen_pos + 2);
    
    if (file_name.size > 0 && line_number.size > 0 && column_number.size > 0)
    {
        jump.location.file   = file_name;
        jump.location.line   = (i32)string_to_integer(line_number, 10);
        jump.location.column = (i32)string_to_integer(column_number, 10);
        jump.colon_position = (i32)(rparen_pos);
        // jump.msg = message;
        jump.success = true;
    }
    
    if (!jump.success)
        block_zero_struct(&jump);
    else
        jump.is_sub_jump = false; // @note(tyler): What is this for?
    
    return jump;
}

Comment_Delimiters odin_comment_delims = {SCu8("//"), SCu8("/*"), SCu8("*/")};

LEX_INIT_DEF(odin, Lex_State_Odin);
LEX_BREAKS_DEF(odin, Lex_State_Odin);

static Language language_def_odin = LANG_DEF("Odin", odin, ".odin");

// Extensions
#include "ext/function_indexer.cpp"
#include "ext/std_include.cpp"

function void init_language_odin()
{
    push_language(&language_def_odin);
    
#ifdef EXT_FUNCTION_INDEX
    Extension_Support findex_support = {EXT_FUNCTION_INDEX, make_data_struct(&odin_function_indexer)};
    language_add_extension(&language_def_odin, findex_support);
#endif
#ifdef EXT_STD_INCLUDE
    Extension_Support std_include_support = {EXT_STD_INCLUDE, make_data_struct(&odin_std_includes)};
    language_add_extension(&language_def_odin, std_include_support);
#endif
}