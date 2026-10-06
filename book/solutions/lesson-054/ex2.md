# Solution: exercise 2 — The additive that cancels

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 054 — the camera](../../lessons/part-2/lesson-054-camera.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

The prediction: the draws only ever see `base + additive`, so base
`(100, 50)` with additive `(−100, −50)` sums to `(0, 0)` — the scene
must look exactly like the view with no camera at all. The base's value
is invisible from the framebuffer; the additive erased it. The check
this resembles is lesson 045's camera check ("the same pixels, moved"),
run in reverse — instead of moving the scene to a new offset, the
offsets move the scene back to where it started.

The patch adds the case — draw the origin view and snapshot it, then
draw the cancelling camera and compare — and the run confirms:

```
engine: camera check: additive (-100,-50) cancels the base — 307200 pixels compared, 0 mismatches
```

All 307,200 pixels of the frame, identical to the origin view. The sum
is the whole contract: there is no stage at which the base is "more
real" than the additive.

Which is the warning in the prompt's last question. If the juice
toolkit can cancel the base, then it can *replace* the view — a shake
large enough, or one with a wandering bias, effectively seizes the
camera from the game. What the toolkit must promise, and what the
separation of fields encodes, is:

1. **The additive is transient.** It returns to exactly zero — always,
   no exceptions. The base is then the view again, byte-for-byte (the
   lesson's third scenario is that promise, checked).
2. **The additive is bounded.** A shake of ±6 is feedback; a shake of
   ±300 is a different view. Effect amplitudes are chosen against the
   screen, not left unbounded.
3. **The additive never writes the base.** Not "usually doesn't" — the
   base is the game's statement about where the player is looking, and
   any effect that needs to *change* the view is not an effect, it is
   camera control (the cutscene's business, or Part 5's capstone).

The engine cannot enforce these — they are contracts, like the ones in
lesson 042's audit, held by the code that writes the field. But the
*structure* makes them thinkable: one struct, two named fields, one
summation function, and checks that watch what each field does to the
pixels. That is what the O2 obligation was really asking for.
