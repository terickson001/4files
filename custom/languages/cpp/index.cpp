
function Code_Index_Nest*
cpp_parse_preprocessor(Code_Index_File *index, Generic_Parse_State *state){
    Token *token = token_it_read(&state->it);
    Code_Index_Nest *result = push_array_zero(state->arena, Code_Index_Nest, 1);
    result->kind = CodeIndexNest_Preprocessor;
    result->open = Ii64(token->pos);
    result->close = Ii64(max_i64);
    result->file = index;

    state->in_preprocessor = true;

    b32 potential_macro  = false;
    if (state->do_cpp_parse){
        if (token->sub_kind == TokenCppKind_PPDefine){
            potential_macro = true;
        }
    }

    generic_parse_inc(state);
    for (;;){
        generic_parse_skip_soft_tokens(index, state);
        token = token_it_read(&state->it);
        if (token == 0 || state->finished){
            break;
        }

        if (!HasFlag(token->flags, TokenBaseFlag_PreprocessorBody) ||
            token->kind == TokenBaseKind_Preprocessor){
            result->is_closed = true;
            result->close = Ii64(token->pos);
            break;
        }

        if (state->do_cpp_parse && potential_macro){
            if (token->sub_kind == TokenCppKind_Identifier){
                index_new_note(index, state, Ii64(token), CodeIndexNote_Macro, result);
            }
            potential_macro = false;
        }

        if (token->kind == TokenBaseKind_ScopeOpen){
            Code_Index_Nest *nest = generic_parse_scope(index, state);
            nest->parent = result;
            code_index_push_nest(&result->nest_list, nest);
            continue;
        }

        if (token->kind == TokenBaseKind_ParentheticalOpen){
            Code_Index_Nest *nest = generic_parse_paren(index, state);
            nest->parent = result;
            code_index_push_nest(&result->nest_list, nest);
            continue;
        }

        if (token->sub_kind == TokenCppKind_Struct ||
            token->sub_kind == TokenCppKind_Union ||
            token->sub_kind == TokenCppKind_Enum)            cpp_parse_type_structure(index, state, 0);
        else if (token->sub_kind == TokenCppKind_Typedef)    cpp_parse_type_def(index, state, 0);
        else if (token->sub_kind == TokenCppKind_Identifier) cpp_parse_function(index, state, 0);
        generic_parse_inc(state);
    }

    result->nest_array = code_index_nest_ptr_array_from_list(state->arena, &result->nest_list);

    state->in_preprocessor = false;

    return(result);
}

function b32 cpp_parse_preproc(Code_Index_File *index, Generic_Parse_State *state)
{
    Token *token = token_it_read(&state->it);
    if (token->kind == TokenBaseKind_Preprocessor) {
        Code_Index_Nest *nest = cpp_parse_preprocessor(index, state);
        code_index_push_nest(&index->nest_list, nest);
        return true;
    }
    return false;
}

function b32 cpp_try_index(Code_Index_File *index, Generic_Parse_State *state)
{
    Token *token = token_it_read(&state->it);
    if (cpp_parse_preproc(index, state));
    else if (token->sub_kind == TokenCppKind_Struct ||
             token->sub_kind == TokenCppKind_Union ||
             token->sub_kind == TokenCppKind_Enum)       cpp_parse_type_structure(index, state, 0);
    else if (token->sub_kind == TokenCppKind_Typedef)    cpp_parse_type_def(index, state, 0);
    else if (token->sub_kind == TokenCppKind_Identifier) cpp_parse_function(index, state, 0);
    else return false;

    return true;
}

function Code_Index_Nest *cpp_parse_statement(Code_Index_File *index, Generic_Parse_State *state)
{
    return generic_parse_statement(index, state);
}
