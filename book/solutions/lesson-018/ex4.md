# Solution: exercise 4 — Prove the fix

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 018 — the optimizer and undefined behavior](../../lessons/part-0/lesson-018-optimizer-ub.md).*

## The diff

```diff
{{#include ex4.patch}}
```

## Walkthrough

The added pass counts bytes that are not the background value, and the
program vouches for its own clear:

```
clear covered 144 bytes
clear verified: 0 wrong bytes
```

The rest of the proof is by comparison, and all of it passes: running the
`-O0` and `-O2` builds into two files and diffing them reports no
difference, and `file paint.bmp` from either build says `PC bitmap,
Windows 3.x format, 8 x 6 x 24 … cbSize 198`. What each check proves is
worth spelling out. The verify pass proves the clear *landed* — a
semantic property, checked in the program itself. The `diff` proves the
two builds agree on everything the program printed. `file` proves the
output artifact is well-formed. And what `diff` of the outputs cannot
catch: anything that never reaches stdout — a wrong byte in the BMP
would sail past a console diff (compare the files too, as the prompt's
last step does) — and, more deeply, two identical outputs never prove the
*absence* of undefined behavior: both builds could be wrong the same way.
Agreement is a symptom of correctness, not a proof of it.
