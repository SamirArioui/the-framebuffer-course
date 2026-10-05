# Solution: exercise 3 — Review a colleague's fill

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 018 — the optimizer and undefined behavior](../../lessons/part-0/lesson-018-optimizer-ub.md).*

## The diff

```diff
{{#include ex3.patch}}
```

## Walkthrough

The pasted `FillGradient` is the lesson-017 bug wearing a different hat:
`while (i >= 0)` with `i++` inside, stopping only when the counter wraps.
Confirming the report takes one round-trip: `-O0` draws the ramp, writes
`paint.bmp`, exits 0; the `-O2` build warns (`iteration 2147483647
invokes undefined behavior`, pointing at `i++` in `FillGradient`) and
`timeout 5 ./paint` comes back 124. The fix is the same medicine: a loop
bound that is the buffer's byte count — `for (int i = 0; i < n; i++)` —
so the loop ends because `i` reaches `n`, which no optimizer may
reinterpret. The ramp math (`(i / 3) * 255 / (w * h - 1)`) is untouched;
only the stopping was broken. With the call added after `ClearBuffer`,
both builds now print identical output and write the same valid BMP —
verified with `diff` of the two runs and `file` on the result. The review
habit to keep: when someone says "same style as" a piece of code you
already fixed, re-review the *shape*, not just the arithmetic.
