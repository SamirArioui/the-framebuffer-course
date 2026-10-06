# Solution: exercise 2 — Cover and reveal

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 031 — presentation through the platform layer](../../lessons/part-1/lesson-031-present.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

An X11 window is **not retained**: the server keeps what is currently
visible, but when another window covers yours, the covered pixels are gone.
When your window is revealed, the server does not reconstruct them — it asks
you to redraw, and the ask is an `Expose` event. (Back in lesson 022 the
character grid had a `front` buffer for exactly this problem; here the
framebuffer itself is the front buffer.)

The prediction to make first: with a single present at startup — the
lesson-030 shape — a covered-and-revealed window shows whatever the server
happened to leave behind: blank, garbage, or another window's remains. The
pixels the engine wrote are gone until something writes them again.

With the instrumenting print applied, the repair is visible as it happens.
The same window, unmapped and mapped again (which is what damage looks like
to a client):

```
platform: expose — the window needs its pixels
platform: expose — the window needs its pixels
engine: framebuffer 640x480, 1228800 bytes, stride 2560
engine: pixel (0,0) = 255 0 0
engine: pixel (639,479) = 0 255 0
engine: pixel (60,101) = 32 32 64
engine: presented
engine: close reported
engine: closed
```

Each `Expose` woke the engine's loop, and the loop's answer was not
"handle the expose" — it was simply to present again. That is the whole
repair mechanism: the engine never fixes regions, never tracks damage, never
decides what to redraw. It owns one framebuffer and re-presents it. The
event only serves to wake the loop; the pixels do the rest.

The proof that the repair worked is exercise 1's checker run after the
reveal: the window's pixels still match the framebuffer, byte for byte.
Cheap in machinery, honest in cost — every re-present is a full copy, and
lesson 036 is where that cost gets measured instead of assumed.
