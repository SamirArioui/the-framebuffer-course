// arena.cpp — the arena: the bump pointer, the alignment, the marks.
//
// Lesson 041: allocation this simple is three pieces of arithmetic and one
// rule — the arena never hands out memory it does not have.

#include "arena.h"

namespace engine {

void ArenaInit(Arena &arena, size_t bytes)
{
    arena.memory = platform::ReserveMemory(bytes);
    arena.used = 0;
}

void ArenaRelease(Arena &arena)
{
    platform::ReleaseMemory(arena.memory);
    arena.used = 0;
}

void *ArenaAlloc(Arena &arena, size_t bytes, size_t align)
{
    if (!arena.memory.bytes)
        return 0;

    /* The bump pointer, rounded up to the alignment (a power of two, so
       the mask does what lesson 007's padding did by hand). */
    size_t base = (size_t)(arena.memory.bytes + arena.used);
    size_t aligned = (base + align - 1) & ~(align - 1);
    size_t start = aligned - (size_t)arena.memory.bytes;

    if (start + bytes > arena.memory.size)
        return 0; /* out of room: the honest answer */

    arena.used = start + bytes;
    return arena.memory.bytes + start;
}

size_t ArenaMark(const Arena &arena)
{
    return arena.used;
}

void ArenaRollback(Arena &arena, size_t mark)
{
    if (mark <= arena.used)
        arena.used = mark; /* nothing is freed; the cursor just moves back */
}

} /* namespace engine */
