function b32 tc_generic_parse_full_input_breaks(Code_Index_File *index, Generic_Parse_State *state, i32 limit)
{
    b32 result = false;
    
    Managed_Scope scope = buffer_get_managed_scope(state->app, index->buffer);
    // Lexer_Kind *lex_kind_ptr = scope_attachment(state->app, scope, buffer_lex_kind, Lexer_Kind);
    Language **language = scope_attachment(state->app, scope, buffer_language, Language*);
    
    i64 first_index = token_it_index(&state->it);
    i64 one_past_last_index = first_index + limit;
    for (;;){
        generic_parse_skip_soft_tokens(index, state);
        Token *token = token_it_read(&state->it);
        
        if (token == 0 || state->finished){
            result = true;
            break;
        }
        
        if (token->kind == TokenBaseKind_Preprocessor) {
            Code_Index_Nest *nest = generic_parse_preprocessor(index, state);
            code_index_push_nest(&index->nest_list, nest);
        }
        else if (token->kind == TokenBaseKind_ScopeOpen) {
            Code_Index_Nest *nest = generic_parse_scope(index, state);
            code_index_push_nest(&index->nest_list, nest);
        }
        else if (token->kind == TokenBaseKind_ParentheticalOpen) {
            Code_Index_Nest *nest = generic_parse_paren(index, state);
            code_index_push_nest(&index->nest_list, nest);
        }
        else if (!(*language)->try_index(index, state, token))
        {
            generic_parse_inc(state);
        }
        /*
                else if (*lex_kind_ptr == LEXER_CPP) {
                    if (token->sub_kind == TokenCppKind_Struct ||
                        token->sub_kind == TokenCppKind_Union ||
                        token->sub_kind == TokenCppKind_Enum){
                        cpp_parse_type_structure(index, state, 0);
                    }
                    else if (token->sub_kind == TokenCppKind_Typedef) {
                        cpp_parse_type_def(index, state, 0);
                    }
                    else if (token->sub_kind == TokenCppKind_Identifier) {
                        cpp_parse_function(index, state, 0);
                    }
                    else {
                        generic_parse_inc(state);
                    }
                }
                else if (*lex_kind_ptr == LEXER_ODIN)
                {
                                if (token->sub_kind == TokenOdinKind_Identifier)
                                {
                                    generic_parse_inc(state);
                                    generic_parse_skip_soft_tokens(index, state);
                                    Token *peek = token_it_read(&state->it);
                                    Token *reset = peek;
                                    state->it = token_iterator(state->it.user_id, state->it.tokens, state->it.count, token);
                                    
                                    if (peek->sub_kind == TokenOdinKind_ColonColon)
                                        odin_parse_decl(index, state, 0, token);
                                    else
                                        generic_parse_inc(state);
                                }
                                else
                                {
                                    generic_parse_inc(state);
                                }
                                 
                    if (!odin_try_index(index, state, token))
                        generic_parse_inc(state);
                }
                else{
                    generic_parse_inc(state);
                }
                 */
        
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
