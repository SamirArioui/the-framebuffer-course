# Solution: exercise 2 — The wrong way around

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 014 — endianness and image-header layout](../../lessons/part-0/lesson-014-image-headers.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

`PutU32BE` is `PutU32LE` with the shift order reversed — most significant
byte first. Encoding the width (8) with it stores `00 00 00 08` at header
offsets 18–21, and the little-endian decoder in `main` faithfully reads
those bytes back as `0x08000000`:

```
width       134217728
```

Nothing malfunctioned; encoder and decoder just disagree about which end
goes first. That number is exactly what a little-endian BMP decoder on any
machine would compute for a file whose width field was written big-endian —
this is the "width 134217728" of the prose, reproduced on purpose. The
experiment confirms the lesson's argument: the file format pins little-endian,
so the *writer* must encode little-endian regardless of the host, and one
reversed function is all it takes to emit a file that is well-formed in
every field except the one that matters.
