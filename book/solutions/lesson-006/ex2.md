# Solution: exercise 2 — The empty line that reads backwards

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 006 — undefined behavior and buffer overflows](../../lessons/part-0/lesson-006-undefined-behavior.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

For an empty line, `line.len` is 0, and `line.len - 1` is `size_t`
arithmetic: it wraps to the largest possible index, and the read lands
exactly one byte *before* the buffer's allocation. The sanitizer names the
geography precisely:

```
==105278==ERROR: AddressSanitizer: heap-buffer-overflow on address 0x50600000001f ...
READ of size 1 at 0x50600000001f thread T0
    #0 ... in CountStream .../sandbox/wordcount/wordcount.c:106
    ...
0x50600000001f is located 1 bytes before 64-byte region [0x506000000020,0x506000000060)
allocated by thread T0 here:
    #1 ... in BufferGrow ...
```

Exit status 1. Note the recursion with the prose's wraparound story: the
*defined* unsigned wrap produced the *undefined* out-of-bounds access. The
plain build is the other half of the lesson — it read a stale byte and
printed `last char: ' '` (whatever byte sat before the block on your run),
exit 0: undefined behavior is not obliged to crash.

The fix is two disciplines. The guard `if (line.len > 0)` makes the empty
line mean "no character to report", so the feature is honest about its own
domain. And the read goes through `BufferAt`, the checked accessor — if
the guard were ever wrong again, the program would name the bad index and
exit cleanly instead of reading backwards. Verified on both builds:
`last char: 'e'` for `story.txt`, `last char: 'o'` for a file whose empty
line is skipped over the earlier `hello`.
