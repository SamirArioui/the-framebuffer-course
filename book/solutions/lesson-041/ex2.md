# Solution: exercise 2 — The bug ASan cannot see

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 041 — arenas](../../lessons/part-1/lesson-041-arenas.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

Another **deliberate teaching state**: this exercise corrupts memory on
purpose to show that the tool the course has trusted since lesson 005 has
a blind spot here. The patch rolls an allocation back, keeps its pointer,
allocates again — and writes through the stale one.

Build it with the sanitizer, the way lesson 005 taught:

```
$ CXXFLAGS="-std=c++17 -O0 -g -Wall -Wextra -fsanitize=address" \
  LDFLAGS="-lX11 -fsanitize=address" ./build.sh
$ DISPLAY=:99 ./build/game '?asan'
engine: stale write landed on the new block: 90 (same memory: yes)
$ echo $?
0
```

The stale write **landed on the new allocation** — `90` is `0x5A`, the
byte written through the stale pointer, read back through the fresh one.
Two "different" allocations are the same memory, and one silently
overwrote the other. And AddressSanitizer printed nothing at all: no
ERROR, no shadow-byte report, exit code 0.

Why the silence? ASan guards *allocations it knows about* — the
`malloc`/`free` traffic it intercepts — by marking freed and never-yet-
allocated bytes in a shadow memory. The arena's memory is none of that: it
is one big anonymous mapping the OS handed us (lesson 040), and every
"allocation" inside it is arithmetic. The bytes `stale` names are *mapped
and owned* at every moment of this program's life; from the machine's
point of view nothing illegal happened. ASan cannot distinguish "the
arena's free space" from "the arena's used space" because the arena did
not tell it — and a bump allocator does not call free.

What *would* catch it, in order of cost:

1. **Marks and discipline** — a rolled-back pointer is dead by
   convention, the same way `free` makes a pointer dead. Cheap, and the
   rule the arena's contract already states.
2. **Poisoning** — an arena can deliberately mark its unused region
   inaccessible (lesson 040's exercise 2, `MakeInaccessible`) or fill it
   with a sentinel and check it. Part 4's arena service can carry debug
   poisoning exactly the way real engines do.
3. **A sanitizer that knows** — ASan's custom-allocator hooks exist for
   this (`__asan_poison_memory_region`); a debug build of the arena can
   speak to it. That is the honest endgame and it is worth knowing exists.

The write-up to aim for: tools see what they are told about. Lesson 005's
sanitizer was honest about the heap; the arena is not the heap, and the
same bug is now invisible — which is exactly why the limitation is taught
here and not discovered in Part 4.
