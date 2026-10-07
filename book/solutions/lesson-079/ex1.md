# Solution: exercise 1 — The paused frame, predicted

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 079 — measurement is not scaled](../../lessons/part-4/lesson-079-wall-clock.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

The diff is two labeled snapshots of the same frame-budget table: one
printed the moment the knob turns to pause — the table as it stands for
the play and hitstop frames — and one at the end of the run, after the
paused frames have joined the account. `PrintFrameBudget` is
cumulative, so the two tables differ exactly by the frames between
them.

The prediction, before any run. **The paused frame's log line** reads
`step 0.000 ms` first — the simulation did not advance — and then
ordinary phases: `update` a few hundredths of a millisecond (the input
read, the walk), `render` about 1.3 ms (the whole scene still drawn),
`present` about 0.3-0.5 ms (the copy to the window), `total` their sum.
Nothing in the line is zero except the step. **The budget table after a
paused run** compared with a playing one: the *rows* are the same
picture — `render` around 1.36 ms, `tilemap` around 0.94 ms, `present`
around 0.5 ms, `update` around 0.01 ms — because a paused frame does the
same work. The numbers that move are the bookkeeping ones: the frame
count grows, and the average moves by whatever the paused frames'
average is (the same as the rest). No row collapses toward zero.

The runs, from this lesson's end state plus the patch — one run, the
knob turning at three, five, and seven seconds:

```
engine: budget before the pause:
engine: frame budget — 49 frames, avg 1.874 ms, worst 3.078 ms (frame 1)
engine:   subsystem   avg ms    share
engine:   update       0.008       0%
engine:   audio        0.000       0%
engine:   render       1.364      73%
engine:     sprites    0.006       0%
engine:     text       0.007       0%
engine:     tilemap    0.942      50%
engine:   present      0.501      27%
...
engine: budget at the end of the run:
engine: frame budget — 145 frames, avg 1.899 ms, worst 4.291 ms (frame 132)
engine:   subsystem   avg ms    share
engine:   update       0.013       1%
engine:   audio        0.000       0%
engine:   render       1.364      72%
engine:     sprites    0.006       0%
engine:     text       0.006       0%
engine:     tilemap    0.938      49%
engine:   present      0.522      27%
```

Exactly as predicted: `render` is 1.364 ms in *both* tables — the same
number to the digit, because the paused frames drew the same scene —
and `tilemap`, `sprites`, `text`, `present` are all within noise of
their pre-pause values. The average moved from 1.874 to 1.899 ms across
96 more frames, of which 32 were paused.

The one sentence the exercise asks for: **the paused run's table proves
that "the game stopped" and "the machine stopped" are different
statements** — the game's step was zero for 32 of those frames and the
record priced them like any others, which is the only way the budget
can tell you what a pause screen actually costs. A scaled record would
have shown a pause as a gift of free frames; a wall-clock record shows
it as the honest thing — the presentation running, the world waiting.

One detail worth noticing: `worst 4.291 ms (frame 132)` is a *paused*
frame's neighbor — the worst frame of a run can be anywhere, and the
record's `worst_number` is what lets you go look at its line in the log
and see the scale it ran at. That is the step field's other job: making
each line self-describing.

Nothing here touches the record, the scale, or the budget's arithmetic:
the patch is two labeled prints of the table the lesson already had.
