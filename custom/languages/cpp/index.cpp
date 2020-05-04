function b32 cpp_parse_preproc(Code_Index_File *index, Generic_Parse_State *state)
{
    Token *token = token_it_read(&state->it);
    if (token->kind == TokenBaseKind_Preprocessor) {
        Code_Index_Nest *nest = generic_parse_preprocessor(index, state);
        code_index_push_nest(&index->nest_list, nest);
        return true;
    }
    return false;
}

function b32 cpp_try_index(Code_Index_File *index, Generic_Parse_State *state)
{
    Token *token = token_it_read(&state->it);
    if (cpp_parse_preproc(index, state)) {
        return true;
    } else if (token->sub_kind == TokenCppKind_Struct ||
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