# Solution: exercise 2 — The four bytes

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 030 — the framebuffer as our own bytes](../../lessons/part-1/lesson-030-framebuffer.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

Two instrumenting lines read the buffer the way lesson 013 read image files:
raw, at known offsets. The prediction to make before running is what four
bytes hold a red pixel and what four hold the background.

```
$ DISPLAY=:99 ./build/game
engine: framebuffer 640x480, 1228800 bytes, stride 2560
engine: pixel (0,0) = 255 0 0
engine: pixel (639,479) = 0 255 0
engine: pixel (60,101) = 32 32 64
engine: bytes at (0,0): 00 00 ff 00
engine: bytes at (1,0): 40 20 20 00
engine: presented
```

Pixel (0,0) is red and its bytes read `00 00 ff 00`: **blue first, green
second, red third, one unused byte** — `PutPixel` writes `p[0]=b, p[1]=g,
p[2]=r`, and the dump is the machine confirming the order instead of the
name. The background at (1,0) reads `40 20 20 00` — `0x40` is 64 (blue),
`0x20` is 32 twice (green and red) — same order, different color.

Why this order and not red-green-blue? Because the format is the *seam's
contract*, chosen to be what the window system on the reference machine
carries natively: on little-endian x86-64 an X11 `ZPixmap` at this depth is
exactly these four bytes per pixel, so `XPutImage` copies our buffer across
without translating a single byte. A second OS implementation is free to
translate on the way out — the contract is in `platform.h`, not in X11.

And why four bytes when three hold color? Lesson 007's alignment, now paying
off at buffer scale: 4 bytes per pixel means every row of 640 pixels is 2560
bytes — a whole number of 8-byte words — so no row boundary ever straddles a
machine word. The unused byte is the cheapest possible stride.
