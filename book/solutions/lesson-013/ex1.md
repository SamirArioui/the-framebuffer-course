# Solution: exercise 1 — Three pixels, by hand

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 013 — raw bytes and pixel formats](../../lessons/part-0/lesson-013-raw-bytes.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

The new call lands at column 2 of row 0, so its offset is
`0 * (8 * 3) + 2 * 3 = 6`: three bytes starting at position 6 of the row.
The predicted first twelve bytes are therefore

```
FF 00 00 00 FF 00 11 22 33 00 00 00
```

— red, green, the new `11 22 33`, then three untouched black pixels — and
the run prints exactly that. Note what did *not* move: the pixels already
in the buffer keep their bytes, because `PutPixel` writes exactly three of
them and nothing else. If your prediction was wrong, check whether you
counted the new pixel at `2 * 3 = 6` or at `2 * 1 = 2`; the latter is the
forgotten-stride bug of exercise 3, and it is the single most common way
this arithmetic goes wrong. Counting bytes by hand once, slowly, is worth
more than three intuitive guesses.
