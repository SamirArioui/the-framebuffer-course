# Solution: exercise 4 — What `realloc` really promises

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 008 — dynarray growth: realloc and capacity](../../lessons/part-0/lesson-008-dynarray.md).*

## The diff

```diff
{{#include ex4.patch}}
```

## Walkthrough

(a) Under doubling, growths happen at capacities 4, 8, 16, …, n/2 and the
elements re-stored across all of them sum to less than n — a geometric
series — so over *n* pushes the copying adds a constant average to each push.
Under one-slot growth, growth number *k* re-stores *k* elements and the total
is n(n+1)/2: quadratic, and no averaging saves it. (b) On failure `realloc`
returns `NULL` and the old block is left intact, allocated, and yours — so
`da->items = realloc(da->items, ...)` overwrites the only pointer to a
perfectly good block with `NULL`: the block leaks instantly. The temporary
`p` keeps the old pointer safe until success is known. (c) `realloc` returns
the same pointer when it can extend the block where it stands — free space
must immediately follow it — and a different one when it must allocate,
copy, and free.

The confirming check labels each case as it happens. Three growths, three
different answers:

```
realloc cap 0 -> 4 (fresh)
realloc cap 4 -> 8 (moved)
realloc cap 8 -> 16 (in place)
```

`fresh` is the first growth: there is no old block to move or extend. The
second growth moved; the third extended in place. Run the driver with fifty
pushes and the stream shows the mix the heap hands you — which is exactly why
the growth *rule* must not depend on the heap being kind (see exercise 3).
