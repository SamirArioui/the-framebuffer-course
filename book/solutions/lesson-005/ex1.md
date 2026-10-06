# Solution: exercise 1 — Three verdicts

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 005 — leaks made visible with sanitizers](../../lessons/part-0/lesson-005-leaks.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

The three verdicts, checked against reality. `./wordcount nope.txt` prints
`./wordcount: cannot open nope.txt` and the sanitizer says nothing, exit 0 — `CountStream`
never ran, so no block was ever allocated. `./wordcount empty.txt` prints
`0 0 0 0 empty.txt` and is likewise silent, exit 0 — this one *did* run
`CountStream`, but an empty file never pushes a byte, `BufferGrow` never
fires, and there is nothing to leak. `./wordcount story.txt` is where the
block exists, and with the free commented out the verdict is:

```
Direct leak of 64 byte(s) in 1 object(s) allocated from:
    ...
SUMMARY: AddressSanitizer: 64 byte(s) leaked in 1 allocation(s).
```

with exit status 1 — one growth, one 64-byte block, one leak. Two files
give `576 byte(s) in 2 object(s)`, matching the lesson's own report.

Two details worth keeping. Commenting out the call makes the compiler
complain — `BufferFree` defined but not used — which is warnings-as-
curriculum doing its job: the build already dislikes a dead owner. And the
exercise's shape is the leak lesson's whole method: predict, then use the
tool as an oracle, and reconcile the two.
