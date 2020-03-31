#include <string>

static void tc_paint_tokens(Application_Links *app, Buffer_ID buffer, Text_Layout_ID text_layout_id, Token_Array *array)
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
        
        // Check if Field
        b32 field = false;
        if (token->sub_kind == TokenCppKind_Dot ||
            token->sub_kind == TokenCppKind_Arrow)
        {
            FColor color = (*language)->get_token_color(*token);
            paint_text_color_fcolor(app, text_layout_id, Ii64_size(token->pos, token->size), color);
            token_it_inc_all(&it);
            token = token_it_read(&it);
            if (token->kind == TokenBaseKind_Identifier)
                field = true;
        }
        // END: Check if Field
        
        // Check Notes
        b32 custom_note_color_used = false;
        if (token->kind == TokenBaseKind_Identifier &&
            !field)
        {
            String_Const_u8 token_as_string = push_token_lexeme(app, scratch, buffer, token);
            
            for (Buffer_ID buf = get_buffer_next(app, 0, Access_Always);
                 buf != 0;
                 buf = get_buffer_next(app, buf, Access_Always))
            {
                Code_Index_File *file = code_index_get_file(buf);
                if (file == 0)
                    continue;
                
                for (i32 i = 0; i < file->note_array.count; i++)
                {
                    Code_Index_Note *note = file->note_array.ptrs[i];
                    
                    if (string_match(note->text, token_as_string, StringMatch_Exact))
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
                                if (string_match((*language)->name, SCu8("CPP"), StringMatch_Exact))
                                {
                                    invalid = peek->sub_kind != TokenCppKind_ParenOp;
                                }
                                else if (string_match((*language)->name, SCu8("Odin"), StringMatch_Exact))
                                {
                                    invalid = peek->sub_kind != TokenOdinKind_ParenOp &&
                                        peek->sub_kind != TokenOdinKind_ColonColon;
                                }
                                
                                if (invalid) break;
                                custom_note_color_used = true;
                                
                                Range_i64 range = {};
                                range.start = token->pos;
                                range.end = token->pos + token->size;
                                paint_text_color_fcolor(app, text_layout_id, range, fcolor_id(defcolor_function_name));
                            } break;
                        }
                        break;
                    }
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

static void tc_paint_comment_keywords(Application_Links *app, Buffer_ID buffer, Text_Layout_ID text_layout_id, Token_Array *token_array)
{
    Comment_Highlight_Pair pairs[] = {
        {string_u8_litexpr("NOTE"), finalize_color(defcolor_comment_pop, 0)},
        {string_u8_litexpr("TODO"), finalize_color(defcolor_comment_pop, 1)},
    };
    draw_comment_highlights(app, buffer, text_layout_id,
                            token_array, pairs, ArrayCount(pairs));
}

static void tc_render_buffer(Application_Links *app, View_ID view_id, Face_ID face_id,
                             Buffer_ID buffer, Text_Layout_ID text_layout_id,
                             Rect_f32 rect)
{
    ProfileScope(app, "[TErickson] render buffer");
    
    View_ID active_view = get_active_view(app, Access_Always);
    b32 is_active_view = active_view == view_id;
    Rect_f32 prev_clip = draw_set_clip(app, rect);
    
    // NOTE(allen): Cursor shape
    Face_Metrics metrics = get_face_metrics(app, face_id);
    f32 cursor_roundness = (metrics.normal_advance*0.5f)*0.9f;
    f32 mark_thickness = 2.f;
    
    Token_Array token_array = get_token_array_from_buffer(app, buffer);
    if (token_array.tokens != 0)
    {
        tc_paint_tokens(app, buffer, text_layout_id, &token_array);
        if (global_config.use_comment_keyword)
            tc_paint_comment_keywords(app, buffer, text_layout_id, &token_array);
    }
    else
    {
        Range_i64 visible_range = text_layout_get_visible_range(app, text_layout_id);
        paint_text_color_fcolor(app, text_layout_id, visible_range, fcolor_id(defcolor_text_default));
    }
    
    i64 cursor_pos = view_correct_cursor(app, view_id);
    view_correct_mark(app, view_id);
    
    // NOTE(allen): Scope highlight
    if (global_config.use_scope_highlight){
        Color_Array colors = finalize_color_array(defcolor_back_cycle);
        draw_scope_highlight(app, buffer, text_layout_id, cursor_pos, colors.vals, colors.count);
    }
    
    if (global_config.use_error_highlight || global_config.use_jump_highlight){
        // NOTE(allen): Error highlight
        String_Const_u8 name = string_u8_litexpr("*compilation*");
        Buffer_ID compilation_buffer = get_buffer_by_name(app, name, Access_Always);
        if (global_config.use_error_highlight){
            draw_jump_highlights(app, buffer, text_layout_id, compilation_buffer,
                                 fcolor_id(defcolor_highlight_junk));
        }
        
        // NOTE(allen): Search highlight
        if (global_config.use_jump_highlight){
            Buffer_ID jump_buffer = get_locked_jump_buffer(app);
            if (jump_buffer != compilation_buffer){
                draw_jump_highlights(app, buffer, text_layout_id, jump_buffer,
                                     fcolor_id(defcolor_highlight_white));
            }
        }
    }
    
    // NOTE(allen): Color parens
    if (global_config.use_paren_helper){
        Color_Array colors = finalize_color_array(defcolor_text_cycle);
        draw_paren_highlight(app, buffer, text_layout_id, cursor_pos, colors.vals, colors.count);
    }
    
    // NOTE(allen): Line highlight
    if (global_config.highlight_line_at_cursor && is_active_view){
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
    paint_fade_ranges(app, text_layout_id, buffer, view_id);
    
    // NOTE(allen): put the actual text on the actual screen
    draw_text_layout_default(app, text_layout_id);
    
    draw_set_clip(app, prev_clip);
}
