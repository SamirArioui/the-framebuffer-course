# Solution: exercise 4 — The contract, written down

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 010 — void*: genericity and its pain](../../lessons/part-0/lesson-010-void-pointer.md).*

## The diff

```diff
{{#include ex4.patch}}
```

## Walkthrough

(a) Through `elem_size`: pass `sizeof nums` — the struct's size — and every
`DaPush` copies more bytes than the element owns (exercise 2's overflow).
Through a wrong cast: read a `long` slot as `const int *` and the same bytes
answer a different question (exercise 1's `7 0`). Both escape the compiler
because `DaInit`'s `size_t` and `DaAt`'s `void *` erased the element type —
there is no declared type left for a mismatch to violate, so the diagnostic
that would fire on typed code has nothing to fire on. (b) Python and Ruby
reject both mistakes at runtime — `TypeError`, wrong results never
half-silently produced — because every value carries its type and every
operation checks it. The cost is per-operation work forever. C's bargain is
the opposite: zero runtime cost, zero runtime checks. (c) `DaAt` and
`DaEach` hand out pointers *into* the data block, not copies — that is what
makes `DaPush`'s `memcpy` and the `qsort` call possible without copying
whole containers. The implication: any growth that triggers `realloc` may
move the block and silently invalidate every pointer previously handed out,
and a pointer from `DaAt` is only good until the next push that grows —
lesson 008's `realloc` contract, now leaking into the API's.

The confirming check prints the machinery as it runs — real lines from one
execution:

```
push elem_size=24 at offset 0
push elem_size=24 at offset 24
push elem_size=24 at offset 48
...
push elem_size=8 at offset 0
push elem_size=8 at offset 8
```

Two element sizes, one code path: stride 24 for `struct Item`, stride 8 for
`long`, each push landing at `len * elem_size`.
