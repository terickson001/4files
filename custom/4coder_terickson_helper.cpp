function Arena *reserve_arena(Thread_Context *tctx, u64 chunk_size, u64 align)
{
    Arena_Node *node = tctx->free_arenas;
    if (node  != 0)
        sll_stack_pop(tctx->free_arenas);
    else
        node = push_array_zero(&tctx->node_arena, Arena_Node, 1);
    node->arena = make_arena(tctx->allocator, chunk_size, align);
    return(&node->arena);
}

function Arena *reserve_arena(Thread_Context *tctx, u64 chunk_size)
{
    return reserve_arena(tctx, chunk_size, 8);
}

function Arena *reserve_arena(Thread_Context *tctx)
{
    return reserve_arena(tctx, KB(64), 8);
}

function Arena *reserve_arena(Application_Links *app)
{
    Thread_Context *tctx = get_thread_context(app);
    return reserve_arena(tctx);
}

function void release_arena(Thread_Context *tctx, Arena *arena)
{
    Arena_Node *node = CastFromMember(Arena_Node, arena, arena);
    linalloc_clear(arena);
    sll_stack_push(tctx->free_arenas, node);
}

function void release_arena(Application_Links *app, Arena *arena)
{
    Thread_Context *tctx = get_thread_context(app);
    release_arena(tctx, arena);
}
