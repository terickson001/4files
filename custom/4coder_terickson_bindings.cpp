/*
4coder_default_bidings.cpp - Supplies the default bindings used for default 4coder behavior.
*/

// TOP

#if !defined(FCODER_DEFAULT_BINDINGS_CPP)
#define FCODER_DEFAULT_BINDINGS_CPP

#include "4coder_default_include.cpp"
typedef String_Const_u8 Data;

// NOTE(allen): Users can declare their own managed IDs here.
#include "4coder_language_ids.cpp"
#include "4coder_terickson_ids.cpp"

#include "generated/managed_id_metadata.cpp"

global Arena tc_global_arena = {};

#include "4coder_terickson_helper.cpp"

#include "4coder_terickson_language.cpp"

// Extensions
#include "ext/4coder_terickson_function_index.cpp"
#include "ext/4coder_terickson_std_include.cpp"
#include "ext/4coder_terickson_todo.cpp"

// Languages
#include "languages/cpp/cpp.cpp"
#include "languages/odin/odin.cpp"
#include "languages/glsl/glsl.cpp"
#include "languages/gas/gas.cpp"
#include "languages/nasm/nasm.cpp"

// Miscellaneous
#include "4coder_terickson_error_message.cpp"
#include "4coder_terickson_scopeline.cpp"
#include "4coder_terickson_highlight.cpp"
#include "4coder_terickson_hooks.cpp"
#include "4coder_terickson_lists.cpp"
#include "4coder_terickson_commands.cpp"


void
custom_layer_init(Application_Links *app)
{
    Thread_Context *tctx = get_thread_context(app);
    tc_global_arena = make_arena_system();
    
    // NOTE(allen): setup for default framework
    default_framework_init(app);
    code_index_init();
    
    // Multi-Language Support
    init_ext_language();
    
    // Language Dependent Extensions
    init_ext_std_include();
    init_ext_todo();
    
    // Language Definitions
	init_language_cpp();
	init_language_odin();
	init_language_glsl();
	init_language_gas();
	init_language_nasm();
    
    // Some Processing on Language Definitions
    finalize_languages(app);
    
    // NOTE(allen): default hooks and command maps
    set_all_default_hooks(app);
    set_language_hooks(app);
    set_custom_hook(app, HookID_WholeScreenRenderCaller, tc_whole_screen_render_caller);
    set_custom_hook(app, HookID_RenderCaller, tc_render_caller);
    
    mapping_init(tctx, &framework_mapping);
    String_ID global_map_id = vars_save_string_lit("keys_global");
    String_ID file_map_id = vars_save_string_lit("keys_file");
    String_ID code_map_id = vars_save_string_lit("keys_code");
#if OS_MAC
    setup_mac_mapping(&framework_mapping, global_map_id, file_map_id, code_map_id);
#else
    setup_default_mapping(&framework_mapping, global_map_id, file_map_id, code_map_id);
#endif
	setup_essential_mapping(&framework_mapping, global_map_id, file_map_id, code_map_id);
    {
        MappingScope();
        SelectMapping(&framework_mapping);
        
        SelectMap(global_map_id);
        BindCore(tc_startup, CoreCode_Startup);
    }
    /*
        mapping_init(tctx, &framework_mapping);
        setup_default_mapping(&framework_mapping, mapid_global, mapid_file, mapid_code);
        tc_setup_mapping(&framework_mapping, mapid_global, mapid_file, mapid_code);
    */
}

#endif //FCODER_DEFAULT_BINDINGS

// BOTTOM

