# Solution: exercise 3 — Conditions see one frame

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 002 — gdb: breakpoints, stepping, stack frames](../../lessons/part-0/lesson-002-gdb.md).*

## The diff

```diff
{{#include ex3.patch}}
```

## Walkthrough

The first condition dies at definition time:

```
(gdb) break CountBytes if i == 2
No symbol "i" in current context.
```

A condition is resolved against the scope of the place the breakpoint sits
in. Inside `CountBytes` those names are its parameter and locals — `f`,
`bytes`, `c` — plus file-scope names; `i` is a local of `main`'s loop and
belongs to a frame that will not even exist when the condition is tested.
`break CountBytes if bytes == 0` *is* accepted (`bytes` is a name
`CountBytes` knows), but at function entry the initializer has not run yet —
`bytes` holds whatever garbage the stack slot carried, almost never zero —
so the breakpoint passes by both calls without stopping.

`$hits` is different in kind: a gdb *convenience variable*. It lives inside
the debugger, not in your program; nothing you compile can see it. gdb
evaluates the condition on every hit, `++$hits` counts them, and the
breakpoint fires on exactly the second call — confirmed by the instrumenting
print, which shows `file 2: b.txt` on stderr just before the stop.
