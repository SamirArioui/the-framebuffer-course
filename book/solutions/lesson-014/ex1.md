# Solution: exercise 1 — Width 258

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 014 — endianness and image-header layout](../../lessons/part-0/lesson-014-image-headers.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

Width 258 is `0x00000102`, so its little-endian bytes are `02 01 00 00`;
height 4 is `04 00 00 00`. The dump's second line (header offsets 16–31)
comes out

```
00 00 02 01 00 00 04 00 00 00 01 00 18 00 00 00
```

and the decoded printout reports `width 258`, `height 4` — the encoder and
decoder round-trip. 258 is the interesting value because its four bytes are
*not* all-but-one zero: `02 01 00 00` shows the byte order working, whereas
width 8 (`08 00 00 00`) cannot distinguish "little-endian" from "the
number happens to be small". Also note the file-size field grew to 3158:
the row is now 774 bytes, which needs 2 padding bytes to reach the
4-byte boundary, giving 776 bytes per row — the first concrete sighting of
the padding lesson 017 lives on.
