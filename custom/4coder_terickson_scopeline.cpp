#define SCOPELINE_MIN_DISTANCE 1

static b32 use_scopeline = true;

CUSTOM_COMMAND_SIG(toggle_scopeline)
CUSTOM_DOC("Toggle scope annotations")
{
    use_scopeline = !use_scopeline;
}

function void tc_render_scopeline(Application_Links *app, Buffer_ID buffer, View_ID view, Text_Layout_ID text_layout_id)
{
    if (!use_scopeline || !buffer)
        return;
    
    ProfileScope(app, "[TErickson] ScopeLine");
    
    Scratch_Block scratch(app);
    
    Managed_Scope scope = buffer_get_managed_scope(app, buffer);
    Token_Array *tokens = scope_attachment(app, scope, attachment_tokens, Token_Array);
    Code_Index_File *index = code_index_get_file(buffer);
    if (!index) return;
    
    Face_ID face_id = get_face_id(app, buffer);
    Face_Metrics metrics = get_face_metrics(app, face_id);
    
    i64 cursor_pos = view_get_cursor_pos(app, view);
    i64 line = get_line_number_from_pos(app, buffer, cursor_pos);
    Range_i64 line_range = get_line_pos_range(app, buffer, line);
    
    Code_Index_Nest *nest = code_index_get_nest(index, line_range.min);
    if (!nest) return;
    
    i64 open_line = get_line_number_from_pos(app, buffer, nest->open.max);
    i64 close_line = get_line_number_from_pos(app, buffer, nest->close.min);
    if (range_contains(line_range, nest->close.min) &&
        close_line - open_line >= SCOPELINE_MIN_DISTANCE &&
        (nest->kind == CodeIndexNest_Scope || nest->kind == CodeIndexNest_Paren))
    {
        
        b32 use_prev_line = false;
        i64 token_index = token_index_from_pos(tokens, nest->open.min);
        while (--token_index)
        {
            Token tok = tokens->tokens[token_index];
            i64 tok_line = get_line_number_from_pos(app, buffer, tok.pos);
            
            if (tok_line < open_line-1)
                break;
            if (tok.kind != TokenBaseKind_Whitespace && tok.kind != TokenBaseKind_Comment)
            {
                if (tok_line < open_line)
                    use_prev_line = true;
                break;
            }
        }
        
        String_Const_u8 str = string_skip_chop_whitespace(push_buffer_line(app, scratch, buffer, open_line));
        
        Rect_f32 eol = text_layout_character_on_screen(app, text_layout_id, line_range.max);
        
        u32 color = finalize_color(defcolor_comment, 0);
        color &= 0x00ffffff;
        color |= 0x80000000;
        Vec2_f32 draw_pos =
        {
            eol.x0 + metrics.space_advance,
            eol.y0,
        };
        
        Language *lang = *buffer_get_language(app, buffer);
        if (lang && lang->comment_delims.line.size != 0)
        {
            draw_pos = draw_string(app, face_id, lang->comment_delims.line, draw_pos, color);
            draw_pos.x += metrics.space_advance;
        }
        
        if (use_prev_line)
        {
            String_Const_u8 prev_str = string_skip_chop_whitespace(push_buffer_line(app, scratch, buffer, open_line-1));
            draw_pos = draw_string(app, face_id, prev_str, draw_pos, color);
            draw_pos.x += metrics.space_advance;
        }
        
        draw_string(app, face_id, str, draw_pos, color);
    }
}