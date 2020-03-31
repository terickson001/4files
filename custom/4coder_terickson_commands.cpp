CUSTOM_COMMAND_SIG(token_at_cursor)
CUSTOM_DOC("Shows info about the token under the cursor.")
{
    View_ID view = get_active_view(app, Access_Always);
    i64 cursor = view_get_cursor_pos(app, view);
    Buffer_ID buffer = view_get_buffer(app, view, Access_Always);
    
    Token *token = get_token_from_pos(app, buffer, cursor);
    
    Managed_Scope scope = buffer_get_managed_scope(app, buffer);
    Language **language = scope_attachment(app, scope, buffer_language, Language*);
    
    char *base_name = token_base_kind_names[token->kind];
    char *sub_name = (*language)->token_kind_names[token->sub_kind];
    
    char message[512];
    snprintf(message, 512, "Token:\n  Base: %s\n  Sub: %s\n", base_name, sub_name);
    print_message(app, SCu8(message));
}