#define EXT_STD_INCLUDE SCu8("Standard Include")

typedef String_Const_u8_Array Standard_Includes;

global Table_u64_u64 loaded_std_includes = {0};

bool load_std_includes(Application_Links *app, Buffer_ID buffer);
void init_ext_std_include()
{
    language_push_hook(Hook_PostBeginBuffer, (Void_Func *)load_std_includes);
}

bool load_std_includes(Application_Links *app, Buffer_ID buffer)
{
    Language *language = *buffer_get_language(app, buffer);
    if (!language) return false;
    Extension_Support *ext = language_get_extension(language, EXT_STD_INCLUDE);
    if (!ext) return false;
    
    Standard_Includes *includes = (Standard_Includes *)ext->ext_interface.str;
    
    if (!loaded_std_includes.allocator) loaded_std_includes = make_table_u64_u64(tc_global_arena.base_allocator, 8);
    
    {
        Table_Lookup lookup = table_lookup(&loaded_std_includes, HandleAsU64(language));
        if (lookup.found_match) return true; // Already loaded
        table_insert(&loaded_std_includes, HandleAsU64(language), 1);
    }
    
    Scratch_Block scratch(app);
    Prj_Pattern_List whitelist = prj_pattern_list_from_extension_array(scratch, language->file_extensions);
    Prj_Pattern_List blacklist = prj_get_standard_blacklist(scratch);
    for (int i = 0; i < includes->count; i++)
        prj_open_files_pattern_filter(app, includes->strings[i], whitelist, blacklist, PrjOpenFileFlag_Recursive);
    // open_files_with_extension(app, includes->strings[i], language->file_extensions, OpenAllFilesFlag_Recursive);
    
    return true;
}