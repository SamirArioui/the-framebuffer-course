# Solution: exercise 1 — The presentation check

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 031 — presentation through the platform layer](../../lessons/part-1/lesson-031-present.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

The contract — *the pixels the engine wrote are the pixels the window
shows* — deserves a checker, and the checker belongs on the OS side of the
seam, because reading a window's pixels is an OS operation. `PresentedMatches`
is that: `XGetImage` takes a snapshot of the window the same way
`XPutImage` wrote it — same geometry, same pixel format — and the loop
compares every pixel with the bytes the engine presented. The readback's
memory is Xlib's (unlike the wrapped buffer in `Present`) and is released
with a plain `XDestroyImage`.

The engine checks its own claim once, right after first light:

```
$ DISPLAY=:99 ./build/game
engine: framebuffer 640x480, 1228800 bytes, stride 2560
engine: pixel (0,0) = 255 0 0
engine: pixel (639,479) = 0 255 0
engine: pixel (60,101) = 32 32 64
engine: presented
engine: presentation verified — window matches framebuffer
engine: close reported
engine: closed
```

Every one of the 307,200 pixels matched — including the two we wrote by
hand and the clipped-away one that never landed.

One comparison detail worth noticing: the readback's `XGetPixel` gives a
32-bit pixel value, and only its low three bytes are color — the unused
fourth byte of the seam's format is not part of what the window stores. The
comparison masks it away (`& 0xFFFFFF`), which is exactly the byte lesson
030's exercise 2 identified as the one no window owes us back.

On a real machine this checker is the difference between "I think the window
shows my pixels" and "the window's pixels equal my buffer, byte for byte".
That is what makes the spec's scenario a scenario instead of a promise.
