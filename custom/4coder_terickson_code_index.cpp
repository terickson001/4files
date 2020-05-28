

function Code_Index_Nest*
tc_generic_parse_scope(Code_Index_File *index, Generic_Parse_State *state, b32 allow_decl){
    Managed_Scope scope = buffer_get_managed_scope(state->app, index->buffer);
    Language **language = scope_attachment(state->app, scope, buffer_language, Language*);
    
    Token *token = token_it_read(&state->it);
    Code_Index_Nest *result = push_array_zero(state->arena, Code_Index_Nest, 1);
    result->kind = CodeIndexNest_Scope;
    result->open = Ii64(token);
    result->close = Ii64(max_i64);
    result->file = index;
    
    state->scope_counter += 1;
    
    generic_parse_inc(state);
    for (;;){
        generic_parse_skip_soft_tokens(index, state);
        token = token_it_read(&state->it);
        if (token == 0 || state->finished){
            break;
        }
        
        if (state->in_preprocessor){
            if (!HasFlag(token->flags, TokenBaseFlag_PreprocessorBody) ||
                token->kind == TokenBaseKind_Preprocessor){
                break;
            }
        }
        else{
            if (token->kind == TokenBaseKind_Preprocessor){
                Code_Index_Nest *nest = generic_parse_preprocessor(index, state);
                code_index_push_nest(&index->nest_list, nest);
                continue;
            }
        }
        
        if (token->kind == TokenBaseKind_ScopeClose){
            result->is_closed = true;
            result->close = Ii64(token);
            generic_parse_inc(state);
            break;
        }
        
        
        if (allow_decl && *language && (*language)->try_index(index, state))
            continue;
        
        
        if (token->kind == TokenBaseKind_ScopeOpen){
            Code_Index_Nest *nest = generic_parse_scope(index, state);
            nest->parent = result;
            code_index_push_nest(&result->nest_list, nest);
            continue;
        }
        
        if (token->kind == TokenBaseKind_ParentheticalClose){
            generic_parse_inc(state);
            continue;
        }
        
        if (token->kind == TokenBaseKind_ParentheticalOpen){
            Code_Index_Nest *nest = generic_parse_paren(index, state);
            nest->parent = result;
            code_index_push_nest(&result->nest_list, nest);
            
            // NOTE(allen): after a parenthetical group we consider ourselves immediately
            // transitioning into a statement
            nest = generic_parse_statement(index, state);
            nest->parent = result;
            code_index_push_nest(&result->nest_list, nest);
            
            continue;
        }
        
        {
            Code_Index_Nest *nest = generic_parse_statement(index, state);
            nest->parent = result;
            code_index_push_nest(&result->nest_list, nest);
        }
    }
    
    result->nest_array = code_index_nest_ptr_array_from_list(state->arena, &result->nest_list);
    
    state->scope_counter -= 1;
    
    return(result);
}

function b32 cpp_parse_extern(Code_Index_File *index, Generic_Parse_State *state)
{
    Token *token = token_it_read(&state->it);
    if (token->sub_kind == TokenCppKind_Extern)
    {
        generic_parse_inc(state);
        generic_parse_skip_soft_tokens(index, state);
        Token *peek = token_it_read(&state->it);
        switch (peek->kind)
        {
            case TokenBaseKind_LiteralString:
            {
                generic_parse_inc(state);
                generic_parse_skip_soft_tokens(index, state);
                if (token_it_read(&state->it)->kind != TokenBaseKind_ScopeOpen)
                    break;
            }
            
            case TokenBaseKind_ScopeOpen:
            {
                Code_Index_Nest *nest = tc_generic_parse_scope(index, state, true);
                code_index_push_nest(&index->nest_list, nest);
                return true;
            } break;
        }
    }
    return false;
}

function b32 tc_generic_parse_scope_paren(Code_Index_File *index, Generic_Parse_State *state)
{
    Token *token = token_it_read(&state->it);
    if (token->kind == TokenBaseKind_ScopeOpen) {
        Code_Index_Nest *nest = tc_generic_parse_scope(index, state, false);
        code_index_push_nest(&index->nest_list, nest);
        return true;
    }
    else if (token->kind == TokenBaseKind_ParentheticalOpen) {
        Code_Index_Nest *nest = generic_parse_paren(index, state);
        code_index_push_nest(&index->nest_list, nest);
        return true;
    }
    return false;
}

function b32 tc_generic_parse_full_input_breaks(Code_Index_File *index, Generic_Parse_State *state, i32 limit)
{
    b32 result = false;
    
    Managed_Scope scope = buffer_get_managed_scope(state->app, index->buffer);
    Language **language = scope_attachment(state->app, scope, buffer_language, Language*);
    if (!*language) return true;
    
    i64 first_index = token_it_index(&state->it);
    i64 one_past_last_index = first_index + limit;
    for (;;){
        generic_parse_skip_soft_tokens(index, state);
        Token *token = token_it_read(&state->it);
        
        if (token == 0 || state->finished){
            result = true;
            break;
        }
        
        if      ((*language)->try_index(index, state));
        else if (tc_generic_parse_scope_paren(index, state));
        else if (cpp_parse_extern(index, state));
        else    generic_parse_inc(state);
        
        i64 index = token_it_index(&state->it);
        if (index >= one_past_last_index){
            token = token_it_read(&state->it);
            if (token == 0){
                result = true;
            }
            break;
        }
    }
    
    if (result){
        index->nest_array = code_index_nest_ptr_array_from_list(state->arena, &index->nest_list);
        index->note_array = code_index_note_ptr_array_from_list(state->arena, &index->note_list);
    }
    
    return(result);
}