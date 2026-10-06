# Solution: exercise 2 — The effect that must not stack

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 067 — effects as one-shots](../../lessons/part-3/lesson-067-effects.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

The diff makes the solo route real beside the normal one.
`MixerPlayEffectSolo` walks the pool's effect channels first, looking
for the same sample **already sounding** — the sample's identity is its
address, and the engine keeps one copy of each sample in the arena, so
the pointer comparison is the whole check. If one is found, the call
answers `AUDIO_ALREADY_PLAYING`, a name for the refusal instead of a
silent `-1`; if not, it defers to `MixerPlayEffect` unchanged — same
pool, same volumes, same steal policy. The gate is one walk; the route
is still the lesson's.

The probe drives both routes with the same three presses on their own
mixers, so the difference is the only variable. Under the solo route,
press 1 takes the first free channel; presses 2 and 3, arriving while
the effect is still sounding, are refused by name. Then thirteen
buffers run — twelve for the effect's length, one for the pull that
ends it — and press 4 starts again on the first free channel. Under
`MixerPlayEffect`, the same three presses stack: three copies in flight
at once, on three channels.

The run's log:

```
engine: solo: press 1 -> channel 1
engine: solo: press 2 refused — the sound is already playing
engine: solo: press 3 refused — the sound is already playing
engine: solo: press 4, after the effect's end -> channel 1
engine: stack: press 1 -> channel 1
engine: stack: press 2 -> channel 2
engine: stack: press 3 -> channel 3
```

Two lines carry the lesson's own facts back. Press 4 lands on channel 1
— the pool returned the first effect's channel, exactly as the lesson's
run showed — and the refusals cost nothing: the pool is untouched by a
refused call, so the next sound still finds a free channel.

Then the design question the two trials frame. Stacking is not a bug
by itself: three footstep sounds in a row *should* overlap, and the
stack route is right for them. The solo route is for the sounds that
are one event — a door slamming once even if the game asked three
times, a jump that sounds like one jump. Which route a sound takes is
a fact about the sound, not about the mixer, and that is why both live
beside each other and the choice stays in the caller's hand.

What the probe cannot settle: whether a triple-fired slam sounds wrong
is an ear's judgment. The channels and the refusals are exact here;
the stacking's sound is yours to hear.
