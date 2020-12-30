
#ifdef EXT_FUNCTION_INDEX
function Function_Index *odin_parse_function__findexer(Application_Links *app, Code_Index_Note *note, Arena *arena)
{
    Buffer_ID buffer = note->file->buffer;
    Token_Array tokens = get_token_array_from_buffer(app, buffer);
    i64 idx = token_index_from_pos(&tokens, note->pos.max+1);
    skip_to_params:
    switch (tokens.tokens[idx].sub_kind)
    {
        case TokenOdinKind_ColonColon:
        case TokenOdinKind_Whitespace:
        case TokenOdinKind_inline:
        case TokenOdinKind_proc:
        case TokenOdinKind_LiteralString:
        idx++;
        goto skip_to_params;
        
        default:
        break;
    }
    
    Code_Index_Nest *nest = code_index_get_nest(note->file, tokens.tokens[idx].pos+1);
    if (!nest || nest->kind != CodeIndexNest_Paren)
        return 0;
    
    Range_i64 param_range = Ii64(nest->open.max, nest->close.min);
    
    Function_Index *index = push_array_zero(arena, Function_Index, 1);
    index->note = note;
    index->name = note->text;
    index->parameters = {0};
    
    idx = token_index_from_pos(&tokens, param_range.min);
    
    String_Const_u8 param_string = push_buffer_range(app, arena, buffer, param_range);
    Function_Parameter *param = 0;
    
    while (tokens.tokens[idx].pos < param_range.max)
    {
        String_Const_u8 prefix = {};
        String_Const_u8 type = {};
        String_Const_u8 postfix = {};
        String_Const_u8 name = {};
        String_Const_u8 default_value = {};
        
        b32 is_specialization = false;
        String_Const_u8 prefix_save = {};
        String_Const_u8 type_save = {};
        
        while (tokens.tokens[idx].kind == TokenBaseKind_Whitespace ||
               tokens.tokens[idx].kind == TokenBaseKind_Comment ||
               tokens.tokens[idx].sub_kind == TokenOdinKind_using) idx++;
        
        //// PARAM NAME
        if (tokens.tokens[idx].sub_kind == TokenOdinKind_Identifier || odin_is_builtin_proc(&tokens.tokens[idx]))
        {
            name.str = &param_string.str[tokens.tokens[idx].pos - param_range.min];
            name.size = tokens.tokens[idx].size;
            idx++;
        }
        
        while (tokens.tokens[idx].kind == TokenBaseKind_Whitespace ||
               tokens.tokens[idx].kind == TokenBaseKind_Comment) idx++;
        
        if (tokens.tokens[idx].sub_kind != TokenOdinKind_Colon)
        {
            type = name;
            name = {0};
            goto type_postfix;
        }
        else
        {
            idx++;
        }
        
        //// TYPE PREFIX
        type_prefix:
        switch (tokens.tokens[idx].sub_kind)
        {
            case TokenOdinKind_Whitespace:
            idx++;
            goto type_prefix;
            
            case TokenOdinKind_Carrot:
            if (prefix.str == 0)
                prefix.str = &param_string.str[tokens.tokens[idx].pos - param_range.min];
            idx++;
            goto type_prefix;
            
            case TokenOdinKind_BrackOp:
            {
                if (prefix.str == 0)
                    prefix.str = &param_string.str[tokens.tokens[idx].pos - param_range.min];
                Code_Index_Nest *nest = code_index_get_nest(note->file, tokens.tokens[idx].pos);
                idx = token_index_from_pos(&tokens, nest->close.max);
                goto type_prefix;
            }
            
            default:
            if (prefix.str != 0)
                prefix.size = &param_string.str[Ii64(&tokens.tokens[idx-1]).max-param_range.min] - prefix.str;
            break;
        }
        while (tokens.tokens[idx].kind == TokenBaseKind_Whitespace ||
               tokens.tokens[idx].kind == TokenBaseKind_Comment) idx++;
        
        //// TYPE NAME
        type_name:
        switch (tokens.tokens[idx].sub_kind)
        {
            case TokenOdinKind_Identifier:
            type.str = &param_string.str[tokens.tokens[idx].pos - param_range.min];
            type.size = tokens.tokens[idx].size;
            idx++;
            break;
            
            default:
            if (odin_is_builtin_type(&tokens.tokens[idx]))
            {
                type.str = &param_string.str[tokens.tokens[idx].pos - param_range.min];
                type.size = tokens.tokens[idx].size;
                idx++;
            }
            break;
        }
        
        //// TYPE POSTFIX
        type_postfix:
        switch (tokens.tokens[idx].sub_kind)
        {
            
            case TokenOdinKind_ParenOp:
            {
                if (postfix.str == 0)
                    postfix.str = &param_string.str[tokens.tokens[idx].pos - param_range.min];
                Code_Index_Nest *nest = code_index_get_nest(note->file, tokens.tokens[idx].pos);
                idx = token_index_from_pos(&tokens, nest->close.max);
                goto type_postfix;
            }
            
            case TokenOdinKind_Div:
            {
                
                if (is_specialization) goto skip_rest;
                if (postfix.str == 0)
                    postfix.str = &param_string.str[tokens.tokens[idx].pos - param_range.min];
                idx++;
                is_specialization = true;
                prefix_save = prefix;
                type_save = type;
                goto type_prefix;
            }
            
            default:
            if (postfix.str != 0)
                postfix.size = &param_string.str[Ii64(&tokens.tokens[idx-1]).max-param_range.min] - postfix.str;
            break;
        }
        postfix = string_skip_chop_whitespace(postfix);
        
        if (is_specialization)
        {
            prefix = prefix_save;
            type = type_save;
        }
        
        while (tokens.tokens[idx].kind == TokenBaseKind_Whitespace ||
               tokens.tokens[idx].kind == TokenBaseKind_Comment) idx++;
        
        //// DEFAULT VALUE
        if (tokens.tokens[idx].sub_kind != TokenOdinKind_Eq) goto skip_rest;
        idx++;
        print_message(app, SCu8("Parsing default value\n"));
        param_default:
        switch (tokens.tokens[idx].sub_kind)
        {
            case TokenOdinKind_Whitespace: 
            {
                idx++;
                goto param_default;
            }
            
            case TokenOdinKind_ParenOp:
            {
                if (default_value.str == 0)
                    default_value.str = &param_string.str[tokens.tokens[idx].pos - param_range.min];
                Code_Index_Nest *nest = code_index_get_nest(note->file, tokens.tokens[idx].pos);
                idx = token_index_from_pos(&tokens, nest->close.max);
                goto param_default;
            }
            
            case TokenOdinKind_Comma:
            case TokenOdinKind_ParenCl:
            {
                if (default_value.str != 0)
                    default_value.size = &param_string.str[Ii64(&tokens.tokens[idx-1]).max-param_range.min] - default_value.str;
                goto skip_rest;
            }
            
            default:
            if (default_value.str == 0)
                default_value.str = &param_string.str[tokens.tokens[idx].pos - param_range.min];
            idx++;
            goto param_default;
        }
        default_value = string_skip_chop_whitespace(default_value);
        
        //// SKIP TO COMMA or PAREN
        skip_rest:
        switch (tokens.tokens[idx].sub_kind)
        {
            case TokenOdinKind_Comma:
            case TokenOdinKind_ParenCl:
            idx++;
            break;
            
            default:
            idx++;
            goto skip_rest;
        }
        
        param = push_array_zero(arena, Function_Parameter, 1);
        if (name.data) param->name = push_string_copy(arena, name);
        if (prefix.data) param->prefix = push_string_copy(arena, prefix);
        if (postfix.data) param->postfix = push_string_copy(arena, postfix);
        if (type.data) param->type = push_string_copy(arena, type);
        if (default_value.data) param->default_value = push_string_copy(arena, default_value);
        sll_queue_push(index->parameters.first, index->parameters.last, param);
        print_message(app, default_value);
    }
    
    return index;
}

function List_String_Const_u8 odin_parameter_strings(Function_Index *index, Arena *arena)
{
    List_String_Const_u8 param_strings = {};//push_array(arena, List_String_Const_u8, 1);
    for (Function_Parameter *param = index->parameters.first;
         param != 0;
         param = param->next)
    {
        string_list_pushf(arena, &param_strings, "%.*s%s%s%s%.*s%.*s%.*s%s%s%.*s",
                          string_expand(param->name),
                          param->type.size==0&&param->default_value.size!=0?" ":"",
                          param->name.size!=0&&(param->type.size!=0||param->default_value.size!=0)?":":"",
                          param->name.size!=0 && param->type.size!=0?" ":"",
                          string_expand(param->prefix),
                          string_expand(param->type),
                          string_expand(param->postfix),
                          param->type.size!=0 && param->default_value.size!=0?" ":"",
                          param->default_value.size!=0?"= ":"",
                          string_expand(param->default_value)
                          );
    }
    
    return param_strings;
}
#undef string_expand_nonnull
static Language_Function_Indexer odin_function_indexer =
{
    .lang = &language_def_odin,
    .parse_function = odin_parse_function__findexer,
    .parameter_strings = odin_parameter_strings,
    .delims {
        TokenOdinKind_ParenOp, SCu8("("),
        TokenOdinKind_ParenCl, SCu8(")"),
        TokenOdinKind_Comma, SCu8(",")
    },
};
#endif
