# Solution: exercise 1 — The growth schedule

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 008 — dynarray growth: realloc and capacity](../../lessons/part-0/lesson-008-dynarray.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

The prediction to beat: capacities change after pushes 1, 5, and 9 — not
after 4 and 8. The growth branch runs *before* the store, on the push that
finds `len == cap`: push 5 arrives at a full four-slot block and doubles it
to eight, push 9 at a full eight-slot block and doubles it to sixteen. The
confirming `fprintf` goes to `stderr`, so `./ds-kit 2>growth.txt` keeps the
streams apart and `growth.txt` holds exactly:

```
grow 0 -> 4
grow 4 -> 8
grow 8 -> 16
```

Three reallocations for ten pushes — and the jump pattern is geometric, which
is the whole argument of the next exercises: however many pushes you make,
the doubling schedule keeps their number at about log₂ of them. If you
predicted a growth at 4 or 8, check where in `DaPush` the growth test sits
relative to `da->items[da->len++] = item`: the comparison happens against
the length *before* the new element is stored.
