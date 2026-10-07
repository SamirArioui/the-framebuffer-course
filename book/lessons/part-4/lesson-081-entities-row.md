# Lesson 081 — the slice's cost in the frame budget

{{#include ../../stability-horizon.md}}

## Prose

The slice runs, and the part has one debt left to pay: the cost of
running it. Lesson 070 paid the same debt for the mixer — the audio
row, measured rather than guessed — and this lesson is its twin. The
slice's per-entity work is real work in a real phase, and the frame
budget says nothing about it yet: the update's row is one number for
intent, walking, and the camera together. So the idea here is the
attribution move lesson 046 made for the render, made for the update:
**the update phase names its entity work, and the budget grows a row
measured from the frames that actually ran.**

### The row is inside the phase

`FrameRecord` grows one named time, and the discipline is the one that
has held since lesson 046: **named times live inside a phase, never
instead of it.**

```cpp
    double entities; /* the walk's per-entity step — the store's
                        entities, moved through the mover */
```

`update` is still the phase — the whole of the input read, the walk, the
score, the camera follow, measured from its start to its end. `entities`
says where inside it the walk's time went. The budget table prints the
row the same way it prints `sprites`, `text`, and `tilemap` under
`render`: indented, sharing, and not double-counted.

The measurement itself is two lines and no thought: `platform::Now()`
around the walk, exactly like every phase since lesson 036. There is no
model of what the walk costs, no per-entity constant multiplied by a
count — the number is what the frames took. That is the whole reason
this part has a frame record at all.

### Measured, not guessed

The claim "the numbers are real measurements" is checkable the way
lesson 070 checked its row: **the average of the frame log's column
reproduces the budget table's row.** From a real run of this lesson's
end state — the slice, driven by scripted input, 121 frames:

```
engine: frame budget — 121 frames, avg 1.830 ms, worst 2.492 ms (frame 58)
engine:   subsystem   avg ms    share
engine:   update       0.012       1%
engine:     entities   0.001       0%
engine:   audio        0.000       0%
engine:   render       1.347      74%
engine:     sprites    0.002       0%
engine:     text       0.008       0%
engine:     tilemap    0.933      51%
engine:   present      0.472      26%
engine:   total        1.830     100%
```

and the same run's 121 `frame N:` lines, averaged:

```
log averages:  update 0.012  entities 0.001  total 1.830 ms
```

Row for row, digit for digit: `update 0.012`, `entities 0.001`,
`total 1.830` — the table is the log's arithmetic, and the log is the
frames. Nothing in that table was estimated.

### What the row says, and what it does not

The numbers above are the slice's honest price: two entities, walked
once each per frame, cost a thousandth of a millisecond — 0.001 ms of a
1.830 ms frame. The scene's cost is where it always was: the tilemap's
walk (0.933 ms) and the presentation (0.472 ms). A game with two
entities is not an entity game yet, and the row exists so that the day
it is — Part 5's screens with dozens of enemies, projectiles, and
bursts — the number moves in public.

What the row does *not* say is what the rest of `update` is. The
0.012 ms of update minus the 0.001 ms of entities is the input read, the
hero's intent, the score, the camera's clamp, and the reports — the
game's own frame work, which is deliberately *not* attributed to
entities, because none of it is per-entity. That boundary is the row's
definition: **the walk is the entity work**; what surrounds it is the
game's. If a future lesson adds per-entity behavior (Part 5's AI), it
goes inside the walk and inside this row — which is exactly the
visibility the row is for.

### What this run verified, and what it did not

- **The reported numbers are real measurements of the slice's frames.**
  The table-vs-log reconciliation above: 121 frames, the `entities`
  column averaging to the row's `0.001 ms`, `update` to `0.012 ms`,
  `total` to `1.830 ms` — and the frame's own arithmetic still closes
  (`update + audio + render + present = total`).
- **The attribution is inside the phase.** The rows sum to the total
  exactly as before the row appeared — `entities` shares `update`'s
  time, it does not add to it.
- **The row is per-entity work only.** The walk's timing brackets the
  loop and nothing else — no input, no camera, no reports.

What this lesson does **not** do is predict anything about Part 5's
costs. The row is an instrument; when the game holds fifty entities, the
row will say what fifty entities cost, on the machine they cost it on.
That is the discipline this course calls "the toolchain is curriculum",
and it is the note Part 4 ends on.

## Code step

One change for this lesson, from one number to two: `src/frame.h` grows
`FrameRecord.entities` and its sum in `FrameStats`; `src/frame.cpp`
prints the row under `update`, indented like the render's names;
`src/main.cpp` times the walk — `platform::Now()` around the loop — and
prints the column in the frame log beside the update's own number. The
slice itself is untouched: same game, same frames, one more named fact.
Its end state is tagged `lesson-081`.

```diff
diff --git a/src/frame.cpp b/src/frame.cpp
index 6868e33..444b1c0 100644
--- a/src/frame.cpp
+++ b/src/frame.cpp
@@ -19,6 +19,7 @@ void AccountFrame(FrameStats &stats, const FrameRecord &frame)
     stats.sprites_sum += frame.sprites;
     stats.text_sum += frame.text;
     stats.tilemap_sum += frame.tilemap;
+    stats.entities_sum += frame.entities;
     if (frame.total > stats.worst) {
         stats.worst = frame.total;
         stats.worst_number = frame.number;
@@ -49,10 +50,15 @@ void PrintFrameBudget(const FrameStats &stats)
     double sprites = stats.sprites_sum / n * 1e3;
     double text = stats.text_sum / n * 1e3;
     double tilemap = stats.tilemap_sum / n * 1e3;
+    double entities = stats.entities_sum / n * 1e3;
     double present = stats.present_sum / n * 1e3;
     std::printf("engine:   subsystem   avg ms    share\n");
     std::printf("engine:   update      %6.3f      %2.0f%%\n", update,
                 100.0 * update / (avg * 1e3));
+    /* Lesson 081: the update's entity work, named inside the phase it
+       lives in — measured from the frames that ran, like every row. */
+    std::printf("engine:     entities  %6.3f      %2.0f%%\n", entities,
+                100.0 * entities / (avg * 1e3));
     std::printf("engine:   audio       %6.3f      %2.0f%%\n", audio,
                 100.0 * audio / (avg * 1e3));
     std::printf("engine:   render      %6.3f      %2.0f%%\n", render,
diff --git a/src/frame.h b/src/frame.h
index 45d4d88..b05bf68 100644
--- a/src/frame.h
+++ b/src/frame.h
@@ -23,10 +23,14 @@ struct FrameRecord {
     /* Lesson 046: the render phase starts naming what is inside it — one
        field per subsystem, the attribution the frame-budget table
        (lesson 058) grows from. The named times are inside render, never
-       instead of it: render stays the phase, these say where it went. */
+       instead of it: render stays the phase, these say where it went.
+       Lesson 081: update grows the same kind of name — the entity work
+       the walk does, attributed inside the phase it lives in. */
     double sprites; /* sprite draws through the blit */
     double text;    /* lesson 051: text drawing — glyphs through the blit */
     double tilemap; /* lesson 053: the map's walk — tiles through the blit */
+    double entities; /* lesson 081: the walk's per-entity step — the
+                        store's entities, moved through the mover */
 
     /* Lesson 079: the game-time step this frame advanced the simulation
        by — not a duration. Every field above is wall-clock, at any
@@ -47,6 +51,7 @@ struct FrameStats {
     double sprites_sum; /* lesson 046's named sub-phase, summed like the rest */
     double text_sum;
     double tilemap_sum;
+    double entities_sum; /* lesson 081: the update's entity work, summed */
     double worst;      /* the longest frame so far */
     long worst_number; /* and which one it was */
 };
diff --git a/src/main.cpp b/src/main.cpp
index edb88b1..9bc7e46 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -446,6 +446,7 @@ int Run(void)
            going. Lesson 080: the walk is the game's now, and its work is
            the world's — no demo scaffolding, no per-kind branches. */
         int visited = 0;
+        double t_entities = platform::Now();
         for (int i = 0; i < ENTITY_CAP; ++i) {
             if (!store.slots[i].live)
                 continue;
@@ -461,6 +462,7 @@ int Run(void)
             else if (e.move_y < 0.0)
                 e.facing = 3;
         }
+        frame.entities = platform::Now() - t_entities;
         walk_visits += visited;
 
         /* The score, and the hero's own report: where the entity the
@@ -654,8 +656,9 @@ int Run(void)
            phase (lesson 060) joins in the record's own order, and
            lesson 079's step leads it: the game's advance beside the
            machine's durations. */
-        std::printf("frame %ld: step %.3f ms, update %.3f ms, audio %.3f ms, render %.3f ms (sprites %.3f, text %.3f, tilemap %.3f), present %.3f ms, total %.3f ms\n",
+        std::printf("frame %ld: step %.3f ms, update %.3f ms (entities %.3f), audio %.3f ms, render %.3f ms (sprites %.3f, text %.3f, tilemap %.3f), present %.3f ms, total %.3f ms\n",
                     frame.number, frame.step * 1e3, frame.update * 1e3,
+                    frame.entities * 1e3,
                     frame.audio * 1e3,
                     frame.render * 1e3,
                     frame.sprites * 1e3, frame.text * 1e3,
```

## Exercises

Two mixed exercises. Each ends with its solution — a diff against this
lesson's end state plus a walkthrough — after the prompt.

### Exercise 1 — The row, at two sizes *(measure-the-performance)*

Two entities cost 0.001 ms; what does the row do when the game holds
more? Add rows to `assets/entities.txt` — this is data, no code — until
the store holds 2, 20, and 200 entities (raise `ENTITY_CAP` if your
200 needs it), and measure the `entities` row at each size on your
machine. Is the walk's cost linear in the entity count? Reconcile one
of the three runs against its own frame log the way this lesson does.
Then answer: at your numbers, how many entities can the game walk
inside 0.1 ms — and is the entity walk the row that will ever matter?

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-081/ex1.md)

### Exercise 2 — What the row does not say *(explain-in-prose)*

The row's boundary is a definition: the walk is the entity work, and
what surrounds it is the game's. Defend that boundary in your own
words: what exactly sits in `update` outside `entities` in this game's
frame, and why should *those* things not be attributed to entities? Then
stress it: name a piece of per-entity work that would tempt you to
measure it outside the walk (pathfinding? animation? sound triggers?),
and say where its time belongs and why. Finally — the measurement
question — what can a row of 0.001 ms *not* tell you about the game's
costs, and what would you measure instead to learn it?

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-081/ex2.md)

---

**Part:** [Part 4 — services](../../index.md) ·
**Previous:** [Lesson 080 — the vertical slice](lesson-080-slice.md) ·
**Next:** [Part 5 — the game](../../index.md) ·
**Code tag:** [`lesson-081`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-081)
