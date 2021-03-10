function ARGB_Color vec4_to_argb(Vec4_f32 color)
{
    u8 r = (u8)(color.r*255);
    u8 g = (u8)(color.g*255);
    u8 b = (u8)(color.b*255);
    u8 a = (u8)(color.a*255);
    ARGB_Color ret  = a << 24;
    ret |= r << 16;
    ret |= g << 8;
    ret |= b << 0;
    return ret;

}

function Vec4_f32 argb_to_vec4(ARGB_Color color)
{
    Vec4_f32 ret = V4f32((f32)((color & 0x00FF0000) >> 16),
                         (f32)((color & 0x0000FF00) >> 8),
                         (f32)((color & 0x000000FF) >> 0),
                         (f32)((color & 0xFF000000) >> 24)
                         );
    ret /= 255;
    printf("%08X -> %08X\n", color, vec4_to_argb(ret));
    return ret;
}

function f32 rgb_to_grey(Vec4_f32 color)
{
    return color.r * 0.21f +
        color.g * 0.72f +
        color.b * 0.04f;
}

function void paint_text_color_amber(Application_Links* app,
                                     Text_Layout_ID layout_id,
                                     Range_i64 range,
                                     ARGB_Color color)
{
    Vec4_f32 amber = argb_to_vec4(0xFFFFB000);
    Vec4_f32 in_color = argb_to_vec4(color);
    f32 grey = rgb_to_grey(in_color);
    ARGB_Color out_color = vec4_to_argb(amber * grey);
    paint_text_color(app, layout_id, range, out_color);
}

static void amberify(Application_Links *app)
{
    Arena *arena = &global_theme_arena;

    Vec4_f32 amber_vec = argb_to_vec4(0xFFFFB000);
    Color_Table original = active_color_table;
    Color_Table amber = make_color_table(app, arena);
    for (int i = 0; i < original.count; i++)
    {
        for (int j = 0; j < original.arrays[i].count; j++)
        {
            Vec4_f32 color_vec = argb_to_vec4(original.arrays[i].vals[j]);
            f32 grey = rgb_to_grey(color_vec);
            amber_vec.a = color_vec.a;
            amber.arrays[i].vals[j] = vec4_to_argb(amber_vec * grey);
        }
    }

    amber.arrays[defcolor_back] = make_colors(arena, 0xFF282828);

    amber.arrays[defcolor_margin] = make_colors(arena, 0xFF282828);
    amber.arrays[defcolor_margin_hover] = make_colors(arena, 0xFF282828);
    amber.arrays[defcolor_margin_active] = make_colors(arena, 0xFF282828);
    amber.arrays[defcolor_list_item] = make_colors(arena, 0xFF282828);
    amber.arrays[defcolor_list_item_hover] = make_colors(arena, 0xFF282828);
    amber.arrays[defcolor_list_item_active] = make_colors(arena, 0xFF282828);
    amber.arrays[defcolor_mark] = make_colors(arena, 0xFF282828);

    active_color_table = amber;
}

static void tc_paint_comment_keywords(Application_Links *app, Buffer_ID buffer, Text_Layout_ID text_layout_id, Token_Array *token_array)
{
    Comment_Highlight_Pair pairs[] = {
        {string_u8_litexpr("NOTE"), finalize_color(defcolor_comment_pop, 0)},
        {string_u8_litexpr("TODO"), finalize_color(defcolor_comment_pop, 1)},
    };
    draw_comment_highlights(app, buffer, text_layout_id,
                            token_array, pairs, ArrayCount(pairs));
}

static void tc_render_buffer(Application_Links *app, View_ID view_id, Face_ID face_id,
                             Buffer_ID buffer, Text_Layout_ID text_layout_id,
                             Rect_f32 rect)
{
    ProfileScope(app, "[TErickson] render buffer");

    View_ID active_view = get_active_view(app, Access_Always);
    b32 is_active_view = active_view == view_id;
    Rect_f32 prev_clip = draw_set_clip(app, rect);

    // NOTE(allen): Cursor shape
    Face_Metrics metrics = get_face_metrics(app, face_id);
    f32 cursor_roundness = (metrics.normal_advance*0.5f)*0.9f;
    f32 mark_thickness = 2.f;

    // Language **language = buffer_get_language(app, buffer);
    Token_Array token_array = get_token_array_from_buffer(app, buffer);
    if (token_array.tokens != 0)
    {
        language_paint_tokens(app, buffer, text_layout_id, &token_array);
        if (global_config.use_comment_keyword)
            tc_paint_comment_keywords(app, buffer, text_layout_id, &token_array);
    }
    else
    {
        Range_i64 visible_range = text_layout_get_visible_range(app, text_layout_id);
        paint_text_color_fcolor(app, text_layout_id, visible_range, fcolor_id(defcolor_text_default));
    }

    i64 cursor_pos = view_correct_cursor(app, view_id);
    view_correct_mark(app, view_id);

    // NOTE(allen): Scope highlight
    if (global_config.use_scope_highlight){
        Color_Array colors = finalize_color_array(defcolor_back_cycle);
        draw_scope_highlight(app, buffer, text_layout_id, cursor_pos, colors.vals, colors.count);
    }

    if (global_config.use_error_highlight || global_config.use_jump_highlight){
        // NOTE(allen): Error highlight
        String_Const_u8 name = string_u8_litexpr("*compilation*");
        Buffer_ID compilation_buffer = get_buffer_by_name(app, name, Access_Always);
        if (global_config.use_error_highlight){
            draw_jump_highlights(app, buffer, text_layout_id, compilation_buffer,
                                 fcolor_id(defcolor_highlight_junk));
            tc_render_error_messages(app, buffer, compilation_buffer, active_view, text_layout_id);
        }

        // NOTE(allen): Search highlight
        if (global_config.use_jump_highlight){
            Buffer_ID jump_buffer = get_locked_jump_buffer(app);
            if (jump_buffer != compilation_buffer){
                draw_jump_highlights(app, buffer, text_layout_id, jump_buffer,
                                     fcolor_id(defcolor_highlight_white));
            }
        }
    }

    if (is_active_view)
        tc_render_scopeline(app, buffer, active_view, text_layout_id);
    function_index_render_preview(app, buffer, active_view, text_layout_id);


    // NOTE(allen): Color parens
    if (global_config.use_paren_helper){
        Color_Array colors = finalize_color_array(defcolor_text_cycle);
        draw_paren_highlight(app, buffer, text_layout_id, cursor_pos, colors.vals, colors.count);
    }

    // NOTE(allen): Line highlight
    if (global_config.highlight_line_at_cursor && is_active_view){
        i64 line_number = get_line_number_from_pos(app, buffer, cursor_pos);
        draw_line_highlight(app, text_layout_id, line_number,
                            fcolor_id(defcolor_highlight_cursor_line));
    }

    // NOTE(allen): Whitespace highlight
    b64 show_whitespace = false;
    view_get_setting(app, view_id, ViewSetting_ShowWhitespace, &show_whitespace);
    if (show_whitespace){
        if (token_array.tokens == 0){
            draw_whitespace_highlight(app, buffer, text_layout_id, cursor_roundness);
        }
        else{
            draw_whitespace_highlight(app, text_layout_id, &token_array, cursor_roundness);
        }
    }

    // NOTE(allen): Cursor
    switch (fcoder_mode){
        case FCoderMode_Original:
        {
            draw_original_4coder_style_cursor_mark_highlight(app, view_id, is_active_view, buffer, text_layout_id, cursor_roundness, mark_thickness);
        } break;
        case FCoderMode_NotepadLike:
        {
            draw_notepad_style_cursor_highlight(app, view_id, buffer, text_layout_id, cursor_roundness);
        } break;
    }

    // NOTE(allen): Fade ranges
    paint_fade_ranges(app, text_layout_id, buffer);

    // NOTE(allen): put the actual text on the actual screen
    draw_text_layout_default(app, text_layout_id);

    draw_set_clip(app, prev_clip);
}
