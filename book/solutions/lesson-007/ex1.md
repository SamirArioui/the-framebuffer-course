# Solution: exercise 1 — Padding in a fresh struct

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 007 — structs: sizeof, alignment, and padding](../../lessons/part-0/lesson-007-struct-layout.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

The prediction most people make is `sizeof(struct Triple) == 6` — one byte,
four bytes, one byte. The machine says 12:

```
== struct Triple ==
sizeof(struct Triple) = 12
  a offset 0
  b offset 4
  c offset 8
```

Walk the alignment rule and the numbers follow. `a` takes offset 0. `b` is an
`int` with alignment 4, so it cannot sit at offset 1: the compiler pads three
bytes and places it at 4. `c` follows at 8. The members end at offset 9, but
the struct's alignment is 4 — the largest of its members' — so the size is
rounded up to 12. Those three trailing bytes are padding too: without them an
array of `Triple` would put the second `a` at offset 9 and its `b` would again
be misaligned. That is why `sizeof(struct Triple[2])` is 24 and not 18.

Re-run the arithmetic against every struct you meet and the numbers stop
surprising you: members at multiples of their alignment, size rounded up to
the struct's alignment.
