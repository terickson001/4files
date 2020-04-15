function void
tc_generate_all_buffers_list__output_buffer(Application_Links *app, Lister *lister,
Buffer_ID buffer){
     Dirty_State dirty = buffer_get_dirty_state(app, buffer);
     String_Const_u8 status = {};
     switch (dirty){
         case DirtyState_UnsavedChanges:  status = string_u8_litexpr("*"); break;
         case DirtyState_UnloadedChanges: status = string_u8_litexpr("!"); break;
         case DirtyState_UnsavedChangesAndUnloadedChanges: status = string_u8_litexpr("*!"); break;
     }
     
     Scratch_Block scratch(app);
     String_Const_u8 buffer_name = {};
     if (current_project.loaded)
     {
         buffer_name = push_buffer_file_name(app, scratch, buffer);
         buffer_name = string_replace(scratch, buffer_name, current_project.dir, SCu8("./"), StringFill_NoTerminate);
     }
     if (buffer_name.size == 0)
     buffer_name = push_buffer_unique_name(app, scratch, buffer);
     lister_add_item(lister, buffer_name, status, IntAsPtr(buffer), 0);
}

function void
tc_generate_all_buffers_list(Application_Links *app, Lister *lister){
     lister_begin_new_item_set(app, lister);
     
     Buffer_ID viewed_buffers[16];
     i32 viewed_buffer_count = 0;
     
     // List currently viewed buffers
     for (View_ID view = get_view_next(app, 0, Access_Always);
          view != 0;
     view = get_view_next(app, view, Access_Always)){
         Buffer_ID new_buffer_id = view_get_buffer(app, view, Access_Always);
         for (i32 i = 0; i < viewed_buffer_count; i += 1){
             if (new_buffer_id == viewed_buffers[i]){
                 goto skip0;
             }
         }
         viewed_buffers[viewed_buffer_count++] = new_buffer_id;
         skip0:;
     }
     
     // Regular Buffers
     for (Buffer_ID buffer = get_buffer_next(app, 0, Access_Always);
          buffer != 0;
     buffer = get_buffer_next(app, buffer, Access_Always)){
         for (i32 i = 0; i < viewed_buffer_count; i += 1){
             if (buffer == viewed_buffers[i]){
                 goto skip1;
             }
         }
         if (!buffer_has_name_with_star(app, buffer)){
             tc_generate_all_buffers_list__output_buffer(app, lister, buffer);
         }
         skip1:;
     }
     
     // Buffers Starting with *
     for (Buffer_ID buffer = get_buffer_next(app, 0, Access_Always);
          buffer != 0;
     buffer = get_buffer_next(app, buffer, Access_Always)){
         for (i32 i = 0; i < viewed_buffer_count; i += 1){
             if (buffer == viewed_buffers[i]){
                 goto skip2;
             }
         }
         if (buffer_has_name_with_star(app, buffer)){
             tc_generate_all_buffers_list__output_buffer(app, lister, buffer);
         }
         skip2:;
     }
     
     // Buffers That Are Open in Views
     for (i32 i = 0; i < viewed_buffer_count; i += 1){
         tc_generate_all_buffers_list__output_buffer(app, lister, viewed_buffers[i]);
     }
}

function Buffer_ID
tc_get_buffer_from_user(Application_Links *app, String_Const_u8 query){
     Lister_Handlers handlers = lister_get_default_handlers();
     handlers.refresh = tc_generate_all_buffers_list;
     Lister_Result l_result = run_lister_with_refresh_handler(app, query, handlers);
     Buffer_ID result = 0;
     if (!l_result.canceled){
         result = (Buffer_ID)(PtrAsInt(l_result.user_data));
     }
     return(result);
}

function Buffer_ID
tc_get_buffer_from_user(Application_Links *app, char *query){
     return(tc_get_buffer_from_user(app, SCu8(query)));
}

CUSTOM_UI_COMMAND_SIG(tc_interactive_switch_buffer)
CUSTOM_DOC("Interactively switch to an open buffer.")
{
     Buffer_ID buffer = tc_get_buffer_from_user(app, "Switch:");
     if (buffer != 0){
         View_ID view = get_this_ctx_view(app, Access_Always);
         view_set_buffer(app, view, buffer, 0);
     }
}