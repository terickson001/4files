
global char const *library_file = "prjlib.4coder";

function b32 load_prjlib(Application_Links *app)
{
    Scratch_Block scratch(app);
    String_Const_u8 user_directory = system_get_path(scratch, SystemPath_UserDirectory);
    String_Const_u8 path = push_stringf(scratch, "%.*s/%s", string_expand(user_directory), library_file);
    File_Name_Data dump = dump_file(scratch, path);
    if (dump.data.str != 0)
    {
        print_message(app, SCu8("Opened prjlib.4coder\n"));
        Config *config = 0;
        Variable_Handle var = vars_get_nil();
        config = def_config_from_text(app, scratch, path, dump.data);
        if (config != 0)
        {
            print_message(app, SCu8("Parsed prjlib.4coder\n"));
            var = def_fill_var_from_config(app, vars_get_root(), vars_save_string_lit("prj_library"), config);
            String8 error_text = config_stringize_errors(app, scratch, config);
            if (error_text.size > 0){
                print_message(app, string_u8_litexpr("Project errors:\n"));
                print_message(app, error_text);
                print_message(app, string_u8_litexpr("\n"));
            }
            vars_print(app, var);
            return true;
        }
    }
    
    vars_new_variable(vars_get_root(), vars_save_string_lit("prj_library"), vars_save_string(path));
    return false;
}

function void write_prjlib_to_file(Application_Links *app, Variable_Handle prj_library, String_Const_u8 filepath)
{
    Scratch_Block scratch(app);
    
    FILE *out = fopen((char *)filepath.str, "wb");
    if (out == 0)
    {
        print_message(app, filepath);
        print_message(app, SCu8("ERROR: Couldn't open prjlib.4coder for writing.\n"));
        return;
    }
    
    Variable_Handle recent_prj = vars_read_key(prj_library, vars_save_string_lit("recent"));
    if (!vars_is_nil(recent_prj))
    {
        String_Const_u8 val = vars_string_from_var(scratch, recent_prj);
        fprintf(out, "recent = \"%.*s\";\n", string_expand(val));
    }
    
    Variable_Handle prj_list = vars_read_key(prj_library, vars_save_string_lit("projects"));
    if (!vars_is_nil(prj_list))
    {
        fprintf(out, "projects = {\n");
        for (Variable_Handle prj = vars_first_child(prj_list);
             !vars_is_nil(prj);
             prj = vars_next_sibling(prj))
        {
            String_Const_u8 prj_name = vars_key_from_var(scratch, prj);
            String_Const_u8 prj_path = vars_string_from_var(scratch, prj);
            fprintf(out, ".%.*s = \"%.*s\",\n", 
                    string_expand(prj_name), 
                    string_expand(prj_path));
        }
        fprintf(out, "};\n");
    }
    fclose(out);
}

function void register_project(Application_Links *app, Variable_Handle prj_var)
{
    if (vars_is_nil(prj_var)) return;
    
    Scratch_Block scratch(app);
    Variable_Handle prj_library = vars_read_key(vars_get_root(), vars_save_string_lit("prj_library"));
    Variable_Handle prj_list = vars_read_key(prj_library, vars_save_string_lit("projects"));
    if (vars_is_nil(prj_list))
    {
        prj_list = vars_new_variable(prj_library, vars_save_string_lit("projects"));
    }
    
    String_Const_u8 prj_name = vars_string_from_var(scratch, vars_read_key(prj_var, vars_save_string_lit("project_name")));
    String_Const_u8 prj_name_ident = push_stringf(scratch, "_%.*s", string_expand(string_replace(scratch, prj_name, SCu8(" "), SCu8("_"))));
    String_ID prj_path = vars_string_id_from_var(prj_var);
    vars_new_variable(prj_list, vars_save_string(prj_name_ident), prj_path);
    
    write_prjlib_to_file(app, prj_library, vars_string_from_var(scratch, prj_library));
}

function Variable_Handle prj_from_user(Application_Links *app, String8 query)
{
    Scratch_Block scratch(app);
    Lister_Block lister(app, scratch);
    lister_set_query(lister, query);
    lister_set_default_handlers(lister);
    
    Variable_Handle prj_lib = vars_read_key(vars_get_root(), vars_save_string_lit("prj_library"));
    Variable_Handle prj_list = vars_read_key(prj_lib, vars_save_string_lit("projects"));
    for (Variable_Handle prj = vars_first_child(prj_list);
         !vars_is_nil(prj);
         prj = vars_next_sibling(prj))
    {
        String8 name = string_skip(vars_key_from_var(scratch, prj), 1); // skip leading underscore
        String8 path = vars_string_from_var(scratch, prj);
        lister_add_item(lister, name, path, prj.ptr, 0);
    }
    
    Variable_Handle result = vars_get_nil();
    Lister_Result l_result = run_lister(app, lister);
    if (!l_result.canceled){
        if (l_result.user_data != 0){
            result.ptr = (Variable*)l_result.user_data;
        }
    }
    
    return(result);
}

function void close_prj_buffers(Application_Links *app)
{
    Scratch_Block scratch(app);
    
    i32 buffers_to_close_max = Thousand(100);
    Buffer_ID *buffers_to_close = push_array(scratch, Buffer_ID, buffers_to_close_max);
    
    Variable_Handle prj_var = vars_read_key(vars_get_root(), vars_save_string_lit("prj_config"));
    
    Variable_Handle whitelist_var = vars_read_key(prj_var, vars_save_string_lit("patterns"));
    Variable_Handle blacklist_var = vars_read_key(prj_var, vars_save_string_lit("blacklist_patterns"));
    Prj_Pattern_List whitelist = prj_pattern_list_from_var(scratch, whitelist_var);
    Prj_Pattern_List blacklist = prj_pattern_list_from_var(scratch, blacklist_var);
    
    b32 do_repeat = false;
    b32 unsaved_changes = false;
    do
    {
        i32 buffers_to_close_count = 0;
        
        for (Buffer_ID buffer = get_buffer_next(app, 0, Access_Always);
             buffer != 0;
             buffer = get_buffer_next(app, buffer, Access_Always))
        {
            String8 file_name = push_buffer_file_name(app, scratch, buffer);
            if (!prj_match_in_pattern_list(file_name, whitelist)) continue;
            if ( prj_match_in_pattern_list(file_name, blacklist)) continue;
            if (buffers_to_close_count >= buffers_to_close_max){
                do_repeat = true;
                break;
            }
            if (!unsaved_changes)
            {
                Dirty_State dirty = buffer_get_dirty_state(app, buffer);
                if (HasFlag(dirty, DirtyState_UnsavedChanges))
                {
                    unsaved_changes = true;
                    b32 close = do_4coder_close_user_check(app, get_active_view(app, Access_Always));
                    if (!close) return;
                }
            }
            buffers_to_close[buffers_to_close_count++] = buffer;
        }
        
        for (i32 i = 0; i < buffers_to_close_count; ++i){
            buffer_kill(app, buffers_to_close[i], BufferKill_AlwaysKill);
        }
    } while(do_repeat);
}

function void set_recent_project(Application_Links *app, Variable_Handle prj_var)
{
    Variable_Handle prj_library = vars_read_key(vars_get_root(), vars_save_string_lit("prj_library"));
    Assert(!vars_is_nil(prj_library));
    vars_new_variable(prj_library, vars_save_string_lit("recent"), vars_key_id_from_var(prj_var));
}

function void prjlib_load_project(Application_Links *app)
{
    Scratch_Block scratch(app);
    
    load_project(app);
    Variable_Handle prj_var = vars_read_key(vars_get_root(), vars_save_string_lit("prj_config"));
    register_project(app, prj_var);
    set_recent_project(app, prj_var);
    Variable_Handle prj_library = vars_read_key(vars_get_root(), vars_save_string_lit("prj_library"));
    write_prjlib_to_file(app, prj_library, vars_string_from_var(scratch, prj_library));
}

function void open_project(Application_Links *app, Variable_Handle prj)
{
    Scratch_Block scratch(app);
    String8 prj_path = vars_string_from_var(scratch, prj);
    String8 prj_root = string_remove_last_folder(prj_path);
    set_hot_directory(app, prj_root);
    load_project(app);
}

function void close_project(Application_Links *app)
{
    Variable_Handle prj_var = vars_read_key(vars_get_root(), vars_save_string_lit("prj_config"));
    if (vars_is_nil(prj_var)) return;
    close_prj_buffers(app);
    // vars_erase(vars_get_root(),  vars_save_string_lit("prj_config"));
}

CUSTOM_COMMAND_SIG(switch_project)
CUSTOM_DOC("Open a lister of all registered projects.")
{
    Variable_Handle prj = prj_from_user(app, SCu8("Switch Project:"));
    if (!vars_is_nil(prj))
    {
        close_project(app);
        open_project(app, prj);
    }
}