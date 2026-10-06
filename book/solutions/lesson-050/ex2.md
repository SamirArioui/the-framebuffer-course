# Solution: exercise 2 — The bytes the sheet never heard of

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 050 — the bitmap font as an asset](../../lessons/part-2/lesson-050-font.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

The prediction, from `FontGlyph`'s index math:
`index = (int)(unsigned char)c - 32`, then a bounds check against
`FONT_COUNT = 96`. So the answer is arithmetic:

| Probe | Byte | Index | Answer |
| ----- | ---- | ----- | ------ |
| `A` | 65 | 33 | **found** — cell 33, row 2 of the sheet |
| newline | 10 | −22 | **missing** — below the range |
| byte `0` | 0 | −32 | **missing** — below the range |
| byte `200` | 200 | 168 | **missing** — past 96 |
| byte `195` | 195 | 163 | **missing** — past 96 |

The run confirms all five:

```
engine: glyph for byte  65: found
engine: glyph for byte  10: missing
engine: glyph for byte   0: missing
engine: glyph for byte 200: missing
engine: glyph for byte 195: missing
```

Now the case the prompt asks for — where a signed `char` computes a
different index than the byte deserves. On this platform `char` is
signed, so byte `200` *as a `char`* is −56. Without the `(unsigned
char)` cast, the index would be `−56 − 32 = −88`; with it, `200 − 32 =
168`. Both are outside the range today, so both return `0` — the bug is
invisible *because the sheet is small*. Grow the sheet past 128
characters (a font with accented letters, cells 0..191, and the bounds
check against 192) and the uncast version starts returning `missing`
for perfectly valid glyphs in the top third of the range, while the cast
version finds them. The cast is not defensive style; it is the index
math knowing what a byte is. Part 0's habit, still paying.

And the label probe:

```
engine: label of 6 bytes: 4 drew glyphs, 2 skipped
```

`"SCÖRE"` looks like five characters to a human and is **six bytes** on
disk: `Ö` is two UTF-8 bytes (`0xC3 0x96`), and the engine reads bytes.
The loop draws four glyphs (`S`, `C`, `R`, `E`) and skips both bytes of
the `Ö` — each independently missing from the sheet. This is the
honest state of a byte-oriented engine with a 96-glyph sheet: it does
not know what a code point is, and it says so by drawing nothing.

Lesson 051 turns that "draws nothing" into a *rule*: a missing character
draws nothing **and the characters after it keep their positions** —
no hole, no shift, no crash. The skip counter in this patch is that rule
before it had a name: `drew`/`skipped` is exactly the accounting the
layout loop will need, and the UTF-8 probe is exactly the input that
will test it.
