# Solution: exercise 1 — Where does render go?

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 046 — the sprite moves](../../lessons/part-2/lesson-046-movable-sprite.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

The prediction first, from the numbers the lesson already printed. Render
averaged 0.435 ms; the sprite draw 0.001 ms. The only other thing the
render phase contains is `ClearBuffer` — and it touches 640 × 480 =
307,200 pixels against the sprite's 256. So the prediction is stark: the
clear takes *essentially all* of render, and the named phases will split
roughly 400 : 1. A renderer that draws one small sprite per frame is
mostly a background painter.

The patch gives `clear` its own field in `FrameRecord`, sums it in the
account, and times it exactly like `sprites` — two clock reads around one
call. The run confirms the prediction and refines it:

```
frame 1: update 0.000 ms, render 0.368 ms (sprites 0.001, clear 0.367), present 0.713 ms, total 1.081 ms
frame 2: update 0.000 ms, render 0.424 ms (sprites 0.001, clear 0.422), present 0.289 ms, total 0.713 ms
frame 3: update 0.000 ms, render 0.385 ms (sprites 0.001, clear 0.383), present 0.328 ms, total 0.713 ms
engine: 4 frames — avg 0.834 ms (update 0.000, render 0.401 incl. sprites 0.001, clear 0.399, present 0.433)
```

- **The clear is the render.** 0.399 of render's 0.401 ms — 99.5% — is
  `ClearBuffer`. It is a copy like any other: 307,200 pixels × 4 bytes =
  1,228,800 bytes of background written every frame, whether or not
  anything is drawn on top of it.
- **The sprite is free at this size.** 256 pixels through the blitter,
  one microsecond — below the frame's noise floor. The interesting
  question is not its cost but its *scaling*: a second sprite adds
  roughly the same microsecond again; a full-screen of sprites would not
  stay free. The named field is what lets that question be answered with
  numbers instead of vibes.
- **The phases do not tile perfectly**, and that is measurement, not
  error. `render − (sprites + clear)` is the frame's own bookkeeping
  inside the phase: the two `platform::Now()` reads around each sub-phase,
  the loop's setup. A few microseconds of honest overhead — which is why
  the rule from the lesson holds: named times live *inside* their phase,
  and the phase's number stays the measured truth.

The second-sprite question from the prompt: adding a second `BlitSprite`
call moves `sprites` and leaves `clear` untouched — the attribution would
show it immediately, where the flat `render` number would have swallowed
it. That is the whole argument for the frame-budget table's per-subsystem
rows, playing out at microsecond scale three lessons before the table
exists.
