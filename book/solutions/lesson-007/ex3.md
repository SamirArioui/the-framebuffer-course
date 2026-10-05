# Solution: exercise 3 — Why the machine insists

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 007 — structs: sizeof, alignment, and padding](../../lessons/part-0/lesson-007-struct-layout.md).*

## The diff

```diff
{{#include ex3.patch}}
```

## Walkthrough

The three answers, in order. (1) `score` goes to offset 8 because it is a
`long` and the machine's rule — the ABI, really a contract between compiler
and CPU — says a `long` lives at an address that is a multiple of 8. Offset 8
is the first such spot after `tag`. (2) At offset 1 the CPU's load
instructions would have to fetch `score` across two aligned words: on x86
that works and is slower; on stricter architectures an unaligned access
traps. Compilers therefore never choose it — the alignment is baked into the
calling convention and every struct layout. (3) A `fwrite` dump contains the
members *and* the padding, laid out for this machine's `long` size and byte
order. Another machine disagrees about both, so the bytes read back as noise.

The confirming check prints the numbers the rule runs on:

```
alignof(char) = 1, alignof(int) = 4, alignof(long) = 8
```

Offsets are multiples of these, struct sizes are multiples of the largest —
every layout in this lesson falls out of that one line.
