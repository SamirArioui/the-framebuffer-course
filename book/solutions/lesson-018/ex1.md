# Solution: exercise 1 — Predict the wreckage

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 018 — the optimizer and undefined behavior](../../lessons/part-0/lesson-018-optimizer-ub.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

The patch restores the lesson-017 fill — the loop that stops when its
offset turns negative — and adds one `fprintf` at its entry. Both
predictions play out. The `-O2` build blames `ClearBuffer`'s increment:

```
warning: iteration 2147483647 invokes undefined behavior
[-Waggressive-loop-optimizations]
   162 |         i++;
```

and the runs split exactly as expected. `-O0` prints the marker, then
`clear ended at offset -2147483648`, then writes the file and exits 0.
`timeout 5 ./paint` at `-O2` prints *only* the marker — `clear: entering
the naive fill` — and `echo $?` says `124`: the run entered the fill and
never came back. The marker is the confirming change that matters: it
proves the hang is *inside* the loop, not before it. The wrapped value
`-2147483648` in the `-O0` output is the whole diagnosis in one number —
the loop only ended because the counter overflowed, which is precisely
what `-O2` is entitled to assume cannot happen.
