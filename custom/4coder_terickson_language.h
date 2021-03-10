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


struct Hook
{
    Hook *next;
    Void_Func *f;
};

struct Hook_List
{
    Hook *first;
    Hook *last;
    int count;
};


#define language_run_hooks(HOOK, ...) \
for (Hook *h = LANGUAGE_HOOKS[Hook_##HOOK].first; h; h = h->next) \
((HOOK##_Hook *)h->f)(__VA_ARGS__); \

typedef Generic_Parse_Comment_Function HandleComment_Hook;
typedef void PostIndex_Hook(Application_Links *app, Code_Index_File *index);
typedef void PreIndex_Hook(Application_Links *app, Code_Index_File *index);
typedef Tick_Function Tick_Hook;
typedef Buffer_Hook_Function PreBeginBuffer_Hook;
typedef Buffer_Hook_Function PostBeginBuffer_Hook;
enum Language_Hook_Kind
{
    Hook_HandleComment,   /* Generic_Parse_Comment_Hook */
    Hook_PreIndex,        /* PreIndex_Hook */
    Hook_PostIndex,       /* PostIndex_Hook */
    Hook_PreBeginBuffer,  /* PreBeginBuffer_Hook */
    Hook_PostBeginBuffer, /* PostBeginBuffer_Hook */
    Hook_Tick,            /* Tick_Hook */
    HOOK_COUNT
};

global Hook_List LANGUAGE_HOOKS[HOOK_COUNT] = {0};

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
if (state->str == 0) \
*state = push_data(arena, sizeof(LEX_STATE)); \
Assert(state->size == sizeof(LEX_STATE)); \
lex_full_input_##NAME##_init((LEX_STATE *)state->str, input); \
}
#define LEX_BREAKS_DEF(NAME, LEX_STATE) \
function b32 NAME##_lex_breaks(Arena *arena, Token_List *list, Generic_Lex_State *state, u64 max) \
{ \
Assert(state->size == sizeof(LEX_STATE)); \
return lex_full_input_##NAME##_breaks(arena, list, (LEX_STATE *)state->str, max); \
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

function void init_ext_language(Application_Links *app);
function void push_language(Language *lang);
function void finalize_languages(Application_Links *app);

/***** CODE INDEX *****/
struct Code_Index_Table
{
    Buffer_ID buffer;
    Arena *arena;
    Table_Data_u64 notes;
};

function Code_Index_Nest*
language_generic_parse_scope(Code_Index_File *index, Generic_Parse_State *state, b32 allow_decl);
function b32 cpp_parse_extern(Code_Index_File *index, Generic_Parse_State *state);
function b32 language_generic_parse_scope_paren(Code_Index_File *index, Generic_Parse_State *state);
function Code_Index_Table *get_code_index_table(Table_u64_u64 *table, Buffer_ID buffer);
function void set_code_index_table(Table_u64_u64 *table, Buffer_ID buffer, Code_Index_Table *code_index);
function b32 language_generic_parse_full_input_breaks(Code_Index_File *index, Generic_Parse_State *state, i32 limit);
function void language_code_index_update_tick(Application_Links *app);

/***** LEXER *****/
function void
language_do_full_lex_async__inner(Async_Context *actx, Buffer_ID buffer_id);
function void
language_do_full_lex_async(Async_Context *actx, Data data);

/***** HIGHLIGHT *****/
static void language_paint_tokens(Application_Links *app, Buffer_ID buffer, Text_Layout_ID text_layout_id, Token_Array *array);

/***** JUMPING *****/
Parsed_Jump try_language_jump(String_Const_u8 line_str);
internal Sticky_Jump_Array language_parse_buffer_to_jump_array(Application_Links *app, Arena *arena, Buffer_ID buffer);
internal void language_init_marker_list(Application_Links *app, Heap *heap, Buffer_ID buffer, Marker_List *list);
internal Marker_List* language_get_or_make_list_for_buffer(Application_Links *app, Heap *heap, Buffer_ID buffer_id);
CUSTOM_COMMAND_SIG(language_if_read_only_goto_position);
CUSTOM_COMMAND_SIG(language_if_read_only_goto_position_same_panel);

/***** HOOKS *****/
function b32 language_begin_buffer__determine_language(Application_Links *app, Buffer_ID buffer_id);
function void language_begin_buffer__launch_lexer(Application_Links *app, Buffer_ID buffer_id);
function void language_init_buffer(Application_Links *app, Buffer_ID buffer_id);
BUFFER_HOOK_SIG(language_begin_buffer);

Token_List language_buffer_edit_range__relex(Application_Links *app, Buffer_ID buffer_id, Arena *arena, String_Const_u8 text);
BUFFER_EDIT_RANGE_SIG(language_buffer_edit_range);

static void language_render_buffer(Application_Links *app, View_ID view_id, Face_ID face_id,
                                   Buffer_ID buffer, Text_Layout_ID text_layout_id,
                                   Rect_f32 rect);
function void language_render_caller(Application_Links *app, Frame_Info frame_info, View_ID view_id);

function void language_tick(Application_Links *app, Frame_Info frame_info);

/***** MISC COMMANDS *****/
CUSTOM_COMMAND_SIG(set_language);
CUSTOM_COMMAND_SIG(print_language);
CUSTOM_COMMAND_SIG(language_comment_line);
CUSTOM_COMMAND_SIG(language_uncomment_line);
CUSTOM_COMMAND_SIG(language_comment_line_toggle);
CUSTOM_COMMAND_SIG(language_comment_range);

/***** HELPERS *****/
function Language **buffer_get_language(Application_Links *app, Buffer_ID buffer);
function void buffer_set_language(Application_Links *app, Buffer_ID buffer, Language *language);
function Language *language_from_extension(String_Const_u8 ext);
function Language *language_from_name(String_Const_u8 name);
function void language_push_hook(Language_Hook_Kind kind, Void_Func *f);

/***** EXTENSIONS *****/
function void language_add_extension(Language *lang, Extension_Support ext);
function void language_add_extension(String_Const_u8 name, Extension_Support ext);
function Extension_Support *language_get_extension(Language *lang, String_Const_u8 ext_name);
function Extension_Support *language_get_extension(String_Const_u8 lang_name, String_Const_u8 ext_name);

#endif // TERICKSON_LANGUAGE_H
