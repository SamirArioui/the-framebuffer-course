# Solution: exercise 2 — The acceptance table

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 043 — the closing demo](../../lessons/part-1/lesson-043-demo.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

"Platform layer done" is a claim, and a claim wants a table. The
instrument's first column is the contract in use, named at startup:

```
engine: platform layer done — window, polled input, arena-backed framebuffer, measured frames
engine: contract: window + presentation (lessons 027-031)
engine: contract: polled input (lessons 032-034)
engine: contract: monotonic clock (lessons 035-036)
engine: contract: whole-file I/O (lessons 037-038)
engine: contract: reservations + arena (lessons 040-041)
```

The rest of the table is yours to fill from the spec's requirements and
your own runs. The shape to aim for:

| Spec requirement | Scenario | Evidence from the run |
| ---------------- | -------- | --------------------- |
| Window and presentation | window opens at the requested size | the window is 640×480 (`xdotool getwindowgeometry`) |
| | pixels presented unchanged | the marker's pixels read back at the marker's reported position |
| | close reported, resources released | `close reported`, `closed`, window gone after exit |
| Polled input | polling reports current state | the marker moves only while keys are down |
| | held keys stay held | one hold, many steps |
| | brief presses not lost | lesson 033's latch report |
| Monotonic frame timing | readings never go backwards | the startup check over 100,000 samples |
| | frame durations resolvable | the frame log's millisecond figures |
| Whole-file I/O | successful read / failure is a value | lesson 037's runs |
| | write round-trips | lesson 038's `round-trip ok` |
| Single OS boundary | engine code is OS-free | `check-boundary.sh` passing |
| | a second OS slots in | the stub's link line (lesson 042's exercise 1) |

Every row has a run behind it — that is what makes the table an
acceptance table and not a wish list. Where a row is missing evidence, the
demo is not done: find the run that settles it.

The write-up's last question is the one worth keeping: which rows would
break first if the engine changed? The presentation row breaks when the
renderer changes the pixel format; the boundary row breaks the first time
someone includes an OS header in `main.cpp`; the timing row breaks when
Part 5's optimization passes start moving frame costs around. The table is
not paperwork — it is the list of things that must stay true while the
engine grows.
