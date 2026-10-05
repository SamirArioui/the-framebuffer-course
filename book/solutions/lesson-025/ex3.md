# Solution: exercise 3 — The vtable under the glass

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 025 — the C++ subset: classes and vtables](../../lessons/part-0/lesson-025-cpp-subset.md).*

## The diff

```diff
{{#include ex3.patch}}
```

## Walkthrough

The instrumented dispatch prints, on one machine:

```
vptr=0x55a956616ce8 draw slot=0x55a956613e9c
```

Compare both with the binary's own symbol table:

```
$ nm snek | grep -E 'GridView4Draw|_ZTVN4snek8GridViewE' | c++filt
0000000000001e9c T snek::GridView::Draw(snek::Grid&) const
0000000000004cd8 V vtable for snek::GridView
```

The draw slot is `GridView::Draw` plus the load address: subtract the offset
`nm` reports (`0x1e9c`) from `0x55a956613e9c` and you get the load address of
the executable — subtract `0x4cd8 + 16` from the vptr and you get the same
number (a PIE binary is relocated at start; `nm` shows offsets, exactly as in
lesson 024). So slot zero of that table holds the address of `GridView::Draw`,
and the vptr holds the table's address past the two header words (offset-to-top
and type pointer) that sit in front of the function pointers. `views[i]->Draw(grid)`
compiles to three instructions in spirit: load the object's first word, load
the slot it points at, call it.

That is lesson 024's command table, automated: the compiler writes one table
per class, plants one pointer per object, and replaces the scanner with a
fixed index. The rows `RunCommand` matched by hand are now a lookup the
hardware does; adding a subclass (exercise 1) edits no dispatch code at all.
