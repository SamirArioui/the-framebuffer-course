# Solution: exercise 1 — Your own key

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 045 — the clipped, transparent blit](../../lessons/part-2/lesson-045-blit.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

The patch moves the key decision from the loader's convention table into
the pixels: `LoadSprite` reads the copy's first three bytes — the image's
top-left pixel — and those become `key_r, key_g, key_b`. One line of
inspection shows what the sprite is now claiming:

```
engine: pixel bytes sum to 125580
engine: key color from the sprite's corner: 255,0,255
engine: blit check: 130 opaque pixels drawn unchanged, 0 mismatches
engine: blit check: 126 key pixels wrote nothing over the background
engine: blit check: clip at -4,-4 landed 144 pixels, 0 wrong, 0 touched outside
```

The course sprite keeps the same key it had — its corner *is* magenta —
and all three checks pass unchanged. The rule moved; the behavior did not.

Now the second half. A sprite whose corner is lime `(0, 255, 0)` and whose
art is *magenta* — the exact inverse of the course convention — loaded
through the same code:

```
engine: pixel 0,0 = 0,255,0
engine: pixel 8,8 = 255,0,255
engine: pixel bytes sum to 111180
engine: key color from the sprite's corner: 0,255,0
engine: blit check: 180 opaque pixels drawn unchanged, 0 mismatches
engine: blit check: 76 key pixels wrote nothing over the background
engine: blit check: clip at -4,-4 landed 144 pixels, 0 wrong, 0 touched outside
```

The key is lime, so the 76 lime pixels write nothing and the **180 magenta
pixels are drawn as art** — with the file and the framebuffer agreeing at
every one. The blit never knew a rule changed; it just compares against
the sprite's own three key bytes.

Now the cost. Every transparency convention sacrifices one color, and this
one spends the image's **top-left pixel**: whatever color lives at
`(0, 0)` can never appear in the drawing — it is declared to be nothing.
An image that needs its corner color as art must move the art (the corner
is the "key sample", like a painter's palette note), pick a different
convention, or grow a format that can *say* which color is the key —
a field PPM does not have. That is why formats like PNG carry an alpha
channel instead of a key color: the "which pixel is nothing" question is
answered per pixel, not per image. Our format is hand-sized on purpose, so
the convention is ours to choose — and to pay for.
