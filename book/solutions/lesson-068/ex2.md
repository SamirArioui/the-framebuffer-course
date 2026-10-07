# Solution: exercise 2 — The music that ducks

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 068 — music and effects together](../../lessons/part-3/lesson-068-together.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

The diff makes ducking real on a scratch mixer: the music looping on
its channel, one effect fired over it, and the music channel's volume
dropped for the effect's moment and climbed back afterwards. There is
no new mixer concept in it. A duck is **per-sound volume applied to the
music channel over time** — the same `volume` field every channel has,
the same fixed-point scale, written and rewritten from the run's
script. `MixerStop`, the routes, and the mix are untouched.

The shape of one duck: at the effect's fire the volume drops to half,
holds there for ten buffers — the floor — then climbs back over ten
more, each step a fixed-point division that lands exactly on
`AUDIO_VOLUME_FULL`. The run's log, one line per buffer:

```
engine: duck: buffer  0: music volume 128 of 256 — the drop
engine: duck: buffer  1: music volume 128 of 256
...
engine: duck: buffer  9: music volume 128 of 256
engine: duck: buffer 10: music volume 140 of 256 — the climb begins
engine: duck: buffer 11: music volume 153 of 256
engine: duck: buffer 12: music volume 166 of 256
engine: duck: buffer 13: music volume 179 of 256
engine: duck: buffer 14: music volume 192 of 256
engine: duck: buffer 15: music volume 204 of 256
engine: duck: buffer 16: music volume 217 of 256
engine: duck: buffer 17: music volume 230 of 256
engine: duck: buffer 18: music volume 243 of 256
engine: duck: buffer 19: music volume 256 of 256 — back where it was
engine: duck: buffer 20: music volume 256 of 256
engine: duck: the music's frame 1: 277 at full volume, 138 at the duck's floor
```

The floor is `AUDIO_VOLUME_FULL / 2` = 128 and the last line is what
the drop means in bytes: the music's frame 1, 277 at full scale, is 138
at the floor — the same sample, the same cursor, half the amplitude.
The mix needs no explanation for any of it: it pulls the channel's next
frame and scales it by the channel's volume, exactly as it does when
nothing is ducked.

Two details where this exercise earns its keep.

**The climb must land on full and stay there.** The naive step — add
the same increment past the end of the climb — runs the volume past
`AUDIO_VOLUME_FULL` on the buffer after the climb: a channel that
*amplifies* its sample, breaking the volume contract (0 to
`AUDIO_VOLUME_FULL`) that keeps lesson 063's promise — a channel emits
at most the sample it plays. The last step lands on 256 exactly because
the step is computed as a fraction of the remaining distance, and the
climb stops at the buffer where it completes.

**Ducking is the effect's volume, moved to the music.** The point is
audibility: the effect is what just happened and the music is the
constant under it, so for the effect's moment the constant steps aside.
The duck's depth and length are game design; the arithmetic is the
volume scale the course has used since lesson 063. Nothing about the
mix knows a duck is happening.

What the probe cannot settle is whether ten buffers is the right
length or half the right depth — those are heard, not computed. The
volumes and the frames are exact here; the sound of the duck is yours
to hear.
