
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

#define LANG(PRETTY, NAME, EXT) \
{ \
SCu8(PRETTY), \
SCu8(EXT), \
token_##NAME##_kind_names, \
lex_full_input_##NAME, \
NAME##_try_index, \
NAME##_get_token_color, \
NAME##_parse_jump_location \
}

global Language *last_compiled_language = 0;
global Language languages[] = {
    LANG("CPP", cpp, ".c.cpp.h.hpp.cc"),
    LANG("Odin", odin, ".odin"),
    LANG("GLSL", glsl, ".glsl.vert.frag.geom.tess.vs.fs.gs.ts.compute")
};

#undef LANG

global Language *default_language = &languages[0];

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

function Language *language_from_name(String_Const_u8 name)
{
    for (i32 l = 0; l < LANG_COUNT; l++)
    {
        if (string_match_insensitive(name, languages[l].name))
        {
            return &languages[l];
        }
    }
    return 0;
}


function void init_buffer(Application_Links *app, Buffer_ID buffer_id);
CUSTOM_COMMAND_SIG(set_language)
CUSTOM_DOC("Set the language for the current buffer.")
{
    View_ID view = get_active_view(app, Access_Always);
    i64 cursor = view_get_cursor_pos(app, view);
    Buffer_ID buffer = view_get_buffer(app, view, Access_Always);
    
    u8 language_buff[1024];
    
    Query_Bar_Group bar_group(app);
    Query_Bar language_bar = {};
    language_bar.prompt = string_u8_litexpr("Language: ");
    language_bar.string = SCu8(language_buff, (u64)0);
    language_bar.string_capacity = 1024;
    if (!query_user_string(app, &language_bar)) return;
    if (language_bar.string.size == 0) return;
    
    Scratch_Block scratch(app);
    
    Language *language = language_from_name(language_bar.string);
    print_message(app, push_stringf(scratch, "Setting language to %.*s\n", string_expand(language->name)));
    buffer_set_language(app, buffer, language_from_name(language_bar.string));
    init_buffer(app, buffer); // @note(tyler): Must re-init with language
}

CUSTOM_COMMAND_SIG(print_language)
CUSTOM_DOC("Print the language for the current buffer.")
{
    View_ID view = get_active_view(app, Access_Always);
    i64 cursor = view_get_cursor_pos(app, view);
    Buffer_ID buffer = view_get_buffer(app, view, Access_Always);
    
    Scratch_Block scratch(app);
    
    Language *lang = *buffer_get_language(app, buffer);
    print_message(app, push_stringf(scratch, "Language is set to %.*s\n", string_expand(lang->name)));
}