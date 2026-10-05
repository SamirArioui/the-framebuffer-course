# Solution: exercise 4 — Runs, not cells

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 022 — the double-buffered character grid](../../lessons/part-0/lesson-022-double-buffer.md).*

## The diff

```diff
{{#include ex4.patch}}
```

## Walkthrough

The diff flush was paying one cursor-position escape per changed cell —
roughly 7 bytes of addressing to deliver 1 byte of character. The patch
changes the unit of transmission from cell to *run*: on finding a changed
cell the flush emits one `ESC [ row ; col H`, then writes that cell and every
consecutive changed neighbor with plain `fputc` calls, breaking at the first
unchanged cell. The inner loop owns `col` until the run ends; the outer loop
resumes just past it. `fputc` is enough because the terminal advances the
cursor by itself as characters arrive.

Measured on the same runs: `./snek 60` falls from 6 949 to 1 127 bytes and
`./snek 120` from 7 289 to 1 324 — about 6× less traffic, no behavior change.
The first flush benefits most (whole rows collapse to one escape plus their
characters), and the moving marker — two adjacent cells — becomes one short
run. This is the same shape as every serious terminal renderer and, for that
matter, every network protocol that batches small writes: when the cost is
per-message, make the messages worth sending.
