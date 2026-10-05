# Solution: exercise 2 — Searching by predicate

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 009 — function pointers: comparators and hooks](../../lessons/part-0/lesson-009-function-pointers.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

`DaFind` is `DaEach`'s loop with a question instead of an action: the hook
here is a predicate — `int (*)(const struct Item *)` — that answers yes (non-
zero) or no (zero) about one item. The walk stops at the first yes and hands
back a pointer to the element; a walk with no yes hands back `NULL`. The two
driver queries print:

```
found grape 7
mango: not found
```

The `NULL` answer is not optional decoration — it is the only answer the
second query can give, so the driver checks before touching `hit`, the same
discipline `fopen` taught with file handles back in lesson 001. A pointer
that can be "empty" is checked; that habit is most of C's defensive style.

Two small notes. `DaFind` returns `const struct Item *` on purpose: a search
should not be a back door to mutation. And look at what the two predicates
cost: one named function each, and neither can ask its question with an
argument — `KeyIsGrape` hard-codes `grape`. Carrying *context* into a
callback is the pain lesson 010 is about; the `g_cmp` of the code step is the
same problem wearing a different hat.
