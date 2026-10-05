# Solution: exercise 2 — What the certainty costs

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 005 — leaks made visible with sanitizers](../../lessons/part-0/lesson-005-leaks.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

On one machine (gcc 13.3.0, x86-64), `time ./wordcount med.txt` over a 20 MB
text file:

```
fgetc, plain:       real 0m0.112s
fgetc, sanitizer:   real 0m0.174s
fread,  plain:      real 0m0.075s
fread,  sanitizer:  real 0m0.150s
```

The per-byte loop was indeed drowning part of the signal, and the
block-read change — `fread` into a 4 KB chunk, then the same per-byte body
over the chunk — is what separates the costs: the sanitizer runs about 1.6×
slower on the `fgetc` loop and about 2× slower on the block loop. Where the
extra time goes: every load and store in the instrumented code consults the
shadow memory map, and every `malloc`, `realloc`, and `free` is the
runtime's, paying for bookkeeping, redzones, and a quarantine of recently
freed blocks. Memory costs too — the shadow map is a fraction of the whole
address space, and each allocation carries fences. The numbers say the
certainty is not free but is cheap enough to leave switched on for the
whole debugging phase — and to switch off for the release build, which is
exactly why the two build commands live side by side.

Your ratios will differ; the method is the deliverable: measure the tool
that watches your code before deciding it is too slow to use.
