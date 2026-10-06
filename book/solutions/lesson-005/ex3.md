# Solution: exercise 3 — How the sanitizer knows

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 005 — leaks made visible with sanitizers](../../lessons/part-0/lesson-005-leaks.md).*

## The diff

```diff
{{#include ex3.patch}}
```

## Walkthrough

The instrumented run prints the truth the report is built from (captured
with output redirected: through one pipe the unbuffered `freeing` lines
arrive first; on a terminal the rows interleave — `story.txt`'s row after
the first `freeing`, `long.txt`'s after the second. Addresses are the
sanitizer allocator's and vary across versions and machines):

```
freeing 64 bytes at 0x506000000020
freeing 512 bytes at 0x515000000580
2 4 21 11 story.txt
1 1 301 300 long.txt
```

The mechanism, in the order it happens. The build replaced `malloc` and
`free` with the sanitizer runtime's versions — not by convention but by
linking: every call site, ours and the C library's, resolves to the
runtime. Each allocation is recorded in a table together with the call
stack that requested it, which is what the report later prints: the stack
was captured at *allocation* time, not at failure time, so it names
`BufferGrow` and friends no matter how the block is lost. At exit the
LeakSanitizer audit runs: it freezes the process and scans every place a
pointer could live — registers, stacks, globals — for addresses of still-
allocated blocks. Anything allocated and unreferenced is reported as a
leak; anything still referenced (like `stdin`'s internals) is not.

Under the plain build none of this exists: `free` is the C library's,
nothing is recorded, nothing audits, and the leak has no visible symptom
except the process's own memory footprint. That is the sanitizer's whole
value proposition — it converts a silent, order-dependent, size-dependent
class of bug into a deterministic report at exit.
