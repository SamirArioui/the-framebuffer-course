# Solution: exercise 1 — Standard input

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 001 — argv and file input: your first `gcc` command](../../lessons/part-0/lesson-001-first-program.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

Standard input is not a new concept in C — it is a `FILE *` that is already
open when the program starts. The fix replaces the usage branch with the same
counting loop the file path uses, reading from `stdin` instead of a handle
from `fopen`, and printing the bare count since there is no file name to
label it with. `./wordcount < notes.txt` now works, and so does a pipe:
`ls | ./wordcount`.

The duplication between this loop and the one in the file loop is deliberate
at this stage of the program: both are five lines you can read end to end.
Lesson 002 pulls the file loop into a function of its own — for a debugger
reason, of all things. When you get there, the stdin path you wrote here can
become a one-line caller of that same function.

One decision is worth naming: when a file argument *is* given, stdin is
ignored entirely. That is what `wc` does, and matching it keeps the program
predictable in pipelines.
