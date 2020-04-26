function void tc_exec_project_command(Application_Links *app, Project_Command *command){
    if (command->cmd.size > 0){
        b32 footer_panel = command->footer_panel;
        b32 save_dirty_files = command->save_dirty_files;
        b32 cursor_at_end = command->cursor_at_end;
        
        if (save_dirty_files){
            save_all_dirty_buffers(app);
        }
        
        View_ID view = 0;
        Buffer_Identifier buffer_id = {};
        u32 flags = CLI_OverlapWithConflict|CLI_SendEndSignal;
        if (cursor_at_end){
            flags |= CLI_CursorAtEnd;
        }
        
        b32 set_fancy_font = false;
        if (command->out.size > 0){
            buffer_id = buffer_identifier(command->out);
            
            if (footer_panel){
                view = get_or_open_build_panel(app);
                if (string_match(command->out, string_u8_litexpr("*compilation*")))
                    set_fancy_font = true;
            }
            else{
                Buffer_ID buffer = buffer_identifier_to_id(app, buffer_id);
                view = get_first_view_with_buffer(app, buffer);
                if (view == 0){
                    view = get_active_view(app, Access_Always);
                }
            }
            
            block_zero_struct(&prev_location);
            lock_jump_buffer(app, command->out);
        }
        else{
            // TODO(allen): fix the exec_system_command call so it can take a null buffer_id.
            buffer_id = buffer_identifier(string_u8_litexpr("*dump*"));
        }
        
        String_Const_u8 dir = current_project.dir;
        String_Const_u8 cmd = command->cmd;
        exec_system_command(app, view, buffer_id, dir, cmd, flags);
        if (set_fancy_font){
            set_fancy_compilation_buffer_font(app);
        }
    }
}

function void
tc_exec_project_command_by_index(Application_Links *app, i32 command_index){
    if (!current_project.loaded){
        return;
    }
    if (command_index < 0 || command_index >= current_project.command_array.count){
        return;
    }
    Project_Command *command = &current_project.command_array.commands[command_index];
    tc_exec_project_command(app, command);
}

function void
tc_exec_project_fkey_command(Application_Links *app, i32 fkey_index){
    if (!current_project.loaded){
        return;
    }
    i32 command_index = current_project.fkey_commands[fkey_index];
    if (command_index < 0 || command_index >= current_project.command_array.count){
        return;
    }
    Project_Command *command = &current_project.command_array.commands[command_index];
    tc_exec_project_command(app, command);
}

function void
tc_exec_project_command_by_name(Application_Links *app, String_Const_u8 name){
    if (!current_project.loaded){
        return;
    }
    Project_Command *command = current_project.command_array.commands;
    for (i32 i = 0; i < current_project.command_array.count; ++i, ++command){
        if (string_match(command->name, name)){
            tc_exec_project_command(app, command);
            break;
        }
    }
}

function void
tc_exec_project_command_by_name(Application_Links *app, char *name){
    tc_exec_project_command_by_name(app, SCu8(name));
}

CUSTOM_COMMAND_SIG(tc_project_fkey_command)
CUSTOM_DOC("Run an 'fkey command' configured in a project.4coder file.  Determines the index of the 'fkey command' by which function key or numeric key was pressed to trigger the command.")
{
    ProfileScope(app, "[TErickson] project fkey command");
    User_Input input = get_current_input(app);
    b32 got_ind = false;
    i32 ind = 0;
    if (input.event.kind == InputEventKind_KeyStroke){
        if (KeyCode_F1 <= input.event.key.code && input.event.key.code <= KeyCode_F16){
            ind = (input.event.key.code - KeyCode_F1);
            got_ind = true;
        }
        else if (KeyCode_1 <= input.event.key.code && input.event.key.code <= KeyCode_9){
            ind = (input.event.key.code - '1');
            got_ind = true;
        }
        else if (input.event.key.code == KeyCode_0){
            ind = 9;
            got_ind = true;
        }
        if (got_ind){
            tc_exec_project_fkey_command(app, ind);
        }
    }
}

CUSTOM_COMMAND_SIG(tc_project_command_lister)
CUSTOM_DOC("Open a lister of all commands in the currently loaded project.")
{
    if (current_project.loaded){
        Project_Command_Lister_Result proj_cmd =
            get_project_command_from_user(app, &current_project, "Command:");
        if (proj_cmd.success){
            tc_exec_project_command_by_index(app, proj_cmd.index);
        }
    }
}
