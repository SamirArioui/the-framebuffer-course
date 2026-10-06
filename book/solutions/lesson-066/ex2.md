# Solution: exercise 2 — Pause and resume

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 066 — music as a loop](../../lessons/part-3/lesson-066-music.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

The diff makes pause real beside `MixerStop`. `MixerPause` halts the
channel where it is — inactive, so the mix stops pulling from it — and
keeps everything: sample, cursor, volume, and the loop flag. Nothing is
thrown away and nothing is rewound. `MixerResume` sets the channel
active again and the next pull takes the frame the pause stopped on.
`MixerStop` is untouched; the two differ in what they mean, not in what
they write.

The probe in `src/main.cpp` runs both on a scratch mixer: the music
plays 300 buffers, pauses for 100, resumes for 50, and the cursor is
read at each stage. The second half is the hazard the prose question is
about: an effect is started, its channel is paused, and a second effect
asks for a channel.

The prediction, before any run. 300 buffers are 220500 frames — one
full pass of the 132300-frame loop and 88200 frames into the second —
so the cursor at the pause is 88200. While paused nothing pulls, so the
cursor stays exactly there, and the 100 paused buffers are silence: the
music channel was the only sound and the mix has nothing to sum. After
the resume, 50 more buffers are 36750 frames, so the cursor reads
124950 — the pause cost the sound nothing but time. Then the hazard:
the paused effect's channel reports `active == false`, and the
first-free walk of `MixerPlay` reads exactly that flag.

The run's log:

```
engine: pause: cursor 88200 at the pause, 88200 while paused, 124950 after 50 more buffers; the paused buffers were silence
engine: pause: the paused effect's channel 1 looked free — the next effect took channel 1
```

Every cursor reconciles: 300 × 735 = 220500 and 220500 − 132300 =
88200; 88200 + 50 × 735 = 124950. And the prose question has its
answer in the second line. **The first-free walk gives a paused effect
channel away.** It asks one question — is this channel active — and a
paused channel answers exactly like a free one. Worse, the new sound
goes through `ChannelPlay`, which starts from the sample's first frame:
the paused sound is not resumed later, it is gone. That is not what a
game wants from a pause button.

What a game wants is a state the allocator can tell apart: a paused
flag beside `active`, with the walk skipping paused channels and the
steal refusing to touch them. The music channel never has the problem —
the walk starts after it — which is why pausing the music here is safe
and pausing an effect is not. The exercise's version is honest about
the difference: `MixerPause` is right for the music channel and wrong
for a pooled effect, and the second line of the log is the wrongness,
measured.

What none of this verifies is the hearing: the paused buffers hold
zero bytes and the resume continues the cursor arithmetic, both
checkable here; what a pause of the music sounds like is yours to
hear.
