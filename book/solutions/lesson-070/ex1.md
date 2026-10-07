# Solution: exercise 1 — The row's two insides

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 070 — the mix's cost in the frame budget](../../lessons/part-3/lesson-070-audio-row.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

The patch applies Part 2's named-phase discipline to the sound: the
frame record grows `mix` and `submit` inside `audio`, the account sums
them like every other field, the `frame N:` line carries them the way
it carries `sprites`, `text`, and `tilemap`, and the table prints them
as indented rows under the phase they explain — inside it, never
instead of it. The run's table:

```
engine: frame budget — 466 frames, avg 2.167 ms, worst 4.030 ms (frame 409)
engine:   subsystem   avg ms    share
engine:   update       0.001       0%
engine:   audio        0.039       2%
engine:     mix        0.032       1%
engine:     submit     0.006       0%
engine:   render       1.390      64%
engine:     sprites    0.001       0%
engine:     text       0.007       0%
engine:     tilemap    0.963      44%
engine:   present      0.737      34%
engine:   total        2.167     100%
```

The log's own lines carry the split per frame:

```
frame 1: update 0.000 ms, audio 0.032 ms (mix 0.023, submit 0.007), render 1.488 ms (sprites 0.001, text 0.005, tilemap 0.851), present 1.365 ms, total 2.886 ms
frame 2: update 0.000 ms, audio 0.030 ms (mix 0.025, submit 0.005), render 1.249 ms (sprites 0.001, text 0.007, tilemap 0.868), present 0.512 ms, total 1.792 ms
```

Reconcile the two rows against the one they live under, the same
cross-check the table gets: over the run's 466 frames the log averages
`audio 0.0387`, `mix 0.0320`, `submit 0.0063` — the insides sum to
0.0383, and the 0.0004 ms difference is the phase's own work outside
the two calls: the rhythm's effect firing, the wrap watch, the feeding
schedule's bookkeeping, and the clock reads themselves. That is exactly
the shape of the render family: the named rows explain the phase; they
do not replace it, and the residue is named work, not error.

Now the row's question. The `submit` inside reads 0.006 ms here — not
zero: even `null` costs a call into the device's library to take the
samples and drop them. On real hardware this is the row that grows: a
submit can wait for room in the device's buffer, and when it does the
wait lands here, not in the mix. Its structural twin is `present` —
both rows are the engine handing bytes to the OS and both carry the
hand-off's sync. `present` waits on the display's copy; `submit` waits
on the sound device's ring buffer. The two rows are the frame's two
places where the machine, not the engine, decides how long the phase
takes — which is why both belong to the port exercise's questions.
