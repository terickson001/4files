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

internal Sticky_Jump_Array tc_parse_buffer_to_jump_array(Application_Links *app, Arena *arena, Buffer_ID buffer){
    Sticky_Jump_Node *jump_first = 0;;
    Sticky_Jump_Node *jump_last = 0;
    i32 jump_count = 0;
    
    Managed_Scope scope = buffer_get_managed_scope(app, buffer);
    Base_Allocator *managed_allocator = managed_scope_allocator(app, scope);
    Arena managed_arena = make_arena(managed_allocator);
    List_String_Const_u8 *msg_list = scope_attachment(app, scope, buffer_errors, List_String_Const_u8);
    
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
                                if (parsed_jump.msg.size > 0)
                                {
                                    string_list_push(&managed_arena, msg_list, push_string_copy(&managed_arena, parsed_jump.msg));
                                }
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

internal void tc_init_marker_list(Application_Links *app, Heap *heap, Buffer_ID buffer, Marker_List *list){
    Scratch_Block scratch(app);
    
    Sticky_Jump_Array jumps = tc_parse_buffer_to_jump_array(app, scratch, buffer);
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

internal Marker_List* tc_get_or_make_list_for_buffer(Application_Links *app, Heap *heap, Buffer_ID buffer_id){
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
        tc_init_marker_list(app, heap, buffer_id, result);
        if (result->jump_count == 0){
            delete_marker_list(result);
            result = 0;
        }
    }
    return(result);
}


CUSTOM_COMMAND_SIG(tc_if_read_only_goto_position)
CUSTOM_DOC("If the buffer in the active view is writable, inserts a character, otherwise performs goto_jump_at_cursor.")
{
    View_ID view = get_active_view(app, Access_ReadVisible);
    Buffer_ID buffer = view_get_buffer(app, view, Access_ReadWriteVisible);
    if (buffer == 0){
        buffer = view_get_buffer(app, view, Access_ReadVisible);
        if (buffer != 0){
            tc_goto_jump_at_cursor(app);
            lock_jump_buffer(app, buffer);
        }
    }
    else{
        leave_current_input_unhandled(app);
    }
}

CUSTOM_COMMAND_SIG(tc_if_read_only_goto_position_same_panel)
CUSTOM_DOC("If the buffer in the active view is writable, inserts a character, otherwise performs goto_jump_at_cursor_same_panel.")
{
    View_ID view = get_active_view(app, Access_ReadVisible);
    Buffer_ID buffer = view_get_buffer(app, view, Access_ReadWriteVisible);
    if (buffer == 0){
        buffer = view_get_buffer(app, view, Access_ReadVisible);
        if (buffer != 0){
            tc_goto_jump_at_cursor_same_panel(app);
            lock_jump_buffer(app, buffer);
        }
    }
    else{
        leave_current_input_unhandled(app);
    }
}