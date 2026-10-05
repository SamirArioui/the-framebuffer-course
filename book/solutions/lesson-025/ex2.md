# Solution: exercise 2 — The hidden word

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 025 — the C++ subset: classes and vtables](../../lessons/part-0/lesson-025-cpp-subset.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

The tempting prediction is "zero — both views carry no data". C++ never lets a
complete object have zero size (distinct objects need distinct addresses), so
the empty twin prints 1; the question is what the views print on top of that.
The instrumented run reports, on one 64-bit machine:

```
sizes: GridView=8 StatusView=8 PlainView=1
```

Eight bytes is exactly one pointer-sized word: the **vptr**, stored in every
object whose class has virtual functions. `PlainView` has an ordinary member
function, no `virtual`, so it keeps only the mandatory byte. The vptr is what
`views[i]->Draw(grid)` loads first — it points at its class's table of function
pointers, which exercise 3 follows to the end. On a 32-bit machine expect
`4 4 1`: the word is a pointer there too. Note the twin's `Draw` is never
called or defined — `sizeof` is a compile-time question, answered before any
of this runs.
