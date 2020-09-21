#ifndef TERICKSON_LANGUAGE_H
#define TERICKSON_LANGUAGE_H

typedef struct Extension_Support
{
    String_Const_u8 ext_name;
    Data ext_interface;
} Extension_Support;

typedef Table_Data_Data Extension_Support_Table;

typedef struct Comment_Delimiters
{
    String_Const_u8 line;
    String_Const_u8 block_start;
    String_Const_u8 block_end;
} Comment_Delimiters;

typedef Data Generic_Lex_State;

struct Language
{
    String_Const_u8 name;
    String_Const_u8 ext_string;
    char **token_kind_names;
    Comment_Delimiters comment_delims;
    // @note(tyler): These are required for a cancellable lexer,
    //               but the `Lex_State_{}` is specific to each language
    void (*lex_init)(Arena *arena, Generic_Lex_State *state, String_Const_u8 input);
    b32 (*lex_breaks)(Arena *arena, Token_List *list, Generic_Lex_State *state_ptr, u64 max);
    
    Token_List (*lex_full_input)(Arena *arena, String_Const_u8 input);
    b32 (*try_index)(Code_Index_File *index, Generic_Parse_State *state);
    FColor (*get_token_color)(Token token);
    Parsed_Jump (*parse_jump_location)(String_Const_u8 line);
    
    Extension_Support_Table extension_support;
    String_Const_u8_Array file_extensions;
    
    Language *next;
};

#define LEX_INIT_DEF(NAME, LEX_STATE) \
function void NAME##_lex_init(Arena *arena, Generic_Lex_State *state, String_Const_u8 input) \
{ \
if (state->data == 0) \
*state = push_data(arena, sizeof(LEX_STATE)); \
Assert(state->size == sizeof(LEX_STATE)); \
lex_full_input_##NAME##_init((LEX_STATE *)state->data, input); \
}
#define LEX_BREAKS_DEF(NAME, LEX_STATE) \
function b32 NAME##_lex_breaks(Arena *arena, Token_List *list, Generic_Lex_State *state, u64 max) \
{ \
Assert(state->size == sizeof(LEX_STATE)); \
return lex_full_input_##NAME##_breaks(arena, list, (LEX_STATE *)state->data, max); \
}

#define LANG_DEF(PRETTY, NAME, EXT) \
{ \
SCu8(PRETTY), \
SCu8(EXT), \
token_##NAME##_kind_names, \
NAME##_comment_delims, \
NAME##_lex_init, \
NAME##_lex_breaks, \
lex_full_input_##NAME, \
NAME##_try_index, \
NAME##_get_token_color, \
NAME##_parse_jump_location \
}

struct Language_List
{
    Language *first;
    Language *last;
    
    i64 count;
};

global Language_List languages = {0};
global Language *default_language = 0;

function void push_language(Language *lang);
function Language **buffer_get_language(Application_Links *app, Buffer_ID buffer);
function void buffer_set_language(Application_Links *app, Buffer_ID buffer, Language *language);
function Language *language_from_extension(String_Const_u8 ext);
function Language *language_from_name(String_Const_u8 name);

function void language_add_extension(Language *lang, Extension_Support ext);
function void language_add_extension(String_Const_u8 name, Extension_Support ext);
function Extension_Support *language_get_extension(Language *lang, String_Const_u8 ext_name);
function Extension_Support *language_get_extension(String_Const_u8 lang_name, String_Const_u8 ext_name);
#endif // TERICKSON_LANGUAGE_H
