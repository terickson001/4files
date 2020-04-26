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
    snprintf(message, 512, "Token:\n  Base: %s\n  Sub: %s\n", base_name, sub_name);
    print_message(app, SCu8(message));
}

CUSTOM_COMMAND_SIG(tc_goto_jump_at_cursor)
CUSTOM_DOC("Language specific goto_jump_at_cursor")
{
    Heap *heap = &global_heap;
    
    View_ID view = get_active_view(app, Access_ReadVisible);
    Buffer_ID buffer = view_get_buffer(app, view, Access_ReadVisible);
    Marker_List *list = tc_get_or_make_list_for_buffer(app, heap, buffer);
    
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

CUSTOM_COMMAND_SIG(tc_goto_jump_at_cursor_same_panel)
CUSTOM_DOC("Language specific goto_jump_at_cursor_same_panel")
{
    Heap *heap = &global_heap;
    
    View_ID view = get_active_view(app, Access_ReadVisible);
    Buffer_ID buffer = view_get_buffer(app, view, Access_ReadVisible);
    
    // @note(tyler): This is the only change
    Marker_List *list = tc_get_or_make_list_for_buffer(app, heap, buffer);
    
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