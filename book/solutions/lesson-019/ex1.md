# Solution: exercise 1 — The life of `strtoul`

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 019 — the game loop](../../lessons/part-0/lesson-019-game-loop.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

With the instrumenting line in place, the five runs report:

```
./snek 3    → parsed=3 end=''           then three frame lines
./snek 0    → parsed=0 end=''           then the error message
./snek abc  → parsed=0 end='abc'        then the error message
./snek 12x  → parsed=12 end='x'         then the error message
./snek      → usage line (the parser is never reached)
```

`strtoul` converts the longest leading run of digits and sets `end` to the
first character it did not consume. For `3` and `0` the whole string is
consumed, so `end` points at the terminator and `*end` is `'\0'` — the test
passes and the difference in outcome is entirely the separate `max_frames ==
0` check. For `abc` no digits are consumed at all: `0` comes back and `end`
still points at the whole string. For `12x` the conversion succeeds with
`12`, and `end` pointing at the `x` is the only evidence of the garbage —
exactly what `*end != '\0'` catches and `atoi` would have missed. The
no-argument run never reaches the parser: the `argc` guard in `main` exits
first. Predicting that one correctly is mostly about remembering the guard's
position in `main`.
