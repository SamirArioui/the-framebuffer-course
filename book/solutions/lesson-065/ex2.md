# Solution: exercise 2 — The policy that refuses

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 065 — channel allocation](../../lessons/part-3/lesson-065-allocation.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

The diff is the rejected policy made real. `MixerTryPlay` in
`src/audio.cpp` walks the pool exactly as `MixerPlay` does — the first
free effect channel, the same `ChannelPlay`, the same `started` stamp
— and stops one step earlier: a full pool returns `AUDIO_NO_CHANNEL`
instead of reaching for the oldest. `MixerPlay` itself is untouched, so
the two policies sit side by side and can be driven in one run. The
demo in `src/main.cpp` does the driving: the lesson's own sixteen-effect
burst, unchanged, and the same burst again on its own mixer through
`MixerTryPlay`.

The run's log, both policies in it:

```
engine: mix: effect 15 -> channel 15
engine: mix: effect 16 -> channel  1 (the oldest was stolen)
engine: mix: music channel 0 is reserved and was never stolen
engine: refuse: effect  1 -> channel  1
engine: refuse: effect  2 -> channel  2
...
engine: refuse: effect 15 -> channel 15
engine: refuse: effect 16 -> refused (every effect channel busy)
```

The sixteenth press is where the policies part, and the report is two
sentences. Under the steal, the sixteenth sound plays — it takes
channel 1 and effect 1 stops at whatever frame it had reached. Under
the refusal, the sixteenth sound does not happen at all — the pool
answers by name and effects 1 to 15 keep playing to their ends. Both
answers are legal; they differ in *who pays* for the busy second. The
steal makes the newest sound pay with the oldest one; the refusal makes
the player pay with nothing.

Nothing else in the run moves: the end report is still 30 buffers of
sound — the refusing burst talks to its own mixer and is never mixed
into the stream — and the music channel is as reserved under one policy
as the other.

Which to ship is the exercise's real question, and the lesson's answer
holds up: for a game's sound effects, steal. A pressed button should
answer with a sound, the busy second is exactly when feedback matters,
and the sound that gives way is the one already heard the most. The
refusal is not nonsense — it is the right policy where every sound is
load-bearing and hearing it whole beats hearing it at all: a cue the
player must catch is better missed than cut, and a missed cue can be
asked again. That is the trade `MixerTryPlay` prices: it never cuts
anything, and it is the only one of the two that can answer a press
with silence.

What the probe cannot hide about itself: no speaker made a sound —
`null` discards the samples — so "what the player experiences" is
argued, not heard. And the two walks are copies of each other, not one
walk with a policy switch; the exercise asked for the policy beside
`MixerPlay`, and leaving the shared shape implicit is what keeps the
comparison honest.
