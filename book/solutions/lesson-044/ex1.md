# Solution: exercise 1 — The header that lies

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 044 — a sprite as loaded bytes](../../lessons/part-2/lesson-044-sprite-bytes.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

First, the prediction. The corrupted copy's header reads `P6 / 32 32 /
255` — every field still *parses*: the magic is right, `32` and `32` are
perfectly good numbers, `255` is the maxval the loader wants. Nothing in
the header looks like a lie. The lie is only visible when the header's
claim meets the file's length: `32 × 32 × 3 = 3072` pixel bytes claimed,
768 present. So the check that refuses is **the pixel count**, and the
report should say so.

The diff splits the single `SPRITE_MALFORMED` into the two lies the parser
can actually tell apart — `SPRITE_BAD_HEADER` (the fields do not form a P6
header) and `SPRITE_TRUNCATED` (the header is fine; the pixels are not all
there) — and gives `Run` its arguments back so the loader can be aimed at
any file. Then the three lies, each naming itself:

```
$ ./build/game lying-32.ppm
engine: lying-32.ppm: the pixel bytes do not fill the header's claim
$ ./build/game bad-magic.ppm
engine: bad-magic.ppm: the header fields are not a P6 header
$ ./build/game no-such.ppm
engine: no-such.ppm: missing or unreadable
```

`lying-32.ppm` is the prediction's confirmation: the header fields passed,
the count refused. (`bad-magic.ppm` starts `P5` — the *text* PPM variant,
whose pixels are decimal numbers; one byte of difference in the magic, a
completely different format.) The default run still loads
`assets/sprite.ppm` and prints the same four inspection lines as the
lesson.

Now the question the bytes ask. If the loader had trusted the header, it
would have believed in 3072 pixel bytes and copied that many out of a file
buffer that holds 768 of them. The extra 2304 bytes are not zeros and not
an error — they are **whatever the OS's file buffer happened to hold past
the end of the file**. The sprite would have been drawn with 2304 bytes of
unrelated memory as its bottom rows: garbage that changes between runs,
between machines, between file sizes.

And it gets worse than garbage. The file buffer's bytes go back to the OS
with `ReleaseFile` the moment the copy is done — that is the ownership
rule of lesson 044's prose. A loader that reported "success" while its
pixels reached past the file would be a sprite whose bytes include memory
the engine already gave back: lesson 041's ASan-blind territory, wearing a
16×16 costume. The count check is not fastidiousness; it is the line
between a typed failure and a debugging session that ends in a chapter
about undefined behavior.

That is the habit the exercise is after: *the header is a claim, the file
is the evidence, and the load only proceeds when they agree.*
