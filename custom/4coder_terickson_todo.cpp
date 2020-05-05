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
    i64 todo_buf_title_pos;
    i64 todo_buf_text_pos;
    
    Comment_Note *next;
};

struct Comment_Note_List
{
    Comment_Note *first;
    Comment_Note *last;
    
    i64 count;
};

global Face_ID todo_face = 0;
global Comment_Note_List todo_note_list;

global Buffer_Identifier todo_buffer_id = buffer_identifier(SCu8("*TODO*"));

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

function void todo_handle_comment(Application_Links *app, Arena *arena, Code_Index_File *index,
                                  Token *token, String_Const_u8 contents)
{
    String_Const_u8 TODO_STR = SCu8("@todo");
    String_Const_u8 str = string_substring(contents, Ii64(token));
    i64 start_idx = 0;
    for (;;)
    {
        i64 idx = string_find_first(str, TODO_STR, StringMatch_CaseInsensitive);
        if (idx == str.size) break;
        
        Comment_Note note = {};
        note.kind = CommentNote_Todo;
        note.title = push_string_copy(tc_global_arena, string_substring(str, Ii64(idx, idx+TODO_STR.size)));
        note.location = {index->buffer, token->pos+start_idx+idx};
        
        start_idx += idx+TODO_STR.size;
        
        idx = string_find_first(str, idx, ':');
        if (idx == str.size)
        {
            push_comment_note(&todo_note_list, note, tc_global_arena);
            continue;
        }
        note.text = push_string_copy(tc_global_arena, string_skip_chop_whitespace(string_skip(str, idx+1)));
        push_comment_note(&todo_note_list, note, tc_global_arena);
        break;
    }
}

function void write_todos_to_buffer(Application_Links *app, Buffer_ID buffer, Comment_Note_List *list)
{
    clear_buffer(app, buffer);
    i64 idx = 0;
    for (Comment_Note *note = list->first;
         note != 0;
         note = note->next)
    {
        note->todo_buf_title_pos = idx;
        switch (note->kind)
        {
            case CommentNote_Todo:
            buffer_replace_range(app, buffer, Ii64(idx), SCu8("TODO: "));
            idx += 6;
            break;
            
            case CommentNote_Hack:
            buffer_replace_range(app, buffer, Ii64(idx), SCu8("HACK: "));
            idx += 6;
            break;
            
            default: continue;
            
        }
        note->todo_buf_text_pos = idx;
        buffer_replace_range(app, buffer, Ii64(idx), note->text);
        idx += note->text.size;
        if (note->text.str[note->text.size-1] != '\n')
        {
            buffer_replace_range(app, buffer, Ii64(idx), SCu8("\n"));
            idx++;
        }
    }
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
    
    Buffer_ID buffer = get_buffer_by_name(app, SCu8("*TODO*"), Access_Always);
    if (!buffer_exists(app, buffer))
    {
        buffer = create_buffer(app, SCu8("*TODO*"), BufferCreate_AlwaysNew|BufferCreate_Background);
        buffer_set_setting(app, buffer, BufferSetting_Unimportant, true);
        buffer_set_setting(app, buffer, BufferSetting_ReadOnly, true);
    }
    write_todos_to_buffer(app, buffer, &todo_note_list);
    Text_Layout_ID text_layout = text_layout_create(app, buffer, rect, {0, 0});
    
    draw_rectangle(app, rect, 4.f, background_color);
    draw_rectangle_outline(app, rect, 4.f, 3.f, border_color);
    
    Face_ID old_face = get_face_id(app, buffer);
    buffer_set_face(app, buffer, todo_face);
    Rect_f32 old_clip = draw_set_clip(app, rect);
    Vec2_f32 pos = {rect.x0+4, rect.y0+4};
    for (Comment_Note *note = todo_note_list.first;
         note != 0;
         note = note->next)
    {
        paint_text_color(app, text_layout, Ii64(note->todo_buf_title_pos, note->todo_buf_text_pos), finalize_color(defcolor_pop1, 0));
        paint_text_color(app, text_layout, Ii64_size(note->todo_buf_text_pos, note->text.size), foreground_color);
    }
    draw_text_layout_default(app, text_layout);
    draw_set_clip(app, old_clip);
    buffer_set_face(app, buffer, old_face);
}