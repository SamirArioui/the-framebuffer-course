# Solution: exercise 2 — The same pixel in packed 32-bit

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 013 — raw bytes and pixel formats](../../lessons/part-0/lesson-013-raw-bytes.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

The run prints

```
packed (1,0) word:  0x0000FF00
packed (1,0) bytes: 00 FF 00 00
rgb888 (1,0) bytes: 00 FF 00
```

The same green pixel costs four bytes packed and three bytes in RGB888 —
25% more memory for the convenience of one word per pixel. But look at the
packed bytes: the *word* is `0x0000FF00`, and in memory it reads
`00 FF 00 00` — the least significant byte first. That is little-endian
storage, and it is why the packed line would print `00 00 FF 00` on a
big-endian machine while the RGB888 line prints `00 FF 00` everywhere.
Three single-byte channels have no byte order at all; a multi-byte word
always has one. Lesson 014 is entirely about this asymmetry — and about
what image files do to avoid depending on which machine wrote them.
