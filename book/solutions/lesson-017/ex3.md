# Solution: exercise 3 — The flipped image

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 017 — writing a real image file by hand](../../lessons/part-0/lesson-017-image-file.md).*

## The diff

```diff
{{#include ex3.patch}}
```

## Walkthrough

Flipping the loop makes the writer store image row 0 first. `file` does
not notice — `PC bitmap ... 8 x 6 x 24` — because every *header* field is
still right. The pixels betray it: decoded with the standard bottom-up
rule, the sampler shows the scene upside down —

```
pixel (1,0): (32, 32, 32) pixel (7,5): (0, 255, 255)
pixel (1,5): (0, 255, 0) pixel (7,0): (255, 255, 0)
```

— the green pixel has moved to the bottom row and the yellow end of the
diagonal to the top-right. The header is lying in exactly one bit: a
positive `biHeight` *means* rows stored bottom-up, and ours now are not.
A legitimate top-down BMP stores the height field as a *negative* number
(its two's-complement bytes through the same `PutU32LE`), which readers
interpret as "rows in top-down order". The quirk survives because the
format predates anyone asking the question — Windows wrote scanlines
bottom-up into its framebuffers, so the file did too, and forty years of
readers now assume it. Format rules are part of the data: right bytes in
the wrong order are the wrong bytes. Restore the loop afterwards.
