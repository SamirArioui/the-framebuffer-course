# Solution: exercise 4 — Where the functions live

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 024 — the function-pointer command table](../../lessons/part-0/lesson-024-command-table.md).*

## The diff

```diff
{{#include ex4.patch}}
```

## Walkthrough

A function pointer is not magic — it is the code's address, held in data.
The instrumented dispatch prints it: pressing space reports
`key=32 run=0x589a21ff7962` and `q` reports `key=113 run=0x589a21ff794d` on
one machine. Compare with the binary's own symbol table:

```
$ nm snek | grep -E 'Cmd(Quit|Start|Up)$'
000000000000194d t CmdQuit
0000000000001962 t CmdStart
00000000000019f2 t CmdUp
```

The low bits match exactly: `CmdQuit` is offset `0x194d`, `CmdStart` is
`0x1962` — the runtime addresses are those offsets plus the load address of
the executable (a PIE binary is relocated at start; `nm` shows offsets).
`commands[i].run()` compiles to an indirect call through that address, and
the struct member is where the address is stored. Calling it and jumping to
it are the same machine act.

This is why the table is the C route to interfaces: a row pairs data (the
key) with behavior (the address of a function), and the scanner does not
know or care *which* function it calls. Swap the address, get different
behavior — no recompilation of the dispatcher needed. Lesson 025 turns this
shape into a C++ class with a virtual method, and the machine code is almost
the same: a table of function addresses, one indirect call, a whole language
feature built on exactly what you just printed.
