# Solution: exercise 4 — The pixel that is not there

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 013 — raw bytes and pixel formats](../../lessons/part-0/lesson-013-raw-bytes.md).*

## The diff

```diff
{{#include ex4.patch}}
```

## Walkthrough

`(0, 6)` computes offset `6 * 24 = 144` — exactly one past a 144-byte
buffer. Without the guard the program "works": the write lands in heap
padding and `HexDump` never sees it. Under `-fsanitize=address` it cannot
hide:

```
==98527==ERROR: AddressSanitizer: heap-buffer-overflow on address ...
WRITE of size 1 at ... thread T0
```

AddressSanitizer aborts on the first poisoned byte touched, with a stack
trace naming `PutPixel` and `main` — far better than a corrupt image three
functions later. The fix makes the function police its own contract: a
coordinate outside `w × h` is refused with a message on `stderr` and no
write, and the same run is clean under AddressSanitizer. `GetPixel`
deserves the identical guard — an out-of-range read is just as undefined.
One caveat to remember: the guard turns silent corruption into a complaint,
but it costs a branch per pixel; real engines keep raw writers and clip at
the call sites — the pattern lesson 015 makes explicit.
