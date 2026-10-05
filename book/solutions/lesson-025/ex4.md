# Solution: exercise 4 — The copy that cannot exist

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 025 — the C++ subset: classes and vtables](../../lessons/part-0/lesson-025-cpp-subset.md).*

## The diff

```diff
{{#include ex4.patch}}
```

## Walkthrough

The compiler does not refuse the copy out of pedantry — `Drawable` is an
interface, and the refusal is the abstract-type rule doing its job:

```
error: cannot allocate an object of abstract type ‘snek::Drawable’
note:   because the following virtual functions are pure within ‘snek::Drawable’:
note:     ‘virtual void snek::Drawable::Draw(snek::Grid&) const’
```

Had `Drawable` not been abstract, the same line would have compiled and been
silently wrong: the copy keeps only the base part of the object — the classic
*slice* — and `saved.Draw(grid)` would call the base's function instead of
`StatusView::Draw`. C++ turned a runtime trap into a build error. The fix is
`Drawable &saved = status;`: a reference is a pointer the compiler dereferences
for you, so nothing is copied and the vptr stays where it belongs. Proof that
dispatch still lands in `StatusView::Draw`: the run is byte-identical to the
lesson's end state — same title text, same death at tick 9 (`frame=28 tick=9
state=dead score=0 dir=up at=2,20`).
