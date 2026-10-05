# Solution: exercise 4 — A counter in two scopes

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 002 — gdb: breakpoints, stepping, stack frames](../../lessons/part-0/lesson-002-gdb.md).*

## The diff

```diff
{{#include ex4.patch}}
```

## Walkthrough

The counter is one file-scope variable, incremented at the top of
`CountBytes` and reported by `main` after the last file:

```
CountBytes called 2 times
3 a.txt
6 b.txt
```

The scope check in gdb is the point. Stopped inside `CountBytes`:

```
(gdb) print calls
$1 = 0
(gdb) print i
No symbol "i" in current context.
(gdb) up
#1  0x00005555555552e4 in main (argc=3, argv=0x7fffffffdab8) at wordcount.c:32
(gdb) print calls
$2 = 0
(gdb) print i
$3 = 1
```

A local like `i` belongs to a frame: it exists only while that frame is
alive and is only nameable from inside it. `calls` has one storage location
for the whole run, so every frame can name it. (It reads `0` at the first
stop because the breakpoint fires before `++calls` runs.) Note that `static`
limits *compile-time* name visibility to this file — it does not hide the
variable from the debugger. This locals-versus-globals difference is exactly
what exercise 3 ran into from the other side.
