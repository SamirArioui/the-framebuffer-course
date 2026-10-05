# Solution: exercise 4 — Ask your own machine

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 014 — endianness and image-header layout](../../lessons/part-0/lesson-014-image-headers.md).*

## The diff

```diff
{{#include ex4.patch}}
```

## Walkthrough

The added line stores the integer 1 and looks at its first byte: 1 if the
least significant byte lives at the lowest address, 0 otherwise — a
run-time endianness test with no macros. On this machine the program prints

```
host byte order: little-endian
```

and `lscpu` agrees (`Byte order: Little Endian`, `uname -m` says `x86_64`).
On a big-endian machine the same code prints `big-endian` — the test
inspects memory, so it needs no knowledge of the architecture. For the
final question, look at which bytes depend on the host: the *header* dump
never changes, because `PutU16LE`/`PutU32LE` emit pinned little-endian
bytes with shifts — the same output everywhere. The lines that would
change are only the host probes: the `0x01020304 in memory` line would
read `01 02 03 04`, and this exercise's new line would flip. That split is
the whole point of the lesson — program output can depend on the machine;
file bytes must not.
