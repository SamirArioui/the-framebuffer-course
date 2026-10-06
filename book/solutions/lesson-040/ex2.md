# Solution: exercise 2 — The page that fights back

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 040 — reservation-backed buffers](../../lessons/part-1/lesson-040-reservations.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

This is a **deliberate teaching state** — the patch exists to crash, once,
on purpose, so the machine's refusal can be watched instead of imagined.
It adds one seam function (`MakeInaccessible` — `mprotect` with
`PROT_NONE` behind the interface) and one guarded reservation that is
touched afterward.

```
$ DISPLAY=:99 ./build/game '!crash'
engine: touching a page with no permissions
Segmentation fault (core dumped)
$ echo $?
139
```

Read what happened at machine level. The reservation mapped a page with
read/write permission; `MakeInaccessible` asked the OS to change the
page-table entries for it to *no* permissions — `PROT_NONE`. Nothing in
the language knows this happened: `guarded.bytes[0]` is an ordinary
unsigned-char read, compiled to an ordinary `mov`. When that instruction
executed, the CPU's address translation found the page's entry, saw the
permission bits forbid reading, and **stopped the instruction**. The OS
received the fault, looked at it, and delivered `SIGSEGV` to the process —
the default action for which is the death you just watched.

Every layer has a say here and none of them could save the read: C++ has
no check for it, the compiler cannot see it, and no exception carries it.
Lesson 006 showed undefined behavior that *sometimes* crashes; this is the
machine itself refusing, deterministically, every time. That is what
"protection" means in lesson 039's page tables: bits, checked in hardware,
on every access.

(One detail in the patch worth noticing: the probe's prints go to
`stderr`. The run dies mid-expression — `stdout`'s buffer would die with
it. Lesson 001's stream-ordering lesson, earning its keep in a crash.)

What the fix would be, if this were not deliberate: never hand out pages
whose permissions do not match their use. The reservation API gives
read/write pages because that is what buffers need — and that is why the
access the engine never performs is the one the OS must forbid at the
page-table level when it matters (stack guard pages work exactly this
way).
