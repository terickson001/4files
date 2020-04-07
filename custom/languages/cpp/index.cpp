b32 cpp_try_index(Code_Index_File *index, Generic_Parse_State *state, Token *token)
{
    if (token->sub_kind == TokenCppKind_Struct ||
        token->sub_kind == TokenCppKind_Union ||
        token->sub_kind == TokenCppKind_Enum){
        cpp_parse_type_structure(index, state, 0);
        return true;
    }
    else if (token->sub_kind == TokenCppKind_Typedef) {
        cpp_parse_type_def(index, state, 0);
        return true;
    }
    else if (token->sub_kind == TokenCppKind_Identifier) {
        cpp_parse_function(index, state, 0);
        return true;
    }
    return false;
}