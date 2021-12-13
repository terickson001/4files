// @todo(tyler): Scoped notes by package

function Code_Index_Note_Kind odin_ident_note(Code_Index_File *index, Generic_Parse_State *state, Token *ident)
{
    Scratch_Block scratch(state->app);
    String_Const_u8 token_as_string = push_token_lexeme(state->app, scratch, index->buffer, ident);
    
    for (Buffer_ID buf = get_buffer_next(state->app, 0, Access_Always);
         buf != 0;
         buf = get_buffer_next(state->app, buf, Access_Always))
    {
        Code_Index_File *file = code_index_get_file(buf);
        if (file == 0)
            continue;
        
        for (i32 i = 0; i < file->note_array.count; i++)
        {
            Code_Index_Note *note = file->note_array.ptrs[i];
            
            if (string_match(note->text, token_as_string, StringMatch_Exact))
                return note->note_kind;
        }
    }
    return CodeIndexNote_4coderCommand; // Invalid for Odin
}

function b32 odin_parse_decl(Code_Index_File *index, Generic_Parse_State *state, Code_Index_Nest *parent, Token *ident)
{
    generic_parse_inc(state);
    generic_parse_skip_soft_tokens(index, state);
    if (state->finished)
        return false;
    
    Token *peek = token_it_read(&state->it);
    if (peek != 0 && peek->sub_kind == TokenOdinKind_ColonColon)
    {
        generic_parse_inc(state);
        generic_parse_skip_soft_tokens(index, state);
        peek = token_it_read(&state->it);
        if (peek == 0)
            return false;
        switch (peek->sub_kind)
        {
            case TokenOdinKind_struct:
            case TokenOdinKind_union:
            case TokenOdinKind_enum: 
            case TokenOdinKind_distinct: {
                index_new_note(index, state, Ii64(ident), CodeIndexNote_Type, parent);
                return true;
            }break;
            
            case TokenOdinKind_force_inline:
            case TokenOdinKind_proc: {
                index_new_note(index, state, Ii64(ident), CodeIndexNote_Function, parent);
                return true;
            } break;
            
            default: {
                if (peek->kind == TokenBaseKind_Keyword)
                {
                    if (odin_is_builtin_type(peek))
                    {
                        index_new_note(index, state, Ii64(ident), CodeIndexNote_Type, parent);
                        return true;
                    }
                    else if (odin_is_builtin_proc(peek))
                    {
                        index_new_note(index, state, Ii64(ident), CodeIndexNote_Function, parent);
                        return true;
                    }
                    
                }
                else if (peek->kind == TokenBaseKind_Identifier)
                {
                    Code_Index_Note_Kind kind = odin_ident_note(index, state, peek);
                    if (kind != CodeIndexNote_4coderCommand)
                    {
                        index_new_note(index, state, Ii64(ident), kind, parent);
                        return true;
                    }
                }
            } break;
        }
    }
    
    return false;
}

function b32 odin_try_index(Code_Index_File *index, Generic_Parse_State *state)
{
    Token *token = token_it_read(&state->it);
    if (token->sub_kind == TokenOdinKind_Identifier)
    {
        generic_parse_inc(state);
        generic_parse_skip_soft_tokens(index, state);
        Token *peek = token_it_read(&state->it);
        state->it = token_iterator(state->it.user_id, state->it.tokens, state->it.count, token);
        
        if (peek->sub_kind == TokenOdinKind_ColonColon)
            return odin_parse_decl(index, state, 0, token);
    }
    
    return false;
}
