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
            case TokenOdinKind_type:
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
    else if (peek != 0 && peek->sub_kind == TokenOdinKind_Colon)
    {
        generic_parse_inc(state);
        generic_parse_skip_soft_tokens(index, state);
        peek = token_it_read(&state->it);
        if (peek == 0)
            return false;

        if (peek->kind == TokenBaseKind_Keyword && odin_is_builtin_proc(peek))
        {
            index_new_note(index, state, Ii64(ident), CodeIndexNote_Function, parent);
            return true;
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
    }

    return false;
}

function Code_Index_Nest *odin_parse_statement(Code_Index_File *index, Generic_Parse_State *state)
{
    Token *token = token_it_read(&state->it);
	Code_Index_Nest *result = push_array_zero(state->arena, Code_Index_Nest, 1);
	result->kind = CodeIndexNest_Statement;
	result->open = Ii64(token->pos);
	result->close = Ii64(max_i64);
	result->file = index;

	state->in_statement = true;

    Token *prev_non_whitespace = token;
	for (;;){
		token = token_it_read(&state->it);
		if (token->sub_kind == TokenOdinKind_EOL && (odin_check_semicolon(prev_non_whitespace) || prev_non_whitespace == token)) {
            result->is_closed = true;
			result->close = Ii64(token);
			generic_parse_inc(state);
			break;
		}
		generic_parse_skip_soft_tokens(index, state);
		token = token_it_read(&state->it);
		prev_non_whitespace = token;
		if (token == 0 || state->finished){
			break;
		}

		if (state->in_preprocessor){
			if (!HasFlag(token->flags, TokenBaseFlag_PreprocessorBody) ||
				token->kind == TokenBaseKind_Preprocessor){
				result->is_closed = true;
				result->close = Ii64(token->pos);
				break;
			}
		}
		else{
			if (token->kind == TokenBaseKind_Preprocessor){
				result->is_closed = true;
				result->close = Ii64(token->pos);
				break;
			}
		}

		if (token->kind == TokenBaseKind_ScopeOpen ||
			token->kind == TokenBaseKind_ScopeClose ||
			token->kind == TokenBaseKind_ParentheticalOpen){
			result->is_closed = true;
			result->close = Ii64(token->pos);
			break;
		}

		if (token->kind == TokenBaseKind_StatementClose) {
			result->is_closed = true;
			result->close = Ii64(token);
			generic_parse_inc(state);
			break;
		}

		generic_parse_inc(state);
	}

	state->in_statement = false;

	return(result);
}

function b32 odin_try_index(Code_Index_File *index, Generic_Parse_State *state)
{
    Token *token = token_it_read(&state->it);
    if (token->sub_kind == TokenOdinKind_Identifier)
    {
        /*
                generic_parse_inc(state);
                generic_parse_skip_soft_tokens(index, state);
        */
        if (!odin_parse_decl(index, state, 0, token))
        {
            state->it = token_iterator(state->it.user_id, state->it.tokens, state->it.count, token);
            return false;
        }
        return true;
        /*
                if (peek->sub_kind == TokenOdinKind_ColonColon)
                    return odin_parse_decl(index, state, 0, token)
        */
    }
    else if (token->sub_kind == TokenOdinKind_foreign)
    {

        generic_parse_inc(state);
        generic_parse_skip_soft_tokens(index, state);
        Token *peek = token_it_read(&state->it);
        if (peek->sub_kind == TokenOdinKind_import)
        {
            state->it = token_iterator(state->it.user_id, state->it.tokens, state->it.count, token);
            return false;
        }
        else if (peek->kind == TokenBaseKind_Identifier)
        {
            generic_parse_inc(state);
            generic_parse_skip_soft_tokens(index, state);
            peek = token_it_read(&state->it);
        }

        if (peek->kind == TokenBaseKind_ScopeOpen)
        {
            generic_parse_inc(state);
            generic_parse_skip_soft_tokens(index, state);
            return true;
        }
    }

    return false;
}
