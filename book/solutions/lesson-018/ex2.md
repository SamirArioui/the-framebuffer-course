# Solution: exercise 2 — The guard that gets deleted

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 018 — the optimizer and undefined behavior](../../lessons/part-0/lesson-018-optimizer-ub.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

The rewritten loop has an explicit stop — `if (i < 0) break;` — and the
`-O0` run honors it: `clear ended at offset -2147483648`, exit 0. The
`-O2` run ignores it entirely: `timeout 5 ./paint` prints nothing and
`echo $?` is 124. Same hang, guard and all. The reason is the lesson's
whole thesis in one line: `i` starts at 0 and the loop only ever does
`i++`, so `i` can become negative *only* by signed overflow — and signed
overflow is undefined, so the compiler is entitled to assume it never
happens. Under that assumption `i < 0` is unreachable and the `break` is
dead code, which the optimizer deletes before anything runs. The
standard's authority, not gcc's mood: any conforming compiler may do
this. What would have saved the loop is a stop that exists in defined
behavior — a real bound like `i < nbytes` (the lesson's fix), or an exit
condition no assumption can erase. Guards against the "impossible" are
exactly the code the impossible-assumption takes with it.
