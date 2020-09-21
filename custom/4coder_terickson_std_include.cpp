#define EXT_STD_INCLUDE SCu8("Standard Include")

typedef String_Const_u8_Array Standard_Includes;

global Table_u64_u64 loaded_std_includes = {0};

bool load_std_includes(Application_Links *app, Buffer_ID buffer)
{
    Language *language = *buffer_get_language(app, buffer);
    if (!language) return false;
    Extension_Support *ext = language_get_extension(language, EXT_STD_INCLUDE);
    if (!ext) return false;
    
    Standard_Includes *includes = (Standard_Includes *)ext->ext_interface.data;
    
    if (!loaded_std_includes.allocator) loaded_std_includes = make_table_u64_u64(tc_global_arena.base_allocator, 8);
    
    {
        Table_Lookup lookup = table_lookup(&loaded_std_includes, HandleAsU64(language));
        if (lookup.found_match) return true; // Already loaded
        table_insert(&loaded_std_includes, HandleAsU64(language), 1);
    }
    
    for (int i = 0; i < includes->count; i++)
        open_files_with_extension(app, includes->strings[i], language->file_extensions, OpenAllFilesFlag_Recursive);
    
    return true;
}