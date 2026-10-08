# Solution: exercise 1 — what the profile cannot see

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 098 — pass 1: measure](../../lessons/part-5/lesson-098-measure.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

The probe is one line and one sum: `ReportSeam` (in `report.*`, beside
the run's other reports, called just before the frame-budget table)
splits the frame's wall time where the seam splits the work — the
engine's phases (`update + audio + render`, what the process computed)
against the present (what the process spent waiting for the display to
take its pixels). From a real run of the patched build:

```
engine: seam: 777 frames — engine work 1.323 ms/frame (update+audio+render), seam wait 0.425 ms/frame (present) — the flat profile sees the first, not the second
engine: frame budget — 777 frames, avg 1.748 ms, worst 2.994 ms (frame 489)
```

The two buckets add up exactly — `1.323 + 0.425 = 1.748`, the table's
own total — because the frame has no third place for time to go.

Now the answers, with that run's numbers. **The profiler is blind to
`0.425 ms` a frame — 24% of the frame's wall time** — the seam's wait.
And the flat profile's percentages describe the **CPU, not the frame**:
`BlitSprite`'s `59.77%` is 59.77% of the profiled process time (3.53 s
over the run), while as a share of the frame's wall the map's draw is
`0.981 / 1.944 ≈ 50%` of a play frame. The two views agree on the
names and the order but not on the arithmetic, and the reason is
exactly this split: `platform::Present` reads `0.28%` of CPU and `23%`
of the frame.

The practical rule, worth keeping beside every profile you ever take:
**a profile's shares are shares of the process's work; a frame's cost
is the process's work plus its waits.** When the seam is cheap (a
local display, a fast copy) the two converge; when it is dear (this
machine's X server round trip) a quarter of the frame is simply outside
the picture the profiler paints. That is why the lesson names the
hotspots from both instruments — and why `present` went on the
future-work list measured, not guessed.
