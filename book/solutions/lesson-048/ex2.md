# Solution: exercise 2 — The compiler's signature

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 048 — the blitter's compiled assembly](../../lessons/part-2/lesson-048-assembly.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

The patch adds one line to the startup report — the compiler's own
predefined macro, `__VERSION__`, printed next to the numbers it produced:

```
engine: compiler 13.3.0
```

That is GCC 13.3.0 — the same toolchain the book's tables were measured
with. From now on every run your machine produces says which compiler
made the code it is timing; two runs that differ in *shape* (not just
digits) can be compared by their signatures first. A measurement without
its machine is a rumor, and the compiler is part of the machine.

Now the comparison itself. Build with your compiler and run
`./tools/disasm.sh`; then walk the same three landmarks the lesson read
in GCC's listing:

1. **The prologue and the argument spills.** How does your compiler bring
   `fb`, `sprite`, `x`, `y` home? x86-64 fixes the *registers* — `%rdi`,
   `%rsi`, `%edx`, `%ecx` is the platform's ABI, not any compiler's idea
   — but whether the values spill to `-0xNN(%rbp)` immediately, live in
   registers, or get copied twice is entirely the compiler's. At `-O0`
   most compilers spill (that is what "no optimization" means), and the
   slot offsets will differ.
2. **The clip clamps.** The book's column shows `test`/`cmovs` and
   `cmovle` — conditional moves, no branch. Another compiler at `-O0`
   may emit compare-and-jump instead: load, `test`, `jns` past a `mov`,
   then continue. Same behavior, different shape; a branch predicts
   trivially here (the clamp almost never fires), so the difference does
   not matter — until it does, which is why you read rather than assume.
3. **The four stores of the copy.** Expect the same store order (the
   statements' order is observable — the compiler may not reorder stores
   to the same address in ways the C++ forbids) and the same `× 4` shift
   for the destination address. What *will* differ is which registers the
   bytes ride in and how the address arithmetic is folded — `lea`
   instructions computing `base + index*4 + 1` in one step are common.

The dividing line the prompt asks for: **the register names, the
addresses, and the instruction choices are the compiler's; the behavior
they produce is the language's.** `BlitSprite`'s contract — which pixels
change and which do not — must come out of every compiler's listing
identically, or the compiler is broken. The listing is opinion; the
pixels are fact. That is the as-if rule of lesson 018, and it is why
this course teaches you to read the listing instead of memorizing one.

One more landmark worth adding to your comparison while you are there:
the sizes of the functions. `wc -l` the disassembly of `BlitSprite` and
of `ClearBuffer` on both compilers. Function size is where compiler
personalities show most clearly, and "my compiler made the loop
twice as long" is the beginning of a conversation about code generation
that lesson 049 continues.
