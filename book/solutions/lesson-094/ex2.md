# Solution: exercise 2 — why the HUD never scrolls

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 094 — the HUD](../../lessons/part-5/lesson-094-hud.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

**The argument.** The draw has two coordinate spaces and one rule
each: the scene draws at `−camera` (it *is* the world, seen through a
window that moves), and the HUD draws at screen coordinates (it *is*
the window's own furniture). If `HudDraw` applied the camera's offsets
the way `GameDrawMap` does, the readouts would ride the world: at the
map's full scroll (`camera 128`, the frame being 640 wide) the `SCORE`
line — laid out at `x = 8` — would draw at `8 − 128 = −120`. Its
64-pixel text would span `−120 … −56`: **entirely off the frame**. The
blitter would clip every glyph away and the score would simply not be
on screen at the exact moment the player is deepest in the map. On the
way there it would slide sideways under the reader's eye, and the
right-hand column would drift inward from `568` to `440`.

That is not merely ugly — it is wrong about what a HUD *is*. A HUD is
not world content; it is the player's instrument panel. Instruments are
bolted to the cockpit, not painted on the terrain. The rule (lesson
054's) is about **ownership**: anything the game *has* — the hero, the
enemies, the map — lives in world coordinates and scrolls; anything the
player *reads* lives in screen coordinates and does not. The moment a
readout scrolls with the world it stops being a readout and becomes
decoration.

**The measurement.** The probe prints the columns' real draw
coordinates beside the camera — and beside the counterfactual, what
the camera *would* make them:

```
engine: probe: hud columns at 8,8 and 568,8 — camera 8,0 would make them 0,8 and 560,8
engine: probe: hud columns at 8,8 and 568,8 — camera 13,0 would make them -5,8 and 555,8
engine: probe: hud columns at 8,8 and 568,8 — camera 20,0 would make them -12,8 and 548,8
engine: probe: hud columns at 8,8 and 568,8 — camera 47,0 would make them -39,8 and 521,8
…
engine: probe: hud columns at 8,8 and 568,8 — camera 128,0 would make them -120,8 and 440,8
```

The left half never moves — `8,8 and 568,8` through the camera's whole
range. The right half is the counterfactual, and it lands exactly where
the argument said: `−120` at full scroll, the readout's pixels past the
frame's left edge. The run's own `hud:` lines agree (`at 8,8 over
camera 8,0 … 128,0`), and the lesson's run walked the map end to end to
show it.

One nuance worth naming: the *right* column's x is computed
(`FRAME_WIDTH − margin − TextWidth(line)`) — it depends on the text,
not on the world. Right-alignment moving as the digits change is
layout; a readout moving because the *camera* moved is the bug. The
rule is about where the number comes from, not about whether x is a
constant.
