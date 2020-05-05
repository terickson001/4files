
 enum Comment_Note_Kind
{
    CommentNote_Todo,
    CommentNote_Note,
    CommentNote_Hack,
    CommentNote_Other,
};

struct Comment_Note
{
    Comment_Note_Kind kind;
    String_Const_u8 title;
    String_Const_u8 text;
    ID_Pos_Jump_Location location;
    
    Comment_Note *next;
};

struct Comment_Note_List
{
    Comment_Note *first;
    Comment_Note *last;
    
    i64 count;
};

global Face_ID todo_face = 0;

function void push_comment_note(Comment_Note_List *list, Comment_Note *note)
{
    sll_queue_push(list->first, list->last, note);
    list->count++;
}

function void push_comment_note(Comment_Note_List *list, Comment_Note note, Arena *arena)
{
    Comment_Note *new_note = push_array_zero(arena, Comment_Note, 1);
    *new_note = note;
    push_comment_note(list, new_note);
}

function void todo_list_render(Application_Links *app, Comment_Note_List *list, Rect_f32 rect)
{
    ARGB_Color background_color = fcolor_resolve(fcolor_id(defcolor_back));
    ARGB_Color border_color = fcolor_resolve(fcolor_id(defcolor_margin_active));
    ARGB_Color foreground_color = fcolor_resolve(fcolor_id(defcolor_text_default));
    
    if (todo_face == 0)
    {
        Face_ID default_face = get_face_id(app, 0);
        Face_Description desc = get_face_description(app, default_face);
        desc.parameters.pt_size = 12;
        todo_face = try_create_new_face(app, &desc);
    }
    
    Scratch_Block scratch(app);
    
    Face_Metrics metrics = get_face_metrics(app, todo_face);
    
    draw_rectangle(app, rect, 4.f, background_color);
    draw_rectangle_outline(app, rect, 4.f, 3.f, border_color);
    Vec2_f32 pos = {rect.x0+4, rect.y0+4};
    for (int i = 0; i < 5; i++)
    {
        pos = draw_string(app, todo_face, SCu8("TODO: "), pos, finalize_color(defcolor_pop1, 0));
        draw_string(app, todo_face, push_stringf(scratch, "%d", i), pos, foreground_color);
        
        pos.y += metrics.line_height;
        pos.x = rect.x0+4;
    }
}