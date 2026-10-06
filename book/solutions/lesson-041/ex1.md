# Solution: exercise 1 — The marker's trail

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 041 — arenas](../../lessons/part-1/lesson-041-arenas.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

The trail is eight positions — the marker's last eight homes — and the
natural place to keep them is the lesson's own allocator. The arena is
cleared of the startup experiment with one `ArenaRollback(arena, 0)`, and
the trail takes its slice:

```
engine: trail: 8 slots at offset 0 (arena used 64)
engine: marker at 548,228 (t=1.002)
engine: marker at 554,228 (t=1.053)
engine: marker at 560,228 (t=1.103)
engine: marker at 566,228 (t=1.154)
engine: trail used 64 bytes of its arena mark 0
```

Eight `TrailPos` of two ints each: 64 bytes at offset 0 — note the
rollback at work in the allocator's own report: the trail landed where the
startup experiment's first allocation had been, because the experiment's
memory went back to the bump pointer and the arena never forgets that it
was all one reservation.

Each move writes the next slot and wraps (`trail_at % 8`) — a ring of
eight, and the renderer draws the older slots dimmer and smaller behind the
marker. The trail is drawn *before* the marker each frame, so the marker
always paints over its own history — lesson 025's draw-order rule.

One design note for the write-up: the ring wrap is *not* a rollback — the
history keeps all eight slots live at once. Rollback is for "everything
since this point is done with" (a frame's scratch, a level's data); a ring
is for "reuse the oldest slot". Knowing which shape you have is the whole
art of arena use.
