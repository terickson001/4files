#include "4coder_terickson_language.h"

global Arena language_arena = {};
/****** SETUP GUIDE *******/
#if 0
// In your bindings file (or `4coder_default_bindings.cpp`) you
// have to #include "4coder_language_ids.cpp" _immediately_ following
// the #include "4coder_default_include.cpp"

#include "4coder_default_include.cpp"
#include "4coder_language_ids.cpp" // <- This needs to be here
#include "generated/managed_id_metadata.cpp"

// Afterwards, you need to #include "4coder_terickson_language.cpp" (this file)
// Followed by any language-dependent extensions, and language definitions
// In that order

#include "4coder_terickson_language.cpp"

// Extensions
#include "ext/4coder_terickson_function_index.cpp"
#include "ext/4coder_terickson_todo.cpp"

// Languages
#include "languages/cpp/cpp.cpp"
#include "languages/odin/odin.cpp"
#include "languages/glsl/glsl.cpp"

// Inside your `custom_layer_init`, initialize the main extension,
// and all the languages and language-dependent extensions you want.
// And finally, call `set_language_hooks` immediately after `set_all_default_hooks`

void custom_layer_init(Application_Links *app)
{
    /*...*/
    
    // Multi-Language Support
    init_ext_language(app);
    
    // Language Dependent Extensions
    init_ext_todo();
    
    // Language Definitions
    init_language_cpp();
    init_language_odin();
    init_language_glsl();
    init_language_gas();
    init_language_nasm();
    
    // Finalize language definitions
    finalize_languages(app);
    
    /*...*/
    set_all_default_hooks(app);
    set_language_hooks(app);
    // You can override any of the language hooks here
    /*...*/
}

// 4Coder requires the lexers to be compiled seperately, which requires an extra
// step before compiling.
// for each language you have installed in `custom/languages/<lang>`, run the
// `build_lang` script that comes bundled with this extension.
// i.e.:
//     $ ./build_lang.sh glsl
//   or
//     $ ./build_lang.bat glsl

// For keybindings, I recommend overriding the default commenting binds
// as well as the default `if_read_only_goto_position*` binds, which handle
// jumping to errors

{
    {"language_comment_line_toggle",  "Semicolon", "Control"},
    {"language_comment_range",        "R", "Alt"},
    
    {"language_if_read_only_goto_position",  "Return"};
    {"language_if_read_only_goto_position_same_panel", "Return", "Shift"};
}

#endif


// @todo(tyler): Use wildcard string instead of extensions

function void init_ext_language()
{
    language_arena = make_arena_system();
}

function void set_language_hooks(Application_Links *app)
{
    set_custom_hook(app, HookID_BeginBuffer, language_begin_buffer);
    set_custom_hook(app, HookID_BufferEditRange, language_buffer_edit_range);
    set_custom_hook(app, HookID_RenderCaller, language_render_caller);
    set_custom_hook(app, HookID_Tick, language_tick);
}

function void push_language(Language *lang)
{
    if (!languages.last)
    {
        languages.first = languages.last = lang;
    }
    else
    {
        languages.last->next = lang;
        languages.last = lang;
    }
    languages.count++;
}

function void finalize_languages(Application_Links *app)
{
    for (Language *lang = languages.first;
         lang != 0;
         lang = lang->next)
    {
        lang->file_extensions = parse_extension_line_to_extension_list(app, &language_arena, lang->ext_string);
    }
}

/***** CODE INDEX *****/
function Code_Index_Nest*
language_generic_parse_scope(Code_Index_File *index, Generic_Parse_State *state, b32 allow_decl){
    Managed_Scope scope = buffer_get_managed_scope(state->app, index->buffer);
    Language **language = scope_attachment(state->app, scope, buffer_language, Language*);
    
    Token *token = token_it_read(&state->it);
    Code_Index_Nest *result = push_array_zero(state->arena, Code_Index_Nest, 1);
    result->kind = CodeIndexNest_Scope;
    result->open = Ii64(token);
    result->close = Ii64(max_i64);
    result->file = index;
    
    state->scope_counter += 1;
    
    generic_parse_inc(state);
    for (;;){
        generic_parse_skip_soft_tokens(index, state);
        token = token_it_read(&state->it);
        if (token == 0 || state->finished){
            break;
        }
        
        if (state->in_preprocessor){
            if (!HasFlag(token->flags, TokenBaseFlag_PreprocessorBody) ||
                token->kind == TokenBaseKind_Preprocessor){
                break;
            }
        }
        else{
            if (token->kind == TokenBaseKind_Preprocessor){
                Code_Index_Nest *nest = generic_parse_preprocessor(index, state);
                code_index_push_nest(&index->nest_list, nest);
                continue;
            }
        }
        
        if (token->kind == TokenBaseKind_ScopeClose){
            result->is_closed = true;
            result->close = Ii64(token);
            generic_parse_inc(state);
            break;
        }
        
        
        if (allow_decl && *language && (*language)->try_index(index, state))
            continue;
        
        
        if (token->kind == TokenBaseKind_ScopeOpen){
            Code_Index_Nest *nest = generic_parse_scope(index, state);
            nest->parent = result;
            code_index_push_nest(&result->nest_list, nest);
            continue;
        }
        
        if (token->kind == TokenBaseKind_ParentheticalClose){
            generic_parse_inc(state);
            continue;
        }
        
        if (token->kind == TokenBaseKind_ParentheticalOpen){
            Code_Index_Nest *nest = generic_parse_paren(index, state);
            nest->parent = result;
            code_index_push_nest(&result->nest_list, nest);
            
            // NOTE(allen): after a parenthetical group we consider ourselves immediately
            // transitioning into a statement
            nest = generic_parse_statement(index, state);
            nest->parent = result;
            code_index_push_nest(&result->nest_list, nest);
            
            continue;
        }
        
        {
            Code_Index_Nest *nest = generic_parse_statement(index, state);
            nest->parent = result;
            code_index_push_nest(&result->nest_list, nest);
        }
    }
    
    result->nest_array = code_index_nest_ptr_array_from_list(state->arena, &result->nest_list);
    
    state->scope_counter -= 1;
    
    return(result);
}

function b32 cpp_parse_extern(Code_Index_File *index, Generic_Parse_State *state)
{
    Token *token = token_it_read(&state->it);
    if (token->sub_kind == TokenCppKind_Extern)
    {
        generic_parse_inc(state);
        generic_parse_skip_soft_tokens(index, state);
        Token *peek = token_it_read(&state->it);
        switch (peek->kind)
        {
            case TokenBaseKind_LiteralString:
            {
                generic_parse_inc(state);
                generic_parse_skip_soft_tokens(index, state);
                if (token_it_read(&state->it)->kind != TokenBaseKind_ScopeOpen)
                    break;
            }
            
            case TokenBaseKind_ScopeOpen:
            {
                Code_Index_Nest *nest = language_generic_parse_scope(index, state, true);
                code_index_push_nest(&index->nest_list, nest);
                return true;
            } break;
        }
    }
    return false;
}

function b32 language_generic_parse_scope_paren(Code_Index_File *index, Generic_Parse_State *state)
{
    Token *token = token_it_read(&state->it);
    if (token->kind == TokenBaseKind_ScopeOpen) {
        Code_Index_Nest *nest = language_generic_parse_scope(index, state, false);
        code_index_push_nest(&index->nest_list, nest);
        return true;
    }
    else if (token->kind == TokenBaseKind_ParentheticalOpen) {
        Code_Index_Nest *nest = generic_parse_paren(index, state);
        code_index_push_nest(&index->nest_list, nest);
        return true;
    }
    return false;
}


global Table_u64_u64 code_index_tables = {0};

function Code_Index_Table *get_code_index_table(Table_u64_u64 *table, Buffer_ID buffer)
{
    Code_Index_Table *code_index;
    Table_Lookup lookup = table_lookup(table, buffer);
    b32 res = table_read(table, lookup, (u64*)&code_index);
    if (res)
        return code_index;
    return 0;
}

function void set_code_index_table(Table_u64_u64 *table, Buffer_ID buffer, Code_Index_Table *code_index)
{
    if (!table->allocator)
        *table = make_table_u64_u64(language_arena.base_allocator, 32);
    table_erase(table, buffer);
    table_insert(table, buffer, HandleAsU64(code_index));
}

function b32 language_generic_parse_full_input_breaks(Code_Index_File *index, Generic_Parse_State *state, i32 limit)
{
    b32 result = false;
    
    Managed_Scope scope = buffer_get_managed_scope(state->app, index->buffer);
    Language **language = scope_attachment(state->app, scope, buffer_language, Language*);
    Thread_Context *tctx = get_thread_context(state->app);
    
    if (!*language) return true;
    
    Code_Index_Table *code_index = get_code_index_table(&code_index_tables, index->buffer);
    if (code_index)
        tctx_release(tctx, code_index->arena);
    {
        Arena *index_arena = tctx_reserve(tctx);
        code_index = push_array_zero(index_arena, Code_Index_Table, 1);
        code_index->buffer = index->buffer;
        code_index->arena = index_arena;
        code_index->notes = make_table_Data_u64(index_arena->base_allocator, 64);
        set_code_index_table(&code_index_tables, index->buffer, code_index);
    }
    
    i64 first_index = token_it_index(&state->it);
    i64 one_past_last_index = first_index + limit;
    for (;;){
        generic_parse_skip_soft_tokens(index, state);
        Token *token = token_it_read(&state->it);
        
        if (token == 0 || state->finished){
            result = true;
            break;
        }
        
        if      ((*language)->try_index(index, state));
        else if (language_generic_parse_scope_paren(index, state));
        else if (cpp_parse_extern(index, state));
        else    generic_parse_inc(state);
        
        i64 index = token_it_index(&state->it);
        if (index >= one_past_last_index){
            token = token_it_read(&state->it);
            if (token == 0){
                result = true;
            }
            break;
        }
    }
    
    if (result){
        index->nest_array = code_index_nest_ptr_array_from_list(state->arena, &index->nest_list);
        index->note_array = code_index_note_ptr_array_from_list(state->arena, &index->note_list);
        for (int i = 0; i < index->note_array.count; i++)
        {
            Data str_data = *(Data *)&index->note_array.ptrs[i]->text;
            Table_Lookup lookup = table_lookup(&code_index->notes, str_data);
            Code_Index_Note_List *list;
            b32 res = table_read(&code_index->notes, lookup, (u64 *)&list);
            if (res)
            {
                sll_queue_push(list->first, list->last, index->note_array.ptrs[i]);
            }
            else
            {
                list = push_array_zero(code_index->arena, Code_Index_Note_List, 1);
                Code_Index_Note *note = push_array_write(code_index->arena, Code_Index_Note, 1, index->note_array.ptrs[i]);
                sll_queue_push(list->first, list->last, note);
                table_insert(&code_index->notes, str_data, HandleAsU64(list));
            }
        }
        
    }
    
    return(result);
}

function void language_code_index_update_tick(Application_Links *app)
{
    Scratch_Block scratch(app);
    for (Buffer_Modified_Node *node = global_buffer_modified_set.first;
         node != 0;
         node = node->next){
        Temp_Memory_Block temp(scratch);
        Buffer_ID buffer_id = node->buffer;
        
        String_Const_u8 contents = push_whole_buffer(app, scratch, buffer_id);
        Token_Array tokens = get_token_array_from_buffer(app, buffer_id);
        if (tokens.count == 0){
            continue;
        }
        
        Arena arena = make_arena_system(KB(16));
        Code_Index_File *index = push_array_zero(&arena, Code_Index_File, 1);
        index->buffer = buffer_id;
        
        Generic_Parse_State state = {};
        generic_parse_init(app, &arena, contents, &tokens, &state);
        if (LANGUAGE_HOOKS[Hook_HandleComment].first)
            state.handle_comment = (Generic_Parse_Comment_Function *)LANGUAGE_HOOKS[Hook_HandleComment].first->f;
        // TODO(allen): Actually determine this in a fair way.
        // Maybe switch to an enum?
        // Actually probably a pointer to a struct that defines the language.
        state.do_cpp_parse = true;
        language_generic_parse_full_input_breaks(index, &state, max_i32);
        
        code_index_lock();
        code_index_set_file(buffer_id, arena, index);
        code_index_unlock();
        
        language_run_hooks(PostIndex, app, index);
        buffer_clear_layout_cache(app, buffer_id);
    }
    
    buffer_modified_set_clear();
}

/***** LEXER *****/

function void
language_do_full_lex_async__inner(Async_Context *actx, Buffer_ID buffer_id){
    Application_Links *app = actx->app;
    ProfileScope(app, "async lex");
    Scratch_Block scratch(app);
    
    String_Const_u8 contents = {};
    {
        ProfileBlock(app, "async lex contents (before mutex)");
        acquire_global_frame_mutex(app);
        ProfileBlock(app, "async lex contents (after mutex)");
        contents = push_whole_buffer(app, scratch, buffer_id);
        release_global_frame_mutex(app);
    }
    
    i32 limit_factor = 10000;
    
    Token_List list = {};
    b32 canceled = false;
    Managed_Scope scope = buffer_get_managed_scope(app, buffer_id);
    Language **language = scope_attachment(app, scope, buffer_language, Language*);
    
    Generic_Lex_State state = {};
    (*language)->lex_init(scratch, &state, contents);
    for (;;){
        ProfileBlock(app, "async lex block");
        if ((*language)->lex_breaks(scratch, &list, &state, limit_factor)){
            break;
        }
        if (async_check_canceled(actx)){
            canceled = true;
            break;
        }
    }
    if (!canceled){
        ProfileBlock(app, "async lex save results (before mutex)");
        acquire_global_frame_mutex(app);
        ProfileBlock(app, "async lex save results (after mutex)");
        
        if (scope != 0){
            Base_Allocator *allocator = managed_scope_allocator(app, scope);
            Token_Array *tokens_ptr = scope_attachment(app, scope, attachment_tokens, Token_Array);
            base_free(allocator, tokens_ptr->tokens);
            Token_Array tokens = {};
            tokens.tokens = base_array(allocator, Token, list.total_count);
            tokens.count = list.total_count;
            tokens.max = list.total_count;
            token_fill_memory_from_list(tokens.tokens, &list);
            block_copy_struct(tokens_ptr, &tokens);
        }
        buffer_mark_as_modified(buffer_id);
        release_global_frame_mutex(app);
    }
}

function void
language_do_full_lex_async(Async_Context *actx, Data data){
    if (data.size == sizeof(Buffer_ID)){
        Buffer_ID *buffer = (Buffer_ID*)data.str;
        language_do_full_lex_async__inner(actx, *buffer);
    }
}

/***** HIGHLIGHT *****/

static void language_paint_tokens(Application_Links *app, Buffer_ID buffer, Text_Layout_ID text_layout_id, Token_Array *array)
{
    Scratch_Block scratch(app);
    FColor col = {0};
    
    Managed_Scope scope = buffer_get_managed_scope(app, buffer);
    Language **language = scope_attachment(app, scope, buffer_language, Language*);
    
    if (array->tokens == 0)
        return;
    
    Range_i64 visible_range = text_layout_get_visible_range(app, text_layout_id);
    i64 first_index = token_index_from_pos(array, visible_range.first);
    Token_Iterator_Array it = token_iterator_index(0, array, first_index);
    
    for (;;)
    {
        Token *token = token_it_read(&it);
        
        if (token->pos >= visible_range.end+1)
            break;
        
        // Check Notes
        b32 custom_note_color_used = false;
        if (token->kind == TokenBaseKind_Identifier)
        {
            String_Const_u8 token_as_string = push_token_lexeme(app, scratch, buffer, token);
            Data str_data = *(Data *)&token_as_string;
            
            for (Buffer_ID buf = get_buffer_next(app, 0, Access_Always);
                 buf != 0;
                 buf = get_buffer_next(app, buf, Access_Always))
            {
                if (*buffer_get_language(app, buf) != *buffer_get_language(app, buffer)) continue;
                
                Code_Index_Table *code_index = get_code_index_table(&code_index_tables, buf);
                if (code_index == 0)
                    continue;
                
                Table_Lookup lookup = table_lookup(&code_index->notes, str_data);
                Code_Index_Note_List *list;
                b32 res = table_read(&code_index->notes, lookup, (u64 *)&list);
                if (!res || !list || !list->first) continue;
                
                for (Code_Index_Note *note = list->first; note; note = note->next)
                {
                    switch (note->note_kind)
                    {
                        case CodeIndexNote_Type:
                        {
                            custom_note_color_used = true;
                            Range_i64 range = {};
                            range.start = token->pos;
                            range.end = token->pos + token->size;
                            paint_text_color_fcolor(app, text_layout_id, range, fcolor_id(defcolor_type_name));
                        } break;
                        case CodeIndexNote_Macro:
                        case CodeIndexNote_Function:
                        {
                            Token *peek;
                            do
                            {
                                token_it_inc_all(&it);
                                peek = token_it_read(&it);
                            } while (peek->kind == TokenBaseKind_Whitespace);
                            it = token_iterator(it.user_id, it.tokens, it.count, token);
                            
                            b32 invalid = false;
                            if (peek->kind != TokenBaseKind_ParentheticalOpen && token->pos != note->pos.min)
                                invalid = true;
                            if (invalid) continue;
                            
                            custom_note_color_used = true;
                            
                            Range_i64 range = {};
                            range.start = token->pos;
                            range.end = token->pos + token->size;
                            paint_text_color_fcolor(app, text_layout_id, range, fcolor_id(defcolor_function_name));
                        } break;
                        
                        default: continue;
                    }
                    break;
                }
            }
        }
        // END: Check Notes
        
        if (!custom_note_color_used)
        {
            FColor color = (*language)->get_token_color(*token);
            paint_text_color_fcolor(app, text_layout_id, Ii64_size(token->pos, token->size), color);
        }
        
        if (!token_it_inc_all(&it))
            break;
    }
}

/***** JUMPING *****/
Parsed_Jump try_language_jump(String_Const_u8 line_str)
{
    Parsed_Jump jump = {};
    for (Language *lang = languages.first;
         lang != 0;
         lang = lang->next)
    {
        jump = lang->parse_jump_location(line_str);
        if (jump.success)
            return jump;
    }
    return jump;
}

internal Sticky_Jump_Array language_parse_buffer_to_jump_array(Application_Links *app, Arena *arena, Buffer_ID buffer){
    Sticky_Jump_Node *jump_first = 0;;
    Sticky_Jump_Node *jump_last = 0;
    i32 jump_count = 0;
    
    Managed_Scope scope = buffer_get_managed_scope(app, buffer);
    Base_Allocator *managed_allocator = managed_scope_allocator(app, scope);
    Arena managed_arena = make_arena(managed_allocator);
    // List_String_Const_u8 *msg_list = scope_attachment(app, scope, buffer_errors, List_String_Const_u8);
    
    for (i32 line = 1;; line += 1){
        b32 output_jump = false;
        i32 colon_index = 0;
        b32 is_sub_error = false;
        Buffer_ID out_buffer_id = 0;
        i64 out_pos = 0;
        {
            Temp_Memory_Block line_auto_closer(arena);
            if (is_valid_line(app, buffer, line)){
                String_Const_u8 line_str = push_buffer_line(app, arena, buffer, line);
                Parsed_Jump parsed_jump = try_language_jump(line_str);
                if (parsed_jump.success){
                    Buffer_ID jump_buffer = {};
                    if (open_file(app, &jump_buffer, parsed_jump.location.file, false, true)){
                        if (buffer_exists(app, jump_buffer)){
                            Buffer_Cursor cursor = buffer_compute_cursor(app, jump_buffer, seek_jump(parsed_jump));
                            if (cursor.line > 0){
                                out_buffer_id = jump_buffer;
                                out_pos = cursor.pos;
                                output_jump = true;
                            }
                        }
                    }
                }
            }
            else{
                break;
            }
        }
        
        if (output_jump){
            Sticky_Jump_Node *jump = push_array(arena, Sticky_Jump_Node, 1);
            sll_queue_push(jump_first, jump_last, jump);
            jump_count += 1;
            jump->jump.list_line = line;
            jump->jump.list_colon_index = colon_index;
            jump->jump.is_sub_error =  is_sub_error;
            jump->jump.jump_buffer_id = out_buffer_id;
            jump->jump.jump_pos = out_pos;
        }
    }
    
    Sticky_Jump_Array result = {};
    result.count = jump_count;
    result.jumps = push_array(arena, Sticky_Jump, result.count);
    i32 index = 0;
    for (Sticky_Jump_Node *node = jump_first;
         node != 0;
         node = node->next){
        result.jumps[index] = node->jump;
        index += 1;
    }
    
    return(result);
}

internal void language_init_marker_list(Application_Links *app, Heap *heap, Buffer_ID buffer, Marker_List *list){
    Scratch_Block scratch(app);
    
    Sticky_Jump_Array jumps = language_parse_buffer_to_jump_array(app, scratch, buffer);
    Range_i32_Array buffer_ranges = get_ranges_of_duplicate_keys(scratch, &jumps.jumps->jump_buffer_id, sizeof(*jumps.jumps), jumps.count);
    Sort_Pair_i32 *range_index_buffer_id_pairs = push_array(scratch, Sort_Pair_i32, buffer_ranges.count);
    for (i32 i = 0; i < buffer_ranges.count; i += 1){
        range_index_buffer_id_pairs[i].index = i;
        range_index_buffer_id_pairs[i].key = jumps.jumps[buffer_ranges.ranges[i].first].jump_buffer_id;
    }
    sort_pairs_by_key(range_index_buffer_id_pairs, buffer_ranges.count);
    Range_i32_Array scoped_buffer_ranges = get_ranges_of_duplicate_keys(scratch,
                                                                        &range_index_buffer_id_pairs->key,
                                                                        sizeof(*range_index_buffer_id_pairs),
                                                                        buffer_ranges.count);
    
    Sticky_Jump_Stored *stored = push_array(scratch, Sticky_Jump_Stored, jumps.count);
    
    Managed_Scope scope_array[2] = {};
    scope_array[0] = buffer_get_managed_scope(app, buffer);
    
    for (i32 i = 0; i < scoped_buffer_ranges.count; i += 1){
        Range_i32 buffer_range_indices = scoped_buffer_ranges.ranges[i];
        
        u32 total_jump_count = 0;
        for (i32 j = buffer_range_indices.first;
             j < buffer_range_indices.one_past_last;
             j += 1){
            i32 range_index = range_index_buffer_id_pairs[j].index;
            Range_i32 range = buffer_ranges.ranges[range_index];
            total_jump_count += range_size(range);
        }
        
        Temp_Memory marker_temp = begin_temp(scratch);
        Marker *markers = push_array(scratch, Marker, total_jump_count);
        Buffer_ID target_buffer_id = 0;
        u32 marker_index = 0;
        for (i32 j = buffer_range_indices.first;
             j < buffer_range_indices.one_past_last;
             j += 1){
            i32 range_index = range_index_buffer_id_pairs[j].index;
            Range_i32 range = buffer_ranges.ranges[range_index];
            if (target_buffer_id == 0){
                target_buffer_id = jumps.jumps[range.first].jump_buffer_id;
            }
            for (i32 k = range.first; k < range.one_past_last; k += 1){
                markers[marker_index].pos = jumps.jumps[k].jump_pos;
                markers[marker_index].lean_right = false;
                stored[k].list_line        = jumps.jumps[k].list_line;
                stored[k].list_colon_index = jumps.jumps[k].list_colon_index;
                stored[k].is_sub_error     = jumps.jumps[k].is_sub_error;
                stored[k].jump_buffer_id   = jumps.jumps[k].jump_buffer_id;
                stored[k].index_into_marker_array = marker_index;
                marker_index += 1;
            }
        }
        
        scope_array[1] = buffer_get_managed_scope(app, target_buffer_id);
        Managed_Scope scope = get_managed_scope_with_multiple_dependencies(app, scope_array, ArrayCount(scope_array));
        Managed_Object marker_handle = alloc_buffer_markers_on_buffer(app, target_buffer_id, total_jump_count, &scope);
        managed_object_store_data(app, marker_handle, 0, total_jump_count, markers);
        
        end_temp(marker_temp);
        
        Assert(managed_object_get_item_size(app, marker_handle) == sizeof(Marker));
        Assert(managed_object_get_item_count(app, marker_handle) == total_jump_count);
        Assert(managed_object_get_type(app, marker_handle) == ManagedObjectType_Markers);
        
        Managed_Object *marker_handle_ptr = scope_attachment(app, scope, sticky_jump_marker_handle, Managed_Object);
        if (marker_handle_ptr != 0){
            *marker_handle_ptr = marker_handle;
        }
    }
    
    Managed_Object stored_jump_array = alloc_managed_memory_in_scope(app, scope_array[0], sizeof(Sticky_Jump_Stored), jumps.count);
    managed_object_store_data(app, stored_jump_array, 0, jumps.count, stored);
    
    list->jump_array = stored_jump_array;
    list->jump_count = jumps.count;
    list->previous_size = (i32)buffer_get_size(app, buffer);
    list->buffer_id = buffer;
}

internal Marker_List* language_get_or_make_list_for_buffer(Application_Links *app, Heap *heap, Buffer_ID buffer_id){
    Marker_List *result = get_marker_list_for_buffer(buffer_id);
    if (result != 0){
        i32 buffer_size = (i32)buffer_get_size(app, buffer_id);
        // TODO(allen):  // TODO(allen): // TODO(allen): // TODO(allen): // TODO(allen): When buffers get an "edit sequence number" use that instead.
        if (result->previous_size != buffer_size){
            delete_marker_list(result);
            result = 0;
        }
    }
    if (result == 0){
        result = make_new_marker_list_for_buffer(heap, buffer_id);
        language_init_marker_list(app, heap, buffer_id, result);
        if (result->jump_count == 0){
            delete_marker_list(result);
            result = 0;
        }
    }
    return(result);
}


CUSTOM_COMMAND_SIG(language_if_read_only_goto_position)
CUSTOM_DOC("If the buffer in the active view is writable, inserts a character, otherwise performs goto_jump_at_cursor.")
{
    View_ID view = get_active_view(app, Access_ReadVisible);
    Buffer_ID buffer = view_get_buffer(app, view, Access_ReadWriteVisible);
    if (buffer == 0){
        buffer = view_get_buffer(app, view, Access_ReadVisible);
        if (buffer != 0){
            language_goto_jump_at_cursor(app);
            lock_jump_buffer(app, buffer);
        }
    }
    else{
        leave_current_input_unhandled(app);
    }
}

CUSTOM_COMMAND_SIG(language_if_read_only_goto_position_same_panel)
CUSTOM_DOC("If the buffer in the active view is writable, inserts a character, otherwise performs goto_jump_at_cursor_same_panel.")
{
    View_ID view = get_active_view(app, Access_ReadVisible);
    Buffer_ID buffer = view_get_buffer(app, view, Access_ReadWriteVisible);
    if (buffer == 0){
        buffer = view_get_buffer(app, view, Access_ReadVisible);
        if (buffer != 0){
            language_goto_jump_at_cursor_same_panel(app);
            lock_jump_buffer(app, buffer);
        }
    }
    else{
        leave_current_input_unhandled(app);
    }
}

/***** HOOKS *****/

function b32 language_begin_buffer__determine_language(Application_Links *app, Buffer_ID buffer_id)
{
    Scratch_Block scratch(app);
    String_Const_u8 file_name = push_buffer_file_name(app, scratch, buffer_id);
    Language **language = buffer_get_language(app, buffer_id);
    
    b32 treat_as_code = false;
    if (!*language && file_name.size > 0){
        String_Const_u8 ext = string_file_extension(file_name);
        *language = language_from_extension(ext);
    }
    if (*language) treat_as_code = true;
    return treat_as_code;
}

function void language_begin_buffer__launch_lexer(Application_Links *app, Buffer_ID buffer_id)
{
    Managed_Scope scope = buffer_get_managed_scope(app, buffer_id);
    Async_Task *lex_task_ptr = scope_attachment(app, scope, buffer_lex_task, Async_Task);
    *lex_task_ptr = async_task_no_dep(&global_async_system, language_do_full_lex_async, make_data_struct(&buffer_id));
}

function void language_init_buffer(Application_Links *app, Buffer_ID buffer_id)
{
    Scratch_Block scratch(app);
    
    Managed_Scope scope = buffer_get_managed_scope(app, buffer_id);
    
    b32 treat_as_code = false;
    b32 auto_load = def_get_config_b32(vars_save_string_lit("automatically_load_project"));
    Variable_Handle prj_var = vars_read_key(vars_get_root(), vars_save_string_lit("prj_config"));
    if (auto_load && vars_is_nil(prj_var)){
        load_project(app);
    }
    
    treat_as_code = language_begin_buffer__determine_language(app, buffer_id);
    
    String_ID file_map_id = vars_save_string_lit("keys_file");
    String_ID code_map_id = vars_save_string_lit("keys_code");
    String_ID map_id = (treat_as_code)?(code_map_id):(file_map_id);
    String_ID *map_id_ptr = scope_attachment(app, scope, buffer_map_id, String_ID);
    *map_id_ptr = map_id;
    
    Line_Ending_Kind setting = guess_line_ending_kind_from_buffer(app, buffer_id);
    Line_Ending_Kind *eol_setting = scope_attachment(app, scope, buffer_eol_setting, Line_Ending_Kind);
    *eol_setting = setting;
    
    // NOTE(allen): Decide buffer settings
    b32 wrap_lines = true;
    b32 use_lexer = false;
    if (treat_as_code){
        wrap_lines = def_get_config_b32(vars_save_string_lit("enable_code_wrapping"));
        use_lexer = true;
    }
    
    String_Const_u8 buffer_name = push_buffer_base_name(app, scratch, buffer_id);
    if (string_match(buffer_name, string_u8_litexpr("*compilation*"))){
        wrap_lines = false;
    }
    
    if (use_lexer){
        ProfileBlock(app, "begin buffer kick off lexer");
        language_begin_buffer__launch_lexer(app, buffer_id);
    }
    
    {
        b32 *wrap_lines_ptr = scope_attachment(app, scope, buffer_wrap_lines, b32);
        *wrap_lines_ptr = wrap_lines;
    }
    
    if (use_lexer){
        buffer_set_layout(app, buffer_id, layout_virt_indent_index_generic);
    }
    else{
        if (treat_as_code){
            buffer_set_layout(app, buffer_id, layout_virt_indent_literal_generic);
        }
        else{
            buffer_set_layout(app, buffer_id, layout_generic);
        }
    }
}

BUFFER_HOOK_SIG(language_begin_buffer){
    ProfileScope(app, "[Language] begin buffer");
    
    language_run_hooks(PreBeginBuffer, app, buffer_id);
    language_init_buffer(app, buffer_id);
    language_run_hooks(PostBeginBuffer, app, buffer_id);
    
    // no meaning for return
    return 0;
}

Token_List language_buffer_edit_range__relex(Application_Links *app, Buffer_ID buffer_id, Arena *arena, String_Const_u8 text)
{
    Language **language = buffer_get_language(app, buffer_id);
    return (*language)->lex_full_input(arena, text);
}

BUFFER_EDIT_RANGE_SIG(language_buffer_edit_range){
    // buffer_id, new_range, original_size
    ProfileScope(app, "[Language] Buffer Edit Range");
    
    Range_i64 old_range = Ii64(old_cursor_range.min.pos, old_cursor_range.max.pos);
    buffer_shift_fade_ranges(buffer_id, old_range.max, (new_range.max - old_range.max));
    
    {
        code_index_lock();
        Code_Index_File *file = code_index_get_file(buffer_id);
        if (file != 0){
            code_index_shift(file, old_range, range_size(new_range));
        }
        code_index_unlock();
    }
    
    i64 insert_size = range_size(new_range);
    i64 text_shift = replace_range_shift(old_range, insert_size);
    
    Scratch_Block scratch(app);
    
    Managed_Scope scope = buffer_get_managed_scope(app, buffer_id);
    Async_Task *lex_task_ptr = scope_attachment(app, scope, buffer_lex_task, Async_Task);
    
    Base_Allocator *allocator = managed_scope_allocator(app, scope);
    b32 do_full_relex = false;
    
    if (async_task_is_running_or_pending(&global_async_system, *lex_task_ptr)){
        async_task_cancel(app, &global_async_system, *lex_task_ptr);
        buffer_unmark_as_modified(buffer_id);
        do_full_relex = true;
        *lex_task_ptr = 0;
    }
    
    Token_Array *ptr = scope_attachment(app, scope, attachment_tokens, Token_Array);
    if (ptr != 0 && ptr->tokens != 0){
        ProfileBlockNamed(app, "attempt resync", profile_attempt_resync);
        
        i64 token_index_first = token_relex_first(ptr, old_range.first, 1);
        i64 token_index_resync_guess =
            token_relex_resync(ptr, old_range.one_past_last, 32);
        
        if (token_index_resync_guess - token_index_first >= 4000){
            do_full_relex = true;
        }
        else{
            Token *token_first = ptr->tokens + token_index_first;
            Token *token_resync = ptr->tokens + token_index_resync_guess;
            
            Range_i64 relex_range = Ii64(token_first->pos, token_resync->pos + token_resync->size + text_shift);
            String_Const_u8 partial_text = push_buffer_range(app, scratch, buffer_id, relex_range);
            
            Token_List relex_list = language_buffer_edit_range__relex(app, buffer_id, scratch, partial_text);
            if (relex_range.one_past_last < buffer_get_size(app, buffer_id)){
                token_drop_eof(&relex_list);
            }
            
            Token_Relex relex = token_relex(relex_list, relex_range.first - text_shift, ptr->tokens, token_index_first, token_index_resync_guess);
            
            ProfileCloseNow(profile_attempt_resync);
            
            if (!relex.successful_resync){
                do_full_relex = true;
            }
            else{
                ProfileBlock(app, "apply resync");
                
                i64 token_index_resync = relex.first_resync_index;
                
                Range_i64 head = Ii64(0, token_index_first);
                Range_i64 replaced = Ii64(token_index_first, token_index_resync);
                Range_i64 tail = Ii64(token_index_resync, ptr->count);
                i64 resynced_count = (token_index_resync_guess + 1) - token_index_resync;
                i64 relexed_count = relex_list.total_count - resynced_count;
                i64 tail_shift = relexed_count - (token_index_resync - token_index_first);
                
                i64 new_tokens_count = ptr->count + tail_shift;
                Token *new_tokens = base_array(allocator, Token, new_tokens_count);
                
                Token *old_tokens = ptr->tokens;
                block_copy_array_shift(new_tokens, old_tokens, head, 0);
                token_fill_memory_from_list(new_tokens + replaced.first, &relex_list, relexed_count);
                for (i64 i = 0, index = replaced.first; i < relexed_count; i += 1, index += 1){
                    new_tokens[index].pos += relex_range.first;
                }
                for (i64 i = tail.first; i < tail.one_past_last; i += 1){
                    old_tokens[i].pos += text_shift;
                }
                block_copy_array_shift(new_tokens, ptr->tokens, tail, tail_shift);
                
                base_free(allocator, ptr->tokens);
                
                ptr->tokens = new_tokens;
                ptr->count = new_tokens_count;
                ptr->max = new_tokens_count;
                
                buffer_mark_as_modified(buffer_id);
            }
        }
    }
    
    if (do_full_relex){
        *lex_task_ptr = async_task_no_dep(&global_async_system, language_do_full_lex_async,
                                          make_data_struct(&buffer_id));
    }
    
    // no meaning for return
    return(0);
}

static void language_render_buffer(Application_Links *app, View_ID view_id, Face_ID face_id,
                                   Buffer_ID buffer, Text_Layout_ID text_layout_id,
                                   Rect_f32 rect)
{
    ProfileScope(app, "[Language] render buffer");
    
    View_ID active_view = get_active_view(app, Access_Always);
    b32 is_active_view = active_view == view_id;
    Rect_f32 prev_clip = draw_set_clip(app, rect);
    
    // NOTE(allen): Cursor shape
    Face_Metrics metrics = get_face_metrics(app, face_id);
    f32 cursor_roundness = (metrics.normal_advance*0.5f)*0.9f;
    f32 mark_thickness = 2.f;
    
    // Language **language = buffer_get_language(app, buffer);
    Token_Array token_array = get_token_array_from_buffer(app, buffer);
    if (token_array.tokens != 0)
    {
        language_paint_tokens(app, buffer, text_layout_id, &token_array);
        // NOTE(allen): Scan for TODOs and NOTEs
        b32 use_comment_keyword = def_get_config_b32(vars_save_string_lit("use_comment_keyword"));
        if (use_comment_keyword){
            Comment_Highlight_Pair pairs[] = {
                {string_u8_litexpr("NOTE"), finalize_color(defcolor_comment_pop, 0)},
                {string_u8_litexpr("TODO"), finalize_color(defcolor_comment_pop, 1)},
            };
            draw_comment_highlights(app, buffer, text_layout_id,
                                    &token_array, pairs, ArrayCount(pairs));
        }
    }
    else
    {
        Range_i64 visible_range = text_layout_get_visible_range(app, text_layout_id);
        paint_text_color_fcolor(app, text_layout_id, visible_range, fcolor_id(defcolor_text_default));
    }
    
    i64 cursor_pos = view_correct_cursor(app, view_id);
    view_correct_mark(app, view_id);
    
    // NOTE(allen): Scope highlight
    b32 use_scope_highlight = def_get_config_b32(vars_save_string_lit("use_scope_highlight"));
    if (use_scope_highlight){
        Color_Array colors = finalize_color_array(defcolor_back_cycle);
        draw_scope_highlight(app, buffer, text_layout_id, cursor_pos, colors.vals, colors.count);
    }
    
    b32 use_error_highlight = def_get_config_b32(vars_save_string_lit("use_error_highlight"));
    b32 use_jump_highlight = def_get_config_b32(vars_save_string_lit("use_jump_highlight"));
    if (use_error_highlight || use_jump_highlight){
        // NOTE(allen): Error highlight
        String_Const_u8 name = string_u8_litexpr("*compilation*");
        Buffer_ID compilation_buffer = get_buffer_by_name(app, name, Access_Always);
        if (use_error_highlight){
            draw_jump_highlights(app, buffer, text_layout_id, compilation_buffer,
                                 fcolor_id(defcolor_highlight_junk));
        }
        
        // NOTE(allen): Search highlight
        if (use_jump_highlight){
            Buffer_ID jump_buffer = get_locked_jump_buffer(app);
            if (jump_buffer != compilation_buffer){
                draw_jump_highlights(app, buffer, text_layout_id, jump_buffer,
                                     fcolor_id(defcolor_highlight_white));
            }
        }
    }
    
    // NOTE(allen): Color parens
    b32 use_paren_helper = def_get_config_b32(vars_save_string_lit("use_paren_helper"));
    if (use_paren_helper){
        Color_Array colors = finalize_color_array(defcolor_text_cycle);
        draw_paren_highlight(app, buffer, text_layout_id, cursor_pos, colors.vals, colors.count);
    }
    
    // NOTE(allen): Line highlight
    b32 highlight_line_at_cursor = def_get_config_b32(vars_save_string_lit("highlight_line_at_cursor"));
    if (highlight_line_at_cursor && is_active_view){
        i64 line_number = get_line_number_from_pos(app, buffer, cursor_pos);
        draw_line_highlight(app, text_layout_id, line_number,
                            fcolor_id(defcolor_highlight_cursor_line));
    }
    
    // NOTE(allen): Whitespace highlight
    b64 show_whitespace = false;
    view_get_setting(app, view_id, ViewSetting_ShowWhitespace, &show_whitespace);
    if (show_whitespace){
        if (token_array.tokens == 0){
            draw_whitespace_highlight(app, buffer, text_layout_id, cursor_roundness);
        }
        else{
            draw_whitespace_highlight(app, text_layout_id, &token_array, cursor_roundness);
        }
    }
    
    // NOTE(allen): Cursor
    switch (fcoder_mode){
        case FCoderMode_Original:
        {
            draw_original_4coder_style_cursor_mark_highlight(app, view_id, is_active_view, buffer, text_layout_id, cursor_roundness, mark_thickness);
        } break;
        case FCoderMode_NotepadLike:
        {
            draw_notepad_style_cursor_highlight(app, view_id, buffer, text_layout_id, cursor_roundness);
        } break;
    }
    
    // NOTE(allen): Fade ranges
    paint_fade_ranges(app, text_layout_id, buffer);
    
    // NOTE(allen): put the actual text on the actual screen
    draw_text_layout_default(app, text_layout_id);
    
    draw_set_clip(app, prev_clip);
}

function void language_render_caller(Application_Links *app, Frame_Info frame_info, View_ID view_id){
    ProfileScope(app, "[Language] render caller");
    View_ID active_view = get_active_view(app, Access_Always);
    b32 is_active_view = (active_view == view_id);
    
    Rect_f32 region = draw_background_and_margin(app, view_id, is_active_view);
    Rect_f32 prev_clip = draw_set_clip(app, region);
    
    Buffer_ID buffer = view_get_buffer(app, view_id, Access_Always);
    Face_ID face_id = get_face_id(app, buffer);
    Face_Metrics face_metrics = get_face_metrics(app, face_id);
    f32 line_height = face_metrics.line_height;
    f32 digit_advance = face_metrics.decimal_digit_advance;
    
    // NOTE(allen): file bar
    b64 showing_file_bar = false;
    if (view_get_setting(app, view_id, ViewSetting_ShowFileBar, &showing_file_bar) && showing_file_bar){
        Rect_f32_Pair pair = layout_file_bar_on_top(region, line_height);
        draw_file_bar(app, view_id, buffer, face_id, pair.min);
        region = pair.max;
    }
    
    Buffer_Scroll scroll = view_get_buffer_scroll(app, view_id);
    
    Buffer_Point_Delta_Result delta = delta_apply(app, view_id,
                                                  frame_info.animation_dt, scroll);
    if (!block_match_struct(&scroll.position, &delta.point)){
        block_copy_struct(&scroll.position, &delta.point);
        view_set_buffer_scroll(app, view_id, scroll, SetBufferScroll_NoCursorChange);
    }
    if (delta.still_animating){
        animate_in_n_milliseconds(app, 0);
    }
    
    // NOTE(allen): query bars
    region = default_draw_query_bars(app, region, view_id, face_id);
    
    // NOTE(allen): FPS hud
    if (show_fps_hud){
        Rect_f32_Pair pair = layout_fps_hud_on_bottom(region, line_height);
        draw_fps_hud(app, frame_info, face_id, pair.max);
        region = pair.min;
        animate_in_n_milliseconds(app, 1000);
    }
    
    // NOTE(allen): layout line numbers
    Rect_f32 line_number_rect = {};
    b32 show_line_number_margins = def_get_config_b32(vars_save_string_lit("show_line_number_margins"));
    if (show_line_number_margins){
        Rect_f32_Pair pair = layout_line_number_margin(app, buffer, region, digit_advance);
        line_number_rect = pair.min;
        region = pair.max;
    }
    
    // NOTE(allen): begin buffer render
    Buffer_Point buffer_point = scroll.position;
    Text_Layout_ID text_layout_id = text_layout_create(app, buffer, region, buffer_point);
    
    // NOTE(allen): draw line numbers
    if (show_line_number_margins){
        draw_line_number_margin(app, view_id, buffer, face_id, text_layout_id, line_number_rect);
    }
    
    // NOTE(allen): draw the buffer
    language_render_buffer(app, view_id, face_id, buffer, text_layout_id, region);
    
    text_layout_free(app, text_layout_id);
    draw_set_clip(app, prev_clip);
}

function void language_tick(Application_Links *app, Frame_Info frame_info)
{
    language_code_index_update_tick(app);
    language_run_hooks(Tick, app, frame_info);
    if (tick_all_fade_ranges(app, frame_info.animation_dt))
        animate_in_n_milliseconds(app, 0);
}

/***** HELPERS *****/
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
    for (Language *lang = languages.first;
         lang != 0;
         lang = lang->next)
    {
        for (i32 e = 0; e < lang->file_extensions.count; e++)
        {
            if (string_match(ext, lang->file_extensions.strings[e]))
            {
                return lang;
            }
        }
    }
    return 0;
}

function Language *language_from_name(String_Const_u8 name)
{
    for (Language *lang = languages.first;
         lang != 0;
         lang = lang->next)
    {
        if (string_match_insensitive(name, lang->name))
        {
            return lang;
        }
    }
    return 0;
}

CUSTOM_COMMAND_SIG(set_language)
CUSTOM_DOC("Set the language for the current buffer.")
{
    View_ID view = get_active_view(app, Access_Always);
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
    ((Buffer_Hook_Function*)get_custom_hook(app, HookID_BeginBuffer))(app, buffer); // @note(tyler): Must re-init with new language
}

CUSTOM_COMMAND_SIG(print_language)
CUSTOM_DOC("Print the language for the current buffer.")
{
    View_ID view = get_active_view(app, Access_Always);
    Buffer_ID buffer = view_get_buffer(app, view, Access_Always);
    
    Scratch_Block scratch(app);
    
    Language *lang = *buffer_get_language(app, buffer);
    print_message(app, push_stringf(scratch, "Language is set to %.*s\n", string_expand(lang->name)));
}

function b32 language_line_comment_starts_at_position(Application_Links *app, Buffer_ID buffer, i64 pos, Language *lang)
{
    b32 already_has_comment = false;
    u8 check_buffer[64] = {0};
    if (buffer_read_range(app, buffer, Ii64(pos, pos + lang->comment_delims.line.size), check_buffer)) {
        print_message(app, SCu8(check_buffer));
        if (string_match(SCu8(check_buffer), lang->comment_delims.line))
            already_has_comment = true;
    }
    return(already_has_comment);
}

CUSTOM_COMMAND_SIG(language_comment_line)
CUSTOM_DOC("Comment the current line with the current language's delimeters.")
{
    View_ID view = get_active_view(app, Access_ReadWriteVisible);
    Buffer_ID buffer = view_get_buffer(app, view, Access_ReadWriteVisible);
    i64 pos = get_start_of_line_at_cursor(app, view, buffer);
    Language *lang = *buffer_get_language(app, buffer);
    b32 already_has_comment = language_line_comment_starts_at_position(app, buffer, pos, lang);
    if (!already_has_comment)
        buffer_replace_range(app, buffer, Ii64(pos), lang->comment_delims.line);
}


CUSTOM_COMMAND_SIG(language_uncomment_line)
CUSTOM_DOC("Comment the current line with the current language's delimeters.")
{
    View_ID view = get_active_view(app, Access_ReadWriteVisible);
    Buffer_ID buffer = view_get_buffer(app, view, Access_ReadWriteVisible);
    i64 pos = get_start_of_line_at_cursor(app, view, buffer);
    Language *lang = *buffer_get_language(app, buffer);
    b32 already_has_comment = language_line_comment_starts_at_position(app, buffer, pos, lang);
    if (already_has_comment)
        buffer_replace_range(app, buffer, Ii64(pos, pos + lang->comment_delims.line.size), string_u8_empty);
}

CUSTOM_COMMAND_SIG(language_comment_line_toggle)
CUSTOM_DOC("Comment the current line with the current language's delimeters.")
{
    View_ID view = get_active_view(app, Access_ReadWriteVisible);
    Buffer_ID buffer = view_get_buffer(app, view, Access_ReadWriteVisible);
    i64 pos = get_start_of_line_at_cursor(app, view, buffer);
    Language *lang = *buffer_get_language(app, buffer);
    b32 already_has_comment = language_line_comment_starts_at_position(app, buffer, pos, lang);
    if (already_has_comment)
        buffer_replace_range(app, buffer, Ii64(pos, pos + lang->comment_delims.line.size), string_u8_empty);
    else
        buffer_replace_range(app, buffer, Ii64(pos), lang->comment_delims.line);
}

CUSTOM_COMMAND_SIG(language_comment_range)
CUSTOM_DOC("Comment the current range according the current language's block comment delimiters.")
{
    View_ID view = get_active_view(app, Access_ReadWriteVisible);
    Buffer_ID buffer = view_get_buffer(app, view, Access_ReadWriteVisible);
    Language *lang = *buffer_get_language(app, buffer);
    
    String_Const_u8 begin = lang->comment_delims.block_start;
    String_Const_u8 end   = lang->comment_delims.block_end;
    
    Range_i64 range = get_view_range(app, view);
    Range_i64 lines = get_line_range_from_pos_range(app, buffer, range);
    range = get_pos_range_from_line_range(app, buffer, lines);
    
    Scratch_Block scratch(app);
    
    b32 min_line_blank = line_is_valid_and_blank(app, buffer, lines.min);
    b32 max_line_blank = line_is_valid_and_blank(app, buffer, lines.max);
    
    if ((lines.min < lines.max) || (!min_line_blank)){
        String_Const_u8 begin_str = {};
        String_Const_u8 end_str = {};
        
        i64 min_adjustment = 0;
        i64 max_adjustment = 0;
        
        if (min_line_blank){
            begin_str = push_u8_stringf(scratch, "\n%.*s", string_expand(begin));
            min_adjustment += 1;
        }
        else{
            begin_str = push_u8_stringf(scratch, "%.*s\n", string_expand(begin));
        }
        if (max_line_blank){
            end_str = push_u8_stringf(scratch, "%.*s\n", string_expand(end));
        }
        else{
            end_str = push_u8_stringf(scratch, "\n%.*s", string_expand(end));
            max_adjustment += 1;
        }
        
        max_adjustment += begin_str.size;
        Range_i64 new_pos = Ii64(range.min + min_adjustment, range.max + max_adjustment);
        
        History_Group group = history_group_begin(app, buffer);
        buffer_replace_range(app, buffer, Ii64(range.min), begin_str);
        buffer_replace_range(app, buffer, Ii64(range.max + begin_str.size), end_str);
        history_group_end(group);
        
        set_view_range(app, view, new_pos);
    }
    else{
        String_Const_u8 str = push_u8_stringf(scratch, "%.*s\n\n%.*s", string_expand(begin), string_expand(end));
        buffer_replace_range(app, buffer, range, str);
        i64 center_pos = range.min + begin.size + 1;
        view_set_cursor_and_preferred_x(app, view, seek_pos(center_pos));
        view_set_mark(app, view, seek_pos(center_pos));
    }
}

function i64 get_column_from_pos(Application_Links *app, Buffer_ID buffer, i64 pos)
{
    Buffer_Cursor cursor = buffer_compute_cursor(app, buffer, seek_pos(pos));
    return(cursor.col);
}

function void indent_nest_list(Application_Links *app, Buffer_ID buffer, Code_Index_Nest_List nests, i64 *indentations, i32 base_indent, i32 tab_width)
{
    for (Code_Index_Nest *current_nest = nests.first;
         current_nest != 0 ;
         current_nest = current_nest->next)
    {
        switch (current_nest->kind)
        {
            case CodeIndexNest_Scope: {
                i64 start = get_line_number_from_pos(app, buffer, current_nest->open.min);
                i64 end   = get_line_number_from_pos(app, buffer, current_nest->close.min);
                i64 count = end - start - 1;
                
                i32 new_indent = base_indent + tab_width;
                block_fill_u64(indentations+start, sizeof(*indentations)*count, (u64)(new_indent));
                indent_nest_list(app, buffer, current_nest->nest_list, indentations, new_indent, tab_width);
            } break;
            
            case CodeIndexNest_Paren: {
                i64 start = get_line_number_from_pos(app, buffer, current_nest->open.min);
                i64 end   = get_line_number_from_pos(app, buffer, current_nest->close.min);
                i64 count = end - start - 1;
                
                i64 column = Max(0, get_column_from_pos(app, buffer, current_nest->open.max)-1);// + indentations[start-1];
                block_fill_u64(indentations+start, sizeof(*indentations)*count, (u64)(column));
                indent_nest_list(app, buffer, current_nest->nest_list, indentations, (i32)column, tab_width);
            } break;
            
            case CodeIndexNest_Statement: {
                i64 start = get_line_number_from_pos(app, buffer, current_nest->open.min);
                i64 end   = get_line_number_from_pos(app, buffer, current_nest->close.min);
                i64 count = end - start;
                
                i32 new_indent = base_indent + tab_width;
                block_fill_u64(indentations+start, sizeof(*indentations)*count, (u64)(new_indent));
                indent_nest_list(app, buffer, current_nest->nest_list, indentations, new_indent, tab_width);
            } break;
        }
        
    }
}

function i64 *get_indentation_array_from_index(Application_Links *app, Arena *arena, Buffer_ID buffer, Range_i64 lines, Indent_Flag flags, i32 tab_width, i32 indent_width)
{
    i64 count = lines.max - lines.min + 1;
    i64 *indentations = push_array(arena, i64, count);
    //     i64 *shifted_indentations = indentations - lines.first;
    block_fill_u64(indentations, sizeof(*indentations)*count, (u64)(0));
    
    Code_Index_File *file = code_index_get_file(buffer);
    if (file == 0)
        return indentations;
    
    Code_Index_Nest_List nests = file->nest_list;
    indent_nest_list(app, buffer, nests, indentations, 1, tab_width);
    return indentations;
}

function void language_add_extension(Language *lang, Extension_Support ext)
{
    Data str_data = *(Data *)&ext.ext_name;
    Data ext_data = push_data_copy(&language_arena, make_data_struct(&ext));
    if (!lang->extension_support.allocator)
        lang->extension_support = make_table_Data_Data(language_arena.base_allocator, 32);
    table_insert(&lang->extension_support, str_data, ext_data);
}

function void language_add_extension(String_Const_u8 name, Extension_Support ext)
{
    language_add_extension(language_from_name(name), ext);
}

function Extension_Support *language_get_extension(Language *lang, String_Const_u8 ext_name)
{
    Data str_data = *(Data *)&ext_name;
    Data ext_data;
    Table_Lookup lookup = table_lookup(&lang->extension_support, str_data);
    b32 res = table_read(&lang->extension_support, lookup, &ext_data);
    if (res)
        return (Extension_Support*)ext_data.str;
    return 0;
}

function Extension_Support *language_get_extension(String_Const_u8 lang_name, String_Const_u8 ext_name)
{
    return language_get_extension(language_from_name(lang_name), ext_name);
}

function void language_push_hook(Language_Hook_Kind kind, Void_Func *f)
{
    Hook *hook = push_array_zero(&language_arena, Hook, 1);
    hook->f = f;
    Hook_List *list = &LANGUAGE_HOOKS[kind];
    if (!list->last)
    {
        list->first = list->last = hook;
    }
    else
    {
        list->last->next = hook;
        list->last = hook;
    }
    list->count++;
}

CUSTOM_COMMAND_SIG(token_at_cursor)
CUSTOM_DOC("Shows info about the token under the cursor.")
{
    View_ID view = get_active_view(app, Access_Always);
    i64 cursor = view_get_cursor_pos(app, view);
    Buffer_ID buffer = view_get_buffer(app, view, Access_Always);
    
    Token *token = get_token_from_pos(app, buffer, cursor);
    
    Managed_Scope scope = buffer_get_managed_scope(app, buffer);
    Language **language = scope_attachment(app, scope, buffer_language, Language*);
    
    char *base_name = token_base_kind_names[token->kind];
    char *sub_name = (*language)->token_kind_names[token->sub_kind];
    
    char message[512];
    snprintf(message, 512, "Token:\n  Base: %s (%d)\n  Sub: %s (%d)\n", base_name, token->kind, sub_name, token->sub_kind);
    print_message(app, SCu8(message));
}

CUSTOM_COMMAND_SIG(language_goto_jump_at_cursor)
CUSTOM_DOC("Language specific goto_jump_at_cursor")
{
    Heap *heap = &global_heap;
    
    View_ID view = get_active_view(app, Access_ReadVisible);
    Buffer_ID buffer = view_get_buffer(app, view, Access_ReadVisible);
    Marker_List *list = language_get_or_make_list_for_buffer(app, heap, buffer);
    
    i64 pos = view_get_cursor_pos(app, view);
    Buffer_Cursor cursor = buffer_compute_cursor(app, buffer, seek_pos(pos));
    
    i32 list_index = get_index_exact_from_list(app, list, cursor.line);
    
    if (list_index >= 0){
        ID_Pos_Jump_Location location = {};
        if (get_jump_from_list(app, list, list_index, &location)){
            if (get_jump_buffer(app, &buffer, &location)){
                change_active_panel(app);
                View_ID target_view = get_active_view(app, Access_Always);
                switch_to_existing_view(app, target_view, buffer);
                jump_to_location(app, target_view, buffer, location);
                
            }
        }
    }
}

CUSTOM_COMMAND_SIG(language_goto_jump_at_cursor_same_panel)
CUSTOM_DOC("Language specific goto_jump_at_cursor_same_panel")
{
    Heap *heap = &global_heap;
    
    View_ID view = get_active_view(app, Access_ReadVisible);
    Buffer_ID buffer = view_get_buffer(app, view, Access_ReadVisible);
    
    // @note(tyler): This is the only change
    Marker_List *list = language_get_or_make_list_for_buffer(app, heap, buffer);
    
    i64 pos = view_get_cursor_pos(app, view);
    Buffer_Cursor cursor = buffer_compute_cursor(app, buffer, seek_pos(pos));
    
    i32 list_index = get_index_exact_from_list(app, list, cursor.line);
    
    if (list_index >= 0){
        ID_Pos_Jump_Location location = {};
        if (get_jump_from_list(app, list, list_index, &location)){
            if (get_jump_buffer(app, &buffer, &location)){
                jump_to_location(app, view, buffer, location);
            }
        }
    }
}
