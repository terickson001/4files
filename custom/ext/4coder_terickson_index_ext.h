/* date = March 10th 2021 11:36 pm */

#ifndef TERICKSON_INDEX_EXT_H
#define TERICKSON_INDEX_EXT_H

#define EXT_INDEX_EXT SCu8("Code Index Extended")

struct Index_File;
struct Index_Scope;

typedef u32 Index_Symbol_Kind;
enum
{
    IndexSymbol_Invalid,
    IndexSymbol_Type,
    IndexSymbol_Function,
    IndexSymbol_Variable,
    IndexSymbol_Constant,
    IndexSymbol_Custom, // Has no relevance to core functionality
}

typedef u32 Index_Symbol_Flags;
enum
{
}

struct Index_Symbol
{
    Index_Symbol_Kind kind;
    u32 sub_kind;
    Index_Symbol_Flags flags;
    u32 sub_flags;
    
    Index_Scope *scope;
    Index_Symbol *next;
    Index_Symbol *prev;
    
    Index_Symbol *head_collision;
    Index_Symbol *next_collision;
    
    Data custom_data;
    
    Index_File *file;
}

struct Index_Scope
{
    Index_Scope *next;
    Index_Scope *prev;
    Index_Scope *children;
    Index_Scope *parent;
    
    Index_Symbol *symbols;
}

struct Index_File
{
    Index_Scope *scope;
    Table_Data_u64 symbols;
    String_Const_u8 path;
    Buffer_ID buffer;
}

#define MAX_INDEX_FILES 8192
struct Code_Index_Ext
{
    System_Mutex mutex;
    Arena arena;
    Table_Data_u64 path_to_index;
    Index_File *file_storage[MAX_INDEX_FILES];
    Index_File *next_file;
}

struct Index_State
{
    Application_Links *app;
    Arena *arena;
    String_Const_u8 contents;
    Token_Iterator_Array it;
    
    b32 finished;
}

global u32 INDEX_NEXT_SUB_KIND = 1;
void register_symbol_sub_kind(u32 *sub_kind);

#endif //TERICKSON_INDEX_EXT_H
