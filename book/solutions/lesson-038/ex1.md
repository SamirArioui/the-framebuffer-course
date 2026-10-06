# Solution: exercise 1 — The screenshot

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 038 — whole-file writes and round-trips](../../lessons/part-1/lesson-038-file-write.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

Lesson 017 wrote an image file by hand in `paint` — the engine can now do
the same thing with its own pixels and its own seam. `SaveScreenshot`
builds a PPM file in memory: a three-line header (`P6`, the dimensions, the
max value) followed by one RGB triple per pixel. The conversion from the
framebuffer's blue-green-red-x bytes to the file's red-green-blue bytes is
lesson 013's byte-order knowledge doing real work — the seam's format is
for the window, the file's format is for the file, and the engine knows
both.

One detail worth noticing in the patch: the screenshot renders the scene
*before* saving — clear, marker, then write. The framebuffer at startup is
whatever the static array holds (zeros); the pixels exist once the render
writes them. A screenshot is of a *frame*, not of a buffer.

Verified — the run writes the file, and reading it back with any PPM
reader finds exactly the scene the engine drew:

```
$ DISPLAY=:99 ./build/game /tmp/opencode/xcheck/rt.bin /tmp/opencode/xcheck/shot.ppm
engine: round-trip ok: 256 bytes match
engine: saved screenshot to /tmp/opencode/xcheck/shot.ppm (640x480 PPM)
```

```
ppm header: b'P6\n640 480\n255\n'
payload bytes: 921600
pixel (308,228): (240, 220, 80)   # the marker's yellow, top-left corner
pixel (10,10):   (32, 32, 64)     # the background's blue-gray
```

921,600 bytes is exactly `640 × 480 × 3` — header plus one RGB triple per
pixel, and the pixel at the marker's reported position is the marker's
color. The engine's pixels, written by the engine, readable by anything
that reads PPM. That is what "whole-file writes" are for.
