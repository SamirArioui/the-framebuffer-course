# Solution: exercise 3 — Removing in the middle

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 010 — void*: genericity and its pain](../../lessons/part-0/lesson-010-void-pointer.md).*

## The diff

```diff
{{#include ex3.patch}}
```

## Walkthrough

`DaRemoveAt` is all arithmetic and one library call. The elements after the
victim must slide down one stride each, and the slide source and the slide
destination overlap by every byte past the gap — which is precisely the case
`memmove` exists for and `memcpy` makes no promises about. `memmove` copies
as if through a temporary, so the overlap is safe; with `memcpy` on an
overlapping slide, behavior is undefined and the corruption is silent.

The demo reuses the driver's five `long`s — sorted, so `10 20 30 40 50` —
and removes index 2, which is the `30`:

```
after removing index 2:
10
20
40
50
```

Two boundary rules make the function honest. Removing the *last* element is
a zero-byte `memmove` and just decrements `len` — the `(len - i - 1)`
arithmetic handles that for free. And an index at or past `len` is rejected
with a message instead of sliding bytes that do not exist. Note what does
*not* happen: no reallocation. `len` shrinks, `cap` does not — the capacity
paid for in lesson 008 stays paid for, ready for the next push.
