# Solution: exercise 1 — One byte, please

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 040 — reservation-backed buffers](../../lessons/part-1/lesson-040-reservations.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

Two predictions to make before running: what does `ReserveMemory(1)` say
the reservation's **size** is, and what are the **first bytes** of memory
nothing has written yet?

```
$ DISPLAY=:99 ./build/game
engine: framebuffer reserved at 0x73ae9a0d4000 — 1228800 bytes = 300.00 pages (page-aligned: yes)
engine: asked for 1 byte: got 4096 bytes at 0x73ae9a618000 — first bytes 0 0 0
```

**Size: 4096.** One byte is one page — the contract said "whole pages", and
a mapping is not a smaller thing than a page. The reservation rounded the
request up, the same arithmetic the memory report does when it counts
pages. (If the rounding surprises you, look at the address: it is
page-aligned too, `...8000` at the end. The OS hands out pages; addresses
are where pages start.)

**First bytes: 0 0 0.** Not "whatever the last program left" — zero.
Anonymous pages are *demand-zero*: the OS does not give the reservation
physical memory at all until the bytes are touched, and when it does, it
gives zeros. The probe in the lesson's prose measured exactly this: the
resident set did not grow when the reservation was taken or when three
bytes were read; it grew by the full 1.2 MB only when every byte was
written.

One byte of request, one page of address space, zero physical frames, three
zero bytes on first read. That is the whole shape of a reservation.
