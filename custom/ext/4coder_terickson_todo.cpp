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

struct Comment_Note_File
{
    Comment_Note *first;
    Comment_Note *last;

    Buffer_ID buffer;
    i64 count;
    b32 finished;
    b32 updated;

    Range_i64 out_buffer_range;

    Arena *arena;
    Comment_Note_File *next;
};

typedef Table_u64_u64 Comment_Note_Table;

global Face_ID todo_face = 0;
global Comment_Note_Table todo_note_table;

global Buffer_Identifier todo_buffer_id = buffer_identifier(SCu8("*TODO*"));

function void todo_handle_comment(Application_Links *app, Arena *arena, Code_Index_File *index,
                                  Token *token, String_Const_u8 contents);
function void todo_post_index(Application_Links *app, Code_Index_File *index);
function void init_ext_todo()
{
    language_push_hook(Hook_HandleComment, (Void_Func *)&todo_handle_comment);
    language_push_hook(Hook_PostIndex, (Void_Func *)&todo_post_index);
}

function void push_comment_note(Comment_Note_File *file, Comment_Note *note)
{
    sll_queue_push(file->first, file->last, note);
    file->count++;
    file->updated = true;
}

function void push_comment_note(Comment_Note_File *file, Comment_Note note, Arena *arena)
{
    Comment_Note *new_note = push_array_zero(arena, Comment_Note, 1);
    *new_note = note;
    push_comment_note(file, new_note);
}

function Comment_Note_File *get_comment_note_file(Comment_Note_Table *table, Buffer_ID buffer)
{
    Comment_Note_File *file = 0;
    Table_Lookup lookup = table_lookup(table, buffer);
    b32 res = table_read(table, lookup, (u64*)&file);
    if (res)
        return file;
    return 0;
}

function void set_comment_note_file(Comment_Note_Table *table, Buffer_ID buffer, Comment_Note_File *file)
{
    if (!table->allocator)
        *table = make_table_u64_u64(tc_global_arena.base_allocator, 32);
    table_erase(table, buffer);
    table_insert(table, buffer, HandleAsU64(file));
}

function void todo_handle_comment(Application_Links *app, Arena *arena, Code_Index_File *index,
                                  Token *token, String_Const_u8 contents)
{
    Comment_Note_File *file = get_comment_note_file(&todo_note_table, index->buffer);
    if (!file || file->finished)
    {
        Range_i64 out_buffer_range = {};
        if (file)
        {
            out_buffer_range = file->out_buffer_range;
            release_arena(app, file->arena);
        }
        Arena *file_arena = reserve_arena(app);
        file = push_array_zero(file_arena, Comment_Note_File, 1);
        file->buffer = index->buffer;
        file->out_buffer_range = out_buffer_range;
        file->arena = file_arena;
        set_comment_note_file(&todo_note_table, index->buffer, file);
    }

    String_Const_u8 TODO_STR = SCu8("@todo");
    String_Const_u8 HACK_STR = SCu8("@hack");
    String_Const_u8 str = string_substring(contents, Ii64(token));
    i64 start_idx = 0;
    for (;;)
    {
        Comment_Note note = {};
        i64 idx = string_find_first(str, TODO_STR, StringMatch_CaseInsensitive);
        if (idx != (i64)str.size)
        {
            note.kind = CommentNote_Todo;
            note.title = push_string_copy(&tc_global_arena, string_substring(str, Ii64(idx, idx+TODO_STR.size)));
            note.location = {index->buffer, token->pos+start_idx+idx};
            start_idx += idx+TODO_STR.size;

            goto found;
        }
        idx = string_find_first(str, HACK_STR, StringMatch_CaseInsensitive);
        if (idx != (i64)str.size)
        {
            note.kind = CommentNote_Hack;
            note.title = push_string_copy(&tc_global_arena, string_substring(str, Ii64(idx, idx+HACK_STR.size)));
            note.location = {index->buffer, token->pos+start_idx+idx};
            start_idx += idx+HACK_STR.size;

            goto found;
        }
        break;



        found:

        idx = string_find_first(str, idx, ':');
        if (idx == (i64)str.size)
        {
            push_comment_note(file, note, file->arena);
            break;
        }
        note.text = push_string_copy(file->arena, string_skip_chop_whitespace(string_skip(str, idx+1)));
        push_comment_note(file, note, file->arena);
        break;
    }
}

function void todo_post_index(Application_Links *app, Code_Index_File *index)
{
    Comment_Note_File *file = get_comment_note_file(&todo_note_table, index->buffer);
    if (file)
        file->finished = true;
}

function void write_todos_to_buffer(Application_Links *app, Buffer_ID buffer, Comment_Note_Table *table)
{
    i64 idx = 0;
    for (Buffer_ID buf = get_buffer_next(app, 0, Access_Always);
         buf != 0;
         buf = get_buffer_next(app, buf, Access_Always))
    {
        Comment_Note_File *file = get_comment_note_file(&todo_note_table, buf);
        if (!file) continue;
        if (!file->updated) continue;
        if (!file->finished) continue;
        file->updated = false;
        buffer_replace_range(app, buffer, file->out_buffer_range, string_u8_empty);
        file->out_buffer_range = Ii64(idx);

        for (Comment_Note *note = file->first;
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
            if (note->text.size == 0 || note->text.str[note->text.size-1] != '\n')
            {
                buffer_replace_range(app, buffer, Ii64(idx), SCu8("\n"));
                idx++;
            }
        }
        file->out_buffer_range.max = idx-1;
    }

}

global bool todo_window_open = false;
function void todo_list_render(Application_Links *app, Comment_Note_Table *table, Rect_f32 rect)
{
    if (!todo_window_open) return;

    ARGB_Color background_color = fcolor_resolve(fcolor_id(defcolor_back));
    ARGB_Color border_color = fcolor_resolve(fcolor_id(defcolor_margin_active));
    ARGB_Color foreground_color = fcolor_resolve(fcolor_id(defcolor_text_default));
    ARGB_Color title_color = fcolor_resolve(fcolor_id(defcolor_pop1));
    background_color = (background_color & 0x00FFFFFF) | 0x80000000;
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
    write_todos_to_buffer(app, buffer, &todo_note_table);

    draw_rectangle(app, rect, 4.f, background_color);
    draw_rectangle_outline(app, rect, 4.f, 3.f, border_color);
    rect.x0 += 3; rect.x1 -= 6;
    rect.y0 += 3; rect.y1 -= 6;
    Text_Layout_ID text_layout = text_layout_create(app, buffer, rect, {0, 0});

    Face_ID old_face = get_face_id(app, buffer);
    buffer_set_face(app, buffer, todo_face);
    Rect_f32 old_clip = draw_set_clip(app, rect);
    for (Buffer_ID buf = get_buffer_next(app, 0, Access_Always);
         buf != 0;
         buf = get_buffer_next(app, buf, Access_Always))
    {
        Comment_Note_File *file = get_comment_note_file(&todo_note_table, buf);
        if (!file) continue;

        for (Comment_Note *note = file->first;
             note != 0;
             note = note->next)
        {
            paint_text_color(app, text_layout, Ii64(note->todo_buf_title_pos, note->todo_buf_text_pos), title_color);
            paint_text_color(app, text_layout, Ii64_size(note->todo_buf_text_pos, note->text.size), foreground_color);
        }
    }

    draw_text_layout_default(app, text_layout);
    draw_set_clip(app, old_clip);
    buffer_set_face(app, buffer, old_face);
}

CUSTOM_UI_COMMAND_SIG(toggle_todo_window)
CUSTOM_DOC("Toggle TODO window")
{
    todo_window_open = !todo_window_open;
}

CUSTOM_UI_COMMAND_SIG(jump_to_note)
CUSTOM_DOC("List all comment notes and jump to one chosen by the user.")
{
    char *query = "Note:";

    Scratch_Block scratch(app);
    Lister_Block lister(app, scratch);
    lister_set_query(lister, query);
    lister_set_default_handlers(lister);

    for (Buffer_ID buffer = get_buffer_next(app, 0, Access_Always);
         buffer != 0;
         buffer = get_buffer_next(app, buffer, Access_Always))
    {
        Comment_Note_File *file = get_comment_note_file(&todo_note_table, buffer);
        if (!file) continue;

        for (Comment_Note *note = file->first;
             note != 0;
             note = note->next)
        {
            Tiny_Jump *jump = push_array(scratch, Tiny_Jump, 1);
            jump->buffer = buffer;
            jump->pos = note->location.pos;

            String_Const_u8 sort = {};
            switch (note->kind){
                case CommentNote_Todo:
                {
                    sort = string_u8_litexpr("TODO");
                }break;
                case CommentNote_Hack:
                {
                    sort = string_u8_litexpr("HACK");
                }break;
                case CommentNote_Note:
                {
                    sort = string_u8_litexpr("NOTE");
                }break;
            }
            lister_add_item(lister, note->text, sort, jump, 0);
        }
    }

    Lister_Result l_result = run_lister(app, lister);
    Tiny_Jump result = {};
    if (!l_result.canceled && l_result.user_data != 0){
        block_copy_struct(&result, (Tiny_Jump*)l_result.user_data);
    }

    if (result.buffer != 0){
        View_ID view = get_this_ctx_view(app, Access_Always);
        jump_to_location(app, view, result.buffer, result.pos);
    }
}
