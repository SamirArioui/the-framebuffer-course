# Solution: exercise 3 — Why the bridge exists

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 009 — function pointers: comparators and hooks](../../lessons/part-0/lesson-009-function-pointers.md).*

## The diff

```diff
{{#include ex3.patch}}
```

## Walkthrough

(a) `qsort`'s comparator parameter is declared `int (*)(const void *, const
void *)`, and `CmpByKey` has type
`int (*)(const struct Item *, const struct Item *)` — a different
function-pointer type. C requires a diagnostic if one is passed where the
other is expected, and while converting *between* function-pointer types is
allowed, calling through a converted pointer whose type does not match the
function actually there is undefined behavior. The bridge is the conforming
crossing: `void *` in, explicit cast, typed call.

(b) `g_cmp` compensates for `qsort` passing the comparator no context —
there is no slot for "which comparator did `DaSort` mean" (POSIX's `qsort_r`
adds one; standard C has none). The workaround stops being safe the moment
two sorts can be in flight at once: comparator code that itself calls
`DaSort`, or two threads sorting concurrently, would fight over the same
file-static variable.

(c) `DaEach` packages the traversal so callers ship only behavior — useful
when the array is one of many, or the walk must stay identical while the
action varies. For a one-off loop over a local array, a plain `for` is
shorter and clearer; callbacks earn their keep when the loop is not yours to
rewrite.

The confirming check makes the plumbing visible — real calls from one run:

```
bridge: pear vs apple
bridge: banana vs cherry
bridge: fig vs banana
```

41 bridge calls in one program run: both sorts share the bridge, and each
`qsort` of ten elements asks roughly twenty questions.
