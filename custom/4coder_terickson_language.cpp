// Conformant interface for CPP builtins

b32 cpp_try_index(Code_Index_File *index, Generic_Parse_State *state, Token *token)
{
    if (token->sub_kind == TokenCppKind_Struct ||
        token->sub_kind == TokenCppKind_Union ||
        token->sub_kind == TokenCppKind_Enum){
        cpp_parse_type_structure(index, state, 0);
        return true;
    }
    else if (token->sub_kind == TokenCppKind_Typedef) {
        cpp_parse_type_def(index, state, 0);
        return true;
    }
    else if (token->sub_kind == TokenCppKind_Identifier) {
        cpp_parse_function(index, state, 0);
        return true;
    }
    return false;
}

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

struct Language
{
    String_Const_u8 name;
    String_Const_u8 ext_string;
    char **token_kind_names;
    // @note(tyler): These are required for a cancellable lexer,
    //               but the `Lex_State_{}` is specific to each language
    /*
        void (*lex_init)(Lex_State_{} *state_ptr, String_Const_u8 input);
        void (*lex_breaks)(Arena *arena, Token_List *list, Lex_State_{} *state_ptr, u64 max);
     */
    Token_List (*lex_full_input)(Arena *arena, String_Const_u8 input);
    b32 (*try_index)(Code_Index_File *index, Generic_Parse_State *state, Token *token);
    FColor (*get_token_color)(Token token);
    Parsed_Jump (*parse_jump_location)(String_Const_u8 line);
    
    String_Const_u8_Array extensions;
};

global Language *last_compiled_language = 0;
global Language languages[] = {
    {
        SCu8("CPP"),
        SCu8(".c.cpp.h.hpp.cc.glsl"),
        token_cpp_kind_names,
        lex_full_input_cpp,
        cpp_try_index,
        cpp_get_token_color,
        parse_jump_location
    },
    {
        SCu8("Odin"),
        SCu8(".odin"),
        token_odin_kind_names,
        lex_full_input_odin,
        odin_try_index,
        odin_get_token_color,
        odin_parse_jump_location
    }
};

#define LANG_COUNT (sizeof(languages)/sizeof(*languages))

function void init_languages(Application_Links *app, Arena *arena)
{
    for (int l = 0; l < LANG_COUNT; l++)
    {
        languages[l].extensions = parse_extension_line_to_extension_list(app, arena, languages[l].ext_string);
    }
}

// Helper Functions
function Language **buffer_get_language(Application_Links *app, Buffer_ID buffer)
{
    Managed_Scope scope = buffer_get_managed_scope(app, buffer);
    return scope_attachment(app, scope, buffer_language, Language*);
}

function void buffer_set_language(Application_Links *app, Buffer_ID buffer, Language *language)
{
    Managed_Scope scope = buffer_get_managed_scope(app, buffer);
    Language **lang_ptr = scope_attachment(app, scope, buffer_language, Language*);
    *lang_ptr = language;
}

function Language *language_from_extension(String_Const_u8 ext)
{
    for (i32 l = 0; l < LANG_COUNT; l++)
    {
        for (i32 e = 0; e < languages[l].extensions.count; e++)
        {
            if (string_match(ext, languages[l].extensions.strings[e]))
            {
                return &languages[l];
            }
        }
    }
    return 0;
}