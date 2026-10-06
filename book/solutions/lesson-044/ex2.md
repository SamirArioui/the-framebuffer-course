# Solution: exercise 2 — Sprite, meet window

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 044 — a sprite as loaded bytes](../../lessons/part-2/lesson-044-sprite-bytes.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

The patch adds `DrawSpriteRaw` — the three-line copy loop that is exactly
what lesson 045 turns into the blitter: every source pixel becomes one
`PutPixel`, key color included. It draws the sprite at `(32,32)` once at
startup for the check, and then again in the render phase after
`ClearBuffer`, so the sprite is on screen beside the marker.

The check reads three pixels back and prints the file's bytes against the
framebuffer's at the same points:

```
engine: sprite pixel 0,0: file 255,0,255 framebuffer 255,0,255 (the key color drew itself)
engine: sprite pixel 8,8: file 220,40,40 framebuffer 220,40,40
engine: sprite pixel 0,1: file 255,0,255 framebuffer 255,0,255 (the key color drew itself)
```

File and framebuffer agree at every point — the copy is faithful. The
window agrees too; this is the same readback from the OS side:

```
$ DISPLAY=:99 ./winread "the framebuffer engine" 32 32 40 40 32 33 100 100
winread: window 4194305 is 640x480
winread: 32,32 -> r=255 g=0 b=255
winread: 40,40 -> r=220 g=40 b=40
winread: 32,33 -> r=255 g=0 b=255
winread: 100,100 -> r=32 g=32 b=64
```

The interesting line is the one with the parentheses. Pixel `(0,0)` and
`(0,1)` are key-colored — the magenta the artist meant as *nothing* — and
the loop drew them as magenta anyway. The file said "this pixel is
transparent" in the only language a PPM has, which is the pixel's own
color, and the raw loop has no idea such a rule exists. So the sprite's
background is a magenta square on the screen, drawn *over* whatever was
there.

That is precisely what lesson 045 owes: a copy loop that consults the key
color and writes nothing for it — transparency — and that drops pixels
whose destination lands outside the framebuffer instead of letting
`PutPixel` silently clip them one at a time — clipping as one decision,
per row, where the copy can see it. Both changes happen inside the loop
you just wrote. Nothing else about it survives: the loop is the blitter,
and the blitter is the one piece of code the next five lessons measure,
read, and vectorize.
