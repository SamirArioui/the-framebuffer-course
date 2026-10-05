# Solution: exercise 2 — Round trip

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 017 — writing a real image file by hand](../../lessons/part-0/lesson-017-image-file.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

`CheckBmp` treats the file exactly the way a stranger's decoder would:
seek, decode fields with the lesson-014 readers, sample pixels through
the documented layout. The run reports agreement on every count:

```
check: size field 198, width 8, height 6, bpp 24
check: file (7,5) = 255 255 0, buffer = 255 255 0
```

The pixel seek is the interesting half: `(w - 1, h - 1)` is the last
pixel of the *first* row on disk, so its file offset is `54 + (w - 1) *
3` — no row arithmetic needed because bottom-up order puts the image's
bottom row first. The three bytes come back blue-green-red and are
un-swapped before comparing with `GetPixel`, which returns RGB — the
round trip crosses the format's byte order twice and lands where it
started. Point the buffer comparison at `(w - 1, 0)` instead and the two
lines stop matching — the file still says `255 255 0` (yellow) while the
buffer answers `0 255 255` (the cyan at `(7, 0)`) — which is the
mismatch detector doing its job. A writer that verifies its own output
catches format bugs on the same run they are introduced.
