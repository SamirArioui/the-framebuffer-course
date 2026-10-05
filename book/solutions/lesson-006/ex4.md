# Solution: exercise 4 — The price of the check

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 006 — undefined behavior and buffer overflows](../../lessons/part-0/lesson-006-undefined-behavior.md).*

## The diff

```diff
{{#include ex4.patch}}
```

## Walkthrough

On one machine (gcc 13.3.0, x86-64), the plain `-O0` build on the 20 MB
`med.txt` — one byte pushed per input byte, so `BufferAt` runs about
20 million times:

```
bounds-checked accessor:  real 0m0.138s   (0m0.139s on the second run)
raw indexing:             real 0m0.111s   (0m0.111s on the second run)
```

The check costs about a quarter of the run — affordable on a program whose
sanitizer build already costs twice that. Where the money goes at `-O0`:
not the comparison. The boundary test is one `size_t` compare; the cost is
that `BufferPush` makes a real function call to `BufferAt` for every single
byte, because `-O0` compiles the source as written and inlines nothing.
That is the honest accounting for debugging builds — and it is also the
argument for keeping the check in the source instead of writing two
versions of the code: compilers that inline small functions fold the check
into the caller, and the closing lessons of Part 0's `paint` program show
what optimizing builds do with exactly this kind of code. Measure before
you decide a safety check is too expensive; most of them are cheaper than
the bug.
