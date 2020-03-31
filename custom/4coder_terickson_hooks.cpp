function void tc_render_caller(Application_Links *app, Frame_Info frame_info, View_ID view_id){
    ProfileScope(app, "[TErickson] render caller");
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
    if (global_config.show_line_number_margins){
        Rect_f32_Pair pair = layout_line_number_margin(app, buffer, region, digit_advance);
        line_number_rect = pair.min;
        region = pair.max;
    }
    
    // NOTE(allen): begin buffer render
    Buffer_Point buffer_point = scroll.position;
    Text_Layout_ID text_layout_id = text_layout_create(app, buffer, region, buffer_point);
    
    // NOTE(allen): draw line numbers
    if (global_config.show_line_number_margins){
        draw_line_number_margin(app, view_id, buffer, face_id, text_layout_id, line_number_rect);
    }
    
    // NOTE(allen): draw the buffer
    tc_render_buffer(app, view_id, face_id, buffer, text_layout_id, region);
    
    text_layout_free(app, text_layout_id);
    draw_set_clip(app, prev_clip);
}


function void
tc_do_full_lex_async__inner(Async_Context *actx, Buffer_ID buffer_id){
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
    
    // @todo(tyler): Make cancellable
    list = (*language)->lex_full_input(scratch, contents);
    
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
tc_do_full_lex_async(Async_Context *actx, Data data){
    if (data.size == sizeof(Buffer_ID)){
        Buffer_ID *buffer = (Buffer_ID*)data.data;
        
        tc_do_full_lex_async__inner(actx, *buffer);
    }
}

BUFFER_HOOK_SIG(tc_begin_buffer){
    ProfileScope(app, "[TErickson] begin buffer");
    Scratch_Block scratch(app);
    
    Managed_Scope scope = buffer_get_managed_scope(app, buffer_id);
    
    b32 treat_as_code = false;
    String_Const_u8 file_name = push_buffer_file_name(app, scratch, buffer_id);
    Language **language = scope_attachment(app, scope, buffer_language, Language*);
    *language = 0;
    
    if (file_name.size > 0){
        String_Const_u8_Array extensions = global_config.code_exts;
        String_Const_u8 ext = string_file_extension(file_name);
        for (i32 l = 0; l < LANG_COUNT; l++)
        {
            for (i32 e = 0; e < languages[l].extensions.count; e++)
            {
                if (string_match(ext, languages[l].extensions.strings[e]))
                {
                    *language = &languages[l];
                    treat_as_code = true;
                }
            }
        }
        
        // @note(tyler): Leave this for now for other file-types
        for (i32 i = 0; i < extensions.count; ++i) {
            if (string_match(ext, extensions.strings[i])) {
                treat_as_code = true;
            }
            break;
        }
    }
    
    Command_Map_ID map_id = (treat_as_code)?(mapid_code):(mapid_file);
    Command_Map_ID *map_id_ptr = scope_attachment(app, scope, buffer_map_id, Command_Map_ID);
    *map_id_ptr = map_id;
    
    Line_Ending_Kind setting = guess_line_ending_kind_from_buffer(app, buffer_id);
    Line_Ending_Kind *eol_setting = scope_attachment(app, scope, buffer_eol_setting, Line_Ending_Kind);
    *eol_setting = setting;
    
    // NOTE(allen): Decide buffer settings
    b32 wrap_lines = true;
    b32 use_lexer = false;
    if (treat_as_code){
        wrap_lines = global_config.enable_code_wrapping;
        use_lexer = true;
    }
    
    String_Const_u8 buffer_name = push_buffer_base_name(app, scratch, buffer_id);
    if (string_match(buffer_name, string_u8_litexpr("*compilation*"))){
        wrap_lines = false;
    }
    
    if (use_lexer){
        ProfileBlock(app, "begin buffer kick off lexer");
        Async_Task *lex_task_ptr = scope_attachment(app, scope, buffer_lex_task, Async_Task);
        *lex_task_ptr = async_task_no_dep(&global_async_system, tc_do_full_lex_async, make_data_struct(&buffer_id));
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
    
    // no meaning for return
    return(0);
}

BUFFER_EDIT_RANGE_SIG(tc_buffer_edit_range){
    // buffer_id, new_range, original_size
    ProfileScope(app, "[TErickson] Buffer Edit Range");
    
    Range_i64 old_range = Ii64(new_range.first, new_range.first + original_size);
    
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
    Language **language = scope_attachment(app, scope, buffer_language, Language*);
    
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
            
            Token_List relex_list = (*language)->lex_full_input(scratch, partial_text);
            /*
                        switch (*lex_kind_ptr)
                        {
                            case LEXER_CPP:  relex_list = lex_full_input_cpp(scratch, partial_text);  break;
                            case LEXER_ODIN: relex_list = lex_full_input_odin(scratch, partial_text); break;
                        }
                         */
            
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
        *lex_task_ptr = async_task_no_dep(&global_async_system, do_full_lex_async,
                                          make_data_struct(&buffer_id));
    }
    
    // no meaning for return
    return(0);
}

function void tc_code_index_update_tick(Application_Links *app)
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
        // TODO(allen): Actually determine this in a fair way.
        // Maybe switch to an enum?
        // Actually probably a pointer to a struct that defines the language.
        state.do_cpp_parse = true;
        tc_generic_parse_full_input_breaks(index, &state, max_i32);
        
        code_index_lock();
        code_index_set_file(buffer_id, arena, index);
        code_index_unlock();
        buffer_clear_layout_cache(app, buffer_id);
    }
    
    buffer_modified_set_clear();
}

function void tc_tick(Application_Links *app, Frame_Info frame_info)
{
    tc_code_index_update_tick(app);
    if (tick_all_fade_ranges(frame_info.animation_dt))
        animate_in_n_milliseconds(app, 0);
}