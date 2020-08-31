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
        jump.msg = message;
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


#ifdef EXT_FUNCTION_INDEX
function Function_Index *odin_parse_function__findexer(Application_Links *app, Code_Index_Note *note, Arena *arena)
{
    Buffer_ID buffer = note->file->buffer;
    Token_Array tokens = get_token_array_from_buffer(app, buffer);
    i64 idx = token_index_from_pos(&tokens, note->pos.max+1);
    skip_to_params:
    switch (tokens.tokens[idx].sub_kind)
    {
        case TokenOdinKind_ColonColon:
        case TokenOdinKind_Whitespace:
        case TokenOdinKind_proc:
        idx++;
        goto skip_to_params;
        
        default:
        break;
    }
    
    Code_Index_Nest *nest = code_index_get_nest(note->file, tokens.tokens[idx].pos+1);
    if (!nest || nest->kind != CodeIndexNest_Paren)
        return 0;
    
    Range_i64 param_range = Ii64(nest->open.max, nest->close.min);
    
    Function_Index *index = push_array_zero(arena, Function_Index, 1);
    index->note = note;
    index->name = note->text;
    index->parameters = {0};
    
    idx = token_index_from_pos(&tokens, param_range.min);
    
    String_Const_u8 param_string = push_buffer_range(app, arena, buffer, param_range);
    Function_Parameter *param = 0;
    
    while (tokens.tokens[idx].pos < param_range.max)
    {
        String_Const_u8 prefix = {};
        String_Const_u8 type = {};
        String_Const_u8 postfix = {};
        String_Const_u8 name = {};
        
        while (tokens.tokens[idx].kind == TokenBaseKind_Whitespace ||
               tokens.tokens[idx].kind == TokenBaseKind_Comment) idx++;
        
        //// PARAM NAME
        if (tokens.tokens[idx].sub_kind == TokenOdinKind_Identifier)
        {
            name.str = &param_string.str[tokens.tokens[idx].pos - param_range.min];
            name.size = tokens.tokens[idx].size;
            idx++;
        }
        
        while (tokens.tokens[idx].kind == TokenBaseKind_Whitespace ||
               tokens.tokens[idx].kind == TokenBaseKind_Comment) idx++;
        
        if (tokens.tokens[idx].sub_kind != TokenOdinKind_Colon)
        {
            type = name;
            name = {0};
            goto type_postfix;
        }
        else
        {
            idx++;
        }
        
        //// TYPE PREFIX
        type_prefix:
        switch (tokens.tokens[idx].sub_kind)
        {
            case TokenOdinKind_Whitespace:
            idx++;
            goto type_prefix;
            
            case TokenOdinKind_Carrot:
            if (prefix.str == 0)
                prefix.str = &param_string.str[tokens.tokens[idx].pos - param_range.min];
            idx++;
            goto type_prefix;
            
            case TokenOdinKind_BrackOp:
            {
                if (prefix.str == 0)
                    prefix.str = &param_string.str[tokens.tokens[idx].pos - param_range.min];
                Code_Index_Nest *nest = code_index_get_nest(note->file, tokens.tokens[idx].pos);
                idx = token_index_from_pos(&tokens, nest->close.max);
                goto type_prefix;
            }
            
            default:
            if (prefix.str != 0)
            {
                prefix.size = &param_string.str[Ii64(&tokens.tokens[idx-1]).max-param_range.min] - prefix.str;
            }
            break;
        }
        while (tokens.tokens[idx].kind == TokenBaseKind_Whitespace ||
               tokens.tokens[idx].kind == TokenBaseKind_Comment) idx++;
        
        //// TYPE NAME
        type_name:
        switch (tokens.tokens[idx].sub_kind)
        {
            case TokenOdinKind_Identifier:
            type.str = &param_string.str[tokens.tokens[idx].pos - param_range.min];
            type.size = tokens.tokens[idx].size;
            idx++;
            break;
            
            default:
            if (odin_is_builtin_type(&tokens.tokens[idx]))
            {
                type.str = &param_string.str[tokens.tokens[idx].pos - param_range.min];
                type.size = tokens.tokens[idx].size;
                idx++;
            }
            break;
        }
        
        //// TYPE POSTFIX
        type_postfix:
        switch (tokens.tokens[idx].sub_kind)
        {
            
            case TokenOdinKind_ParenOp:
            {
                if (postfix.str == 0)
                    postfix.str = &param_string.str[tokens.tokens[idx].pos - param_range.min];
                Code_Index_Nest *nest = code_index_get_nest(note->file, tokens.tokens[idx].pos);
                idx = token_index_from_pos(&tokens, nest->close.max);
                goto type_postfix;
            }
            
            default:
            if (postfix.str != 0)
                postfix.size = &param_string.str[Ii64(&tokens.tokens[idx-1]).max-param_range.min] - postfix.str;
            break;
        }
        postfix = string_skip_chop_whitespace(postfix);
        
        //// SKIP TO COMMA or PAREN
        skip_rest:
        switch (tokens.tokens[idx].sub_kind)
        {
            case TokenOdinKind_Comma:
            case TokenOdinKind_ParenCl:
            idx++;
            break;
            
            default:
            idx++;
            goto skip_rest;
        }
        
        param = push_array_zero(arena, Function_Parameter, 1);
        param->name = push_string_copy(arena, name);
        param->postfix = push_string_copy(arena, postfix);
        param->type = push_string_copy(arena, type);
        sll_queue_push(index->parameters.first, index->parameters.last, param);
    }
    
    return index;
}

function List_String_Const_u8 odin_parameter_strings(Function_Index *index, Arena *arena)
{
    List_String_Const_u8 param_strings = {};//push_array(arena, List_String_Const_u8, 1);
    for (Function_Parameter *param = index->parameters.first;
         param != 0;
         param = param->next)
    {
        if(param->type.size == 0)
            continue;
        string_list_pushf(arena, &param_strings, "%.*s%s%.*s%.*s%.*s",
                          string_expand(param->name),
                          param->name.size!=0?": ":0,
                          string_expand(param->prefix),
                          string_expand(param->type),
                          string_expand(param->postfix)
                          );
    }
    
    return param_strings;
}

static Language_Function_Indexer odin_function_indexer =
{
    .lang = &language_def_odin,
    .parse_function = odin_parse_function__findexer,
    .parameter_strings = odin_parameter_strings,
    .delims {
        TokenOdinKind_ParenOp, SCu8("("),
        TokenOdinKind_ParenCl, SCu8(")"),
        TokenOdinKind_Comma, SCu8(",")
    },
};
#endif

function void init_language_odin()
{
    push_language(&language_def_odin);
    
#ifdef EXT_FUNCTION_INDEX
    Extension_Support findex_support = {EXT_FUNCTION_INDEX, make_data_struct(&odin_function_indexer)};
    language_add_extension(&language_def_odin, findex_support);
#endif
}