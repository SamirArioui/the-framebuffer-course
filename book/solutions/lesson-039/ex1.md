# Solution: exercise 1 — Find your mapping

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 039 — the virtual-memory deep dive](../../lessons/part-1/lesson-039-virtual-memory.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

The engine prints the framebuffer's address; the machine prints its map.
The instrument closes the gap by computing the pages the bytes actually
touch — the address rounded *down* to a page boundary and the last byte's
address rounded down likewise:

```
engine: page size 4096 bytes
engine: framebuffer at 0x64cc86d16040 — 1228800 bytes = 300.00 pages (page-aligned: no)
engine: framebuffer spans 0x64cc86d16000 .. 0x64cc86e43000 — 301 pages
```

300.00 pages *of data*, spread over 301 pages *of address space*. The
difference is the `0x40` at the end of the address: the buffer starts 64
bytes into its first page, so it cannot fit in 300 whole pages — it spills
into a 301st. That is the alignment lesson 007 taught, showing up in the
map.

Now find it. Run the engine and look at its map while it runs (the addresses
differ every run — ASLR puts the mappings somewhere fresh each time, which
is itself part of the point):

```
$ cat /proc/<pid>/maps | head -7
5923fe127000-5923fe128000 r--p 00000000 08:30 397590   .../build/game
5923fe128000-5923fe12b000 r-xp 00001000 08:30 397590   .../build/game
5923fe12b000-5923fe12c000 r--p 00004000 08:30 397590   .../build/game
5923fe12c000-5923fe12d000 r--p 00004000 08:30 397590   .../build/game
5923fe12d000-5923fe12e000 rw-p 00005000 08:30 397590   .../build/game
5923fe12e000-5923fe25a000 rw-p 00000000 00:00 0
59242e643000-59242e664000 rw-p 00000000 00:00 0        [heap]
```

In that run the framebuffer sat at `0x5923fe12d040` — inside the *fifth*
line, the executable's own read-write data page — and its 300 pages of
bytes spilled across into the *sixth*, the anonymous mapping that runs from
`5923fe12e000` to `5923fe25a000` (a span of `0x12C000` = 1,228,800 bytes,
exactly the framebuffer's size). The map is not a picture of "the program":
it is a table of ranges, and one buffer can straddle two rows of it.

Read the columns: **range** (start and end, the end exclusive),
**permissions** (`r--`, `r-x`, `rw-p` — the `p` is "private",
copy-on-write), **offset** (where in the file this mapping starts —
`00000000` for anonymous memory), **device and inode** (the file behind the
mapping, absent when there is none), and **the path** (empty for anonymous
memory, `[heap]`/`[stack]` for the kernel's own labels).

The executable's first five lines are one per segment of one file — note
the `r-xp` line in the middle: that is *code*, mapped read-and-execute,
never write. The protections the map prints are the page-table bits the
deep dive described, and this is what they look like from the outside.

The write-up to aim for: the frame's bytes do not live "in the program" as
a vague fact — they live in one specific range of virtual addresses, backed
by page frames the OS assigned, and the map is the receipt.
