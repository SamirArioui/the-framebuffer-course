# Solution: exercise 2 — What the mix costs

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 064 — the mix](../../lessons/part-3/lesson-064-mix.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

The diff is one probe: `MixBuffer` timed on its own, on a scratch
mixer, at 0, 1, 2, 4, 8, and 16 active channels. Two details make the
numbers mean what they say. The mixer is **re-armed before every
trial** — `MixerInit`, then one `ChannelPlay` per channel — so every
timed buffer has exactly the advertised channels playing. And a trial
is 29 buffers, one short of the tone's thirty, so no channel runs out
of sample mid-trial: without these two moves "16 active" would decay
into "16 fading to silence" and the measurement would drift as the
trial ran. Each configuration totals 5800 buffers — 200 trials × 29 —
so the clock's own tick is inside the number, not beside it.

The measurement, one run on this machine:

```
engine: mix cost:  0 channels — 0.0206 ms per 735-frame buffer, 0.0280 us per output frame
engine: mix cost:  1 channels — 0.0224 ms per 735-frame buffer, 0.0305 us per output frame
engine: mix cost:  2 channels — 0.0241 ms per 735-frame buffer, 0.0328 us per output frame
engine: mix cost:  4 channels — 0.0255 ms per 735-frame buffer, 0.0346 us per output frame
engine: mix cost:  8 channels — 0.0349 ms per 735-frame buffer, 0.0475 us per output frame
engine: mix cost: 16 channels — 0.0343 ms per 735-frame buffer, 0.0466 us per output frame
```

What the numbers show, in the order the exercise asked.

**Against the frame budget.** The worst configuration — all sixteen
channels playing — costs 0.0343 ms of a 16.7 ms frame: 0.2% of the
budget, 0.047 µs per output frame. Even a full pool mixing at full
rate cannot starve the loop; the mix is not where this engine's frame
time goes.

**What grows and what does not.** The fixed part is the walk over the
pool: at zero active channels the mix still costs 0.0206 ms per buffer
— sixteen pulls per output frame, 11760 calls to `ChannelFrame` per
buffer, nearly all of them returning nothing. That cost is the pool's
size and does not move when channels start playing. The active part is
the rest: from zero to sixteen channels the buffer grows by 0.0137 ms,
about 0.8 µs per added channel per buffer — the sample read, the
fixed-point scaling, the cursor. The growth is real but small enough
that neighboring counts are inside the run-to-run noise: a second run
on this machine gave 0.0202 / 0.0218 / 0.0237 / 0.0278 / 0.0366 /
0.0346 — the same shape, and 8 and 16 again trading places.

**Against the frame log.** The run's own record agrees with the probe.
This frame, from the same run as the numbers above:

```
frame 1: update 0.001 ms, audio 0.033 ms, render 1.567 ms (sprites 0.001, text 0.006, tilemap 0.872), present 1.190 ms, total 2.791 ms
```

The audio phase at a feed lands around 0.025–0.035 ms on this machine —
the probe's 0.0241 ms of mixing at the demo's two active channels plus
the submit. The occasional 0.2–0.4 ms audio line is not the mix: the
probe bounds the mix at 0.035 ms even with a full pool, so the rest is
the submit path and whatever the scheduler did that frame.

**What the probe cannot hide about itself.** The absolute numbers are
this build's — `-O0 -g`, the course's default flags — and a different
optimization level moves them. The sample is one 44 KB buffer read
over and over; it is cache-hot in a way a game's sample set is not. The
timing covers `MixBuffer` alone — no device, no submission, no loop
around it — so it prices the mix, not the sound path. And it is one
machine: the second run above is the honest error bar, a microsecond or
two per buffer.

The lesson's shape is what survives all of that: the pool's walk is
paid whether or not anything plays, the channels add a little each,
and the total is small against the frame — which is why lesson 070 can
put the audio row in the frame-budget table without a budget fight.
