# Solution: exercise 2 — the worst frame, predicted

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 101 — pass 3: the frame-budget report](../../lessons/part-5/lesson-101-frame-budget.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

**The prediction, written down first.** A played run's worst frame is
a *busy* frame, and the play frame's budget is dominated by the two
rows the game spends everywhere: the map's draw and the clear, with
the seam's wait riding along whatever else happens. So the prediction:
the worst frame is a play frame whose `render` takes the lion's share
— the tilemap above all — with `present` roughly doubled against its
average (the seam spikes when it spikes), and the update visibly
fatter than its `0.013 ms` self (a wave's spawn frame creates its
roster in one frame). The alternative hypothesis — the worst frame
*is* the seam's — is the interesting one to hold in reserve.

**The account remembers the shape.** The worst frame's whole record is
kept beside its cost (`stats.worst_record`), and the report gives it
the same attribution the table gives the average. From the exercise's
own ten-leg run of the finished game (2,583 frames):

```
engine: frame budget — 2583 frames, avg 1.239 ms, worst 2.821 ms (frame 519)
engine:   budget      60 fps is 16.667 ms a frame — 0 of 2583 frames over it, worst 2.821 ms (17% of it)
engine:   worst was   2.821 ms (frame 519): update 0.110 (entities 0.092), audio 0.000, render 1.852 (clear 0.575, sprites 0.010, text 0.014, tilemap 1.253), present 0.858
```

**The verdict against the prediction:** right in the shape, better
than expected in the detail. `render 1.852` is the lion's share with
`tilemap 1.253` on top — and `clear 0.575` at double *its* average
too (a cold buffer on that frame). `present 0.858` is indeed about
double its `0.438 ms` average — the seam spikes when it spikes. And
`update 0.110` with `entities 0.092` is seven times the update's
average — a wave's spawn frame, creating its roster in one frame, the
prediction's tell. The frame is 2.821 ms — 17% of the budget — and
the tail is *the game being busy*, not any one subsystem misbehaving.

**The counterfactual.** If the worst frame were the seam's — `present`
dominant, the engine's own rows at their averages — the honest
conclusion would be that the worst case is owned by the platform
layer: the frozen menu cannot shorten it (the seam's copy is the
second OS's file, on the future-work ledger), and the fix would have
to arrive as a double-buffered or MIT-SHM present before the engine's
own numbers mattered again. In this run it is the reverse: the worst
frame's cost is spread across the engine's own draw work, which means
it is ordinary load — the kind that scales down with exactly the
levers this menu already spent. That is the difference between "our
tail" and "our machine's tail", and the worst-frame row is what tells
them apart.
