// arena.h — the arena: a bump allocator over a reservation.
//
// Lesson 041: allocation is a pointer that moves forward (and comes back
// on demand). No free lists, no bookkeeping per allocation — the arena is
// one reservation and a cursor in it. Memory comes from the OS reservation
// of lesson 040; the allocator is ours.
#ifndef ARENA_H
#define ARENA_H

#include <stddef.h>

#include "platform.h"

namespace engine {

struct Arena {
    platform::Reservation memory; /* the whole reserve, from the OS */
    size_t used;                  /* the bump pointer: bytes handed out */
};

/* An arena over an OS reservation of at least this many bytes. */
void ArenaInit(Arena &arena, size_t bytes);

/* Gives the reservation back to the OS. */
void ArenaRelease(Arena &arena);

/* Allocates from the bump pointer, aligned to align (a power of two).
   Returns 0 when the arena has no room — the honest answer, never a
   smaller allocation. */
void *ArenaAlloc(Arena &arena, size_t bytes, size_t align);

/* A rollback mark: where the bump pointer is now. */
size_t ArenaMark(const Arena &arena);

/* Everything allocated since the mark is gone — and the next allocation
   lands where the marked one did. */
void ArenaRollback(Arena &arena, size_t mark);

} /* namespace engine */

#endif
