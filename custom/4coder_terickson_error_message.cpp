function void tc_render_error_messages(Application_Links *app, Buffer_ID buffer, Buffer_ID compilation_buffer, View_ID view, Text_Layout_ID text_layout_id)
{
    Heap *heap = &global_heap;
    ProfileScope(app, "[TErickson] Error Messages");
    
    if (!buffer || !compilation_buffer)
        return;
    
    Scratch_Block scratch(app);
    
    Managed_Scope scope = buffer_get_managed_scope(app, compilation_buffer);
    List_String_Const_u8 *msg_list = scope_attachment(app, scope, buffer_errors, List_String_Const_u8);
    Marker_List *markers = tc_get_or_make_list_for_buffer(app, heap, compilation_buffer);
    if (!markers) return;
    Sticky_Jump_Stored *jump_array = push_array(scratch, Sticky_Jump_Stored, markers->jump_count);
    managed_object_load_data(app, markers->jump_array, 0, markers->jump_count, jump_array);
    
    Face_ID face_id = get_face_id(app, buffer);
    Face_Metrics metrics = get_face_metrics(app, face_id);
    
    Node_String_Const_u8 *node = msg_list->first;
    int last_line = -1;
    for (i32 i = 0; i < markers->jump_count; i += 1, node = node->next){
        if (jump_array[i].jump_buffer_id != buffer) continue;
        ID_Pos_Jump_Location location = {};
        if (!get_jump_from_list(app, markers, i, &location))
            continue;
        
        i64 line_number = get_line_number_from_pos(app, buffer, location.pos);
        if (last_line == line_number) continue;
        
        i64 line_end = get_line_end_pos(app, buffer, line_number);
        Rect_f32 eol = text_layout_character_on_screen(app, text_layout_id, line_end);
        u32 color = finalize_color(defcolor_pop2, 0);
        Vec2_f32 draw_pos =
        {
            eol.x0 + metrics.space_advance,
            eol.y0,
        };
        
        draw_string(app, face_id, node->string, draw_pos, color);
        last_line = line_number;
    }
}