
function b32 nasm_is_first_token(Code_Index_File *index, Generic_Parse_State *state)
{
     if (token_it_index(&state->it) == 0)
     return true;
     
     Token *curr = token_it_read(&state->it);
     Token *prev = curr-1;
     while (prev > state->it.tokens && (prev->kind == TokenBaseKind_Whitespace || prev->kind == TokenBaseKind_Comment))
     prev--;
     
     i64 curr_line = get_line_number_from_pos(state->app, index->buffer, curr->pos);
     i64 prev_line = get_line_number_from_pos(state->app, index->buffer, prev->pos);
     
     return prev_line < curr_line;
}

function void nasm_break_scope(Code_Index_File *index, Generic_Parse_State *state, Token *center)
{
     Code_Index_Nest *prev_scope = index->nest_list.last;
     prev_scope->close = Ii64(center-1);
     
     Code_Index_Nest *new_scope = push_array_zero(state->arena, Code_Index_Nest, 1);
     new_scope->kind  = CodeIndexNest_Scope;
     new_scope->open  = Ii64(center+1);
     new_scope->close = Ii64(max_i64);
     code_index_push_nest(&index->nest_list, new_scope);
}

function b32 nasm_try_index(Code_Index_File *index, Generic_Parse_State *state)
{
     if (index->nest_list.first == 0)
     {
         Code_Index_Nest *first_scope = push_array_zero(state->arena, Code_Index_Nest, 1);
         first_scope->kind  = CodeIndexNest_Scope;
         first_scope->open  = Ii64((i64)-1);
         first_scope->close = Ii64(max_i64);
         code_index_push_nest(&index->nest_list, first_scope);
     }
     
     Token *token = token_it_read(&state->it);
     if (token->kind == TokenBaseKind_Identifier &&
     nasm_is_first_token(index, state))
     {
         generic_parse_inc(state);
         generic_parse_skip_soft_tokens(index, state);
         Token *peek = token_it_read(&state->it);
         
         if (peek->sub_kind == TokenNasmKind_Colon)
         {
             nasm_break_scope(index, state, token);
             index_new_note(index, state, Ii64(token), CodeIndexNote_Function, 0);
             
             generic_parse_inc(state);
             return true;
         }
         else
         {
             state->it = token_iterator(state->it.user_id,
                                        state->it.tokens,
                                        state->it.count,
             token);
         }
     }
     else if (token->kind == TokenBaseKind_Preprocessor &&
     nasm_is_first_token(index, state))
     {
         nasm_break_scope(index, state, token);
     }
     
     return false;
}
