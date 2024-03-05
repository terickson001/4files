function b32 glsl_try_index(Code_Index_File *index, Generic_Parse_State *state)
{
     return cpp_try_index(index, state);
}

function Code_Index_Nest *glsl_parse_statement(Code_Index_File *index, Generic_Parse_State *state)
{
    return generic_parse_statement(index, state);
}
