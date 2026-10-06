# Solution: exercise 1 — FillRect, back from Part 0

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 030 — the framebuffer as our own bytes](../../lessons/part-1/lesson-030-framebuffer.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

`FillRect` is paint's, with the fold lesson 015 taught moved where it
belongs: the clip happens in *rectangle space* — one clamp of the rectangle
against the buffer's edges — and the inner two loops then walk only pixels
that exist. One check replaces one check per pixel, and the walk is exactly
the clipped rectangle.

The three rectangles exercise all three clip cases:

```
$ DISPLAY=:99 ./build/game
engine: framebuffer 640x480, 1228800 bytes, stride 2560
engine: pixel (0,0) = 255 0 0
engine: pixel (639,479) = 200 0 200
engine: pixel (60,101) = 32 32 64
engine: pixel (150,150) = 200 120 40
engine: pixel (20,410) = 0 200 200
engine: pixel (630,470) = 200 0 200
engine: presented
```

`(150,150)` sits inside the plain 200×100 block at (100,100). `(20,410)`
sits inside the left-clipped rectangle: it asked to start at x=-20, and the
fold brought its first column to x=0 — the pixels that exist were painted,
the ones that never existed were not. `(630,470)` shows the corner rectangle,
which asked for 100×100 at (600,440) and got 40×40.

One thing the corner rectangle reveals: `(639,479)` is no longer green. The
clipped rectangle's surviving corner covers it — rectangles paint in the
order they are called, later ones over earlier ones, the same painter's
order Part 0's `paint` and lesson 025's view table used. Nothing here clips
*against what is already drawn*; that is compositing, and it is Part 2's
business.
