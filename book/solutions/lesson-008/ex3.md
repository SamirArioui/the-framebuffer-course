# Solution: exercise 3 — Doubling versus one-at-a-time

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 008 — dynarray growth: realloc and capacity](../../lessons/part-0/lesson-008-dynarray.md).*

## The diff

```diff
{{#include ex3.patch}}
```

## Walkthrough

The counters — `reallocs`, `model_elems` (elements the growth rule makes the
array re-store), `moved_elems` (elements really copied when a block moved) —
tell two different stories at once. Measured on the author's machine:

```
GROW_BY_ONE=0 N=1000000   reallocs=19        model_elems=1048572        moved_elems=1044480    real 0.01s
GROW_BY_ONE=1 N=1000000   reallocs=1000000   model_elems=499999500000   moved_elems=16374177   real 0.01s
GROW_BY_ONE=0 N=5000000   reallocs=22        model_elems=8388604        moved_elems=8384512    real 0.06s
GROW_BY_ONE=1 N=5000000   reallocs=5000000   model_elems=12499997500000 moved_elems=8715529    real 0.06s
```

The model's verdict is brutal — half a trillion elements versus one million,
seven orders of magnitude. The wall clocks, though, cannot see it at all.
Where did the quadratic copying go? Not into `moved_elems`: only 16 million
elements actually moved. The allocator is dodging the model — when free space
follows the block, `realloc` extends it in place and copies nothing (the
check in exercise 4's solution makes this visible). On a fresh heap, growth
is nearly free no matter the schedule.

Do not conclude that the schedule does not matter. The in-place escape is a
*windfall*, not a contract: on a fragmented heap, or with another live
allocation sitting behind yours, growth must move — and then `model_elems`
becomes real copying. Doubling bounds that work under every heap shape
(under 2n elements, ever); one-slot growth bets everything on luck. The
realloc-call count alone — 19 versus 1,000,000 — is a measured cost you pay
either way.
