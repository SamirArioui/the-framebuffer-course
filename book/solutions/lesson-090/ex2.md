# Solution: exercise 2 — the schedule's timeline

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 090 — the boss](../../lessons/part-5/lesson-090-boss.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

**The prediction.** The pattern is chase (3 s) → keep (2 s) → flee
(1 s) → repeat, starting at play, on the game clock. So from the start
of play:

| Game time | Phase |
| --------- | ----- |
| 0 – 3 s | chase |
| 3 – 5 s | keep |
| 5 – 6 s | flee |
| 6 – 9 s | chase |
| 9 – 11 s | keep |
| 11 – 12 s | flee |
| 12 – 15 s | chase |

The phase *reports* fire when a phase begins, so the first fifteen
seconds of play should announce `keep` at t≈3, `flee` at t≈5, `chase`
at t≈6, `keep` at t≈9, `flee` at t≈11, `chase` at t≈12 — the chase
phases mostly silent (the pattern started on one), the keep and flee
phases announcing themselves every six seconds of cycle.

The distance curve over one full cycle: **falling** during the chase
(the boss closes on the hero), **rising to the keep distance** during
the keep (it backs off to its 160 and holds), **rising faster** during
the flee — then falling again. Sawtooth, with a flat top.

**The probe stamps the timeline** with the pattern's own game clock
(the `t=` here is game time the pattern has seen, not wall time):

```
engine: boss: golem's pattern -> keep (2 s) at t=3.0
engine: boss: golem's pattern -> flee (1 s) at t=5.0
engine: boss: golem's pattern -> chase (3 s) at t=6.1
engine: boss: golem's pattern -> keep (2 s) at t=9.1
```

`keep at 3.0` — the first chase ran its full 3 s. `flee at 5.0` — the
keep ran its 2. `chase at 6.1` — the flee's 1 s (the 0.1 is the frame
the change lands in: the schedule notices when a frame's step crosses
the line, so events land on frame boundaries). `keep at 9.1` — the
chase's 3 s again. The prediction's table matches, jittered by exactly
one frame where it crosses.

**The long-frame caveat** is why the pattern runs on game time and
counts *up* through it: the headless run's first frame can carry a step
of seconds (this run's did), and a countdown-style schedule initialized
to zero would have skipped its first phase or advanced at the wrong
moment. Counting the elapsed phase time against the phase's length
makes the schedule a function of game time alone — the same game time
every other part of the simulation uses — and the freeze freezes it
too.
