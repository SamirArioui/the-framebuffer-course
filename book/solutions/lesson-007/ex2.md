# Solution: exercise 2 — The padding is real memory

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 007 — structs: sizeof, alignment, and padding](../../lessons/part-0/lesson-007-struct-layout.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

`DumpBytes` walks the object as `unsigned char` bytes and prints each one in
hex — one run here:

```
41 00 00 00 00 00 00 00 2a 00 00 00 00 00 00 00 5a 71 27 5a fc 7f 00 00
41 00 00 00 00 00 00 00 2a 00 00 00 00 00 00 00 5a 00 00 00 00 00 00 00
```

The first line is the hand-assigned struct, the second the `memset`-zeroed
one. `41` is `tag` = 'A', `2a 00 00 00 00 00 00 00` is `score` = 42 (low byte
first — endianness is lesson 014's subject), and `5a` is `flag` = 'Z'. The
bytes between them are padding, and the first line shows them holding
whatever the stack left there: the trailing gap changed on every run of this
program (`5a 71 27 5a fc 7f 00 00`, then `5a 32 ed ac fc 7f 00 00`, …), and
the interior gap happened to read as zero — by luck, not by rule.

`memset` is the only reliable way to zero a struct's padding, and the second
line proves it. That matters whenever bytes are compared or hashed as a
whole: `memcmp` on two structs with equal fields can still report "different"
because their padding differs.
