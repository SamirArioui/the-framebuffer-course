# Solution: exercise 4 — The debugger on your machine

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 003 — char buffers: strings by hand](../../lessons/part-0/lesson-003-char-buffers.md).*

## The diff

```diff
{{#include ex4.patch}}
```

## Walkthrough

The instrument prints each length `LineLen` computes, so it confirms what
your stepping says on any machine and any debugger:

```
$ ./wordcount story.txt
LineLen: 11
LineLen: 8
2 4 21 11 story.txt
```

Breaking on `LineLen` under gdb on this machine shows the walk directly —
note that gdb prints the string a `char *` points at as part of the frame:

```
(gdb) break LineLen
Breakpoint 1 at 0x1219: file wordcount.c, line 16.
(gdb) run story.txt
...
Breakpoint 1, LineLen (s=0x7fffffffd830 "hello world") at wordcount.c:16
16	    unsigned long n = 0;
(gdb) next
17	    while (s[n] != '\0')
(gdb) next
18	        ++n;
(gdb) print n
$1 = 0
```

The same session under lldb is a spelling exercise. The translations to try
(check `help` on your machine — this book verifies gdb and quotes only gdb
output): `gdb ./wordcount` → `lldb ./wordcount`; `break LineLen` →
`breakpoint set --name LineLen`; `run`, `next`, `step`, `finish`, and
`continue` keep their names; `backtrace` → `bt`; `info locals` →
`frame variable`; `print n` → `print n`. Windows developers meet the
debugger inside Visual Studio or WinDbg, where the same five moves —
breakpoint, run, step, inspect, backtrace — are all present under other
names. Report anything you could not translate; that list is a map of the
tooling the course will keep in the margin.
