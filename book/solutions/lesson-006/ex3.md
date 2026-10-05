# Solution: exercise 3 — Three fates

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 006 — undefined behavior and buffer overflows](../../lessons/part-0/lesson-006-undefined-behavior.md).*

## The diff

```diff
{{#include ex3.patch}}
```

## Walkthrough

The sorting. *Undefined*: signed overflow (exercise 1's `++big`) and
out-of-bounds access (exercise 2's backwards read) — the standard voids
its contract entirely. *Defined but wrong*: `size_t` wraparound — the
doubling `buf->cap * 2` is specified to circle back to a small number; the
result is nonsense without any rules breaking. *Indeterminate*:
uninitialized reads, like lesson 003's garbage locals.

The three checks map onto the first two fates and one adjacent bug.
`BufferAt` defends against out-of-bounds: the index is tested against the
allocation before the access, so UB becomes a named error and a clean exit.
The growth check defends against the defined-but-wrong wraparound: it
computes the doubled value first (the wrap, if any, has already happened —
unsigned arithmetic is like that) and rejects `new_cap < buf->cap`. The
`realloc` NULL check guards no undefined behavior at all — it guards the
classic `p = realloc(p, n)` pattern where failure loses the old pointer.
The indeterminate fate is fought by initialization instead of a check
(`BufferInit` zeroes, `last` starts as `?`).

"It worked when I tested it" answers none of this: it records one build's
behavior at sizes you happened to try. The instrument shows the checks at
work on every growth:

```
checked growth: 0 -> 64
checked growth: 64 -> 128
...
checked growth: 4096 -> 8192
```
