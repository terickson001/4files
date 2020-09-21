/*
4coder_default_bidings.cpp - Supplies the default bindings used for default 4coder behavior.
*/

// TOP

#if !defined(FCODER_DEFAULT_BINDINGS_CPP)
#define FCODER_DEFAULT_BINDINGS_CPP

#include "4coder_default_include.cpp"

// NOTE(allen): Users can declare their own managed IDs here.
#include "4coder_terickson_ids.cpp"

#include "generated/managed_id_metadata.cpp"

global Arena tc_global_arena = {};

#include "4coder_terickson_helper.cpp"

#include "4coder_terickson_language.cpp"
#include "4coder_terickson_code_index.cpp"

// Extensions
#include "4coder_terickson_function_index.cpp"
#include "4coder_terickson_std_include.cpp"

// Languages
#include "languages/cpp/cpp.cpp"
#include "languages/odin/odin.cpp"
#include "languages/glsl/glsl.cpp"
#include "languages/gas/gas.cpp"
#include "languages/nasm/nasm.cpp"

// Miscellaneous
#include "4coder_terickson_todo.cpp"
#include "4coder_terickson_jump.cpp"
#include "4coder_terickson_error_message.cpp"
#include "4coder_terickson_scopeline.cpp"
#include "4coder_terickson_highlight.cpp"
#include "4coder_terickson_hooks.cpp"
#include "4coder_terickson_project.cpp"
#include "4coder_terickson_lists.cpp"
#include "4coder_terickson_commands.cpp"
#include "4coder_terickson_map.cpp"


void
custom_layer_init(Application_Links *app)
{
    Thread_Context *tctx = get_thread_context(app);
    
    // NOTE(allen): setup for default framework
    default_framework_init(app);
    code_index_init();
    
    tc_global_arena = make_arena_system();
	init_language_cpp();
	init_language_odin();
	init_language_glsl();
	init_language_gas();
	init_language_nasm();
    init_languages(app, &tc_global_arena);
    
    // NOTE(allen): default hooks and command maps
    set_all_default_hooks(app);
    set_custom_hook(app, HookID_WholeScreenRenderCaller, tc_whole_screen_render_caller);
    set_custom_hook(app, HookID_RenderCaller, tc_render_caller);
    set_custom_hook(app, HookID_BeginBuffer, tc_begin_buffer);
    set_custom_hook(app, HookID_BufferEditRange, tc_buffer_edit_range);
    set_custom_hook(app, HookID_Tick, tc_tick);
    
    mapping_init(tctx, &framework_mapping);
    // setup_default_mapping(&framework_mapping, mapid_global, mapid_file, mapid_code);
    tc_setup_mapping(&framework_mapping, mapid_global, mapid_file, mapid_code);
    
    // amberify(app);
}

#endif //FCODER_DEFAULT_BINDINGS

// BOTTOM

