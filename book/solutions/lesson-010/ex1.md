# Solution: exercise 1 — The same bytes, differently

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 010 — void*: genericity and its pain](../../lessons/part-0/lesson-010-void-pointer.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

The prediction on any little-endian machine — which is what this run shows:

```
7 0
42 0
```

Each element slot is eight bytes holding a `long`: 7 is
`07 00 00 00 00 00 00 00`. Reading the slot through `const int *` does not
fetch anything new — it reinterprets the *same* bytes as two four-byte
integers, and on a little-endian machine the low half comes first: `7`, then
the all-zero high half, `0`. Hence `7 0`, and `42 0` for the same reason.

On a big-endian machine the bytes of 7 are `00 00 00 00 00 00 00 07`, so the
prediction flips to `0 7` and `0 42` — same program, same values, different
answer, and `void *` has no opinion about it. Nothing here touched an
invalid address and no sanitizer will complain: the bytes were in bounds, the
cast was legal, and the interpretation is entirely the caller's business.
That is exactly the power and the danger of the generic array — the bytes
behave perfectly and the type means nothing, and lesson 006 taught where
that road ends.
