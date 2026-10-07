# Solution: exercise 1 — Feedback starts with the event

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 086 — feedback and animation](../../lessons/part-5/lesson-086-feedback-animation.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

The game-feel rule is that a feel effect begins **in the frame its
triggering event happens** — the hit and its weight are one moment, not
a hit now and a shake a moment later. So the change is *what fires the
hooks*, not the hooks themselves: the hit handler that was already
lowering the hero's health now also fires a short hitstop and a small
screenshake, right there.

```cpp
hero.health -= 1;
FeelHitstop(feel, 0.25, 0.1);
FeelShake(feel, 4.0, 0.2);
```

`GameInput` gains a `Feedback &feel` so the hit can reach the hooks — the
game's input already knows about the event; it just needs the hooks to
tell. The hooks' own fire-and-rest is untouched: they still run down
their own wall-time and return to full speed and to `0,0` on their own.
The lesson's wall-time demonstration script is now redundant — the
toolkit fires the hooks from events, not a clock — so drop it (or leave
it and watch both paths fire).

Run it and press Space: the run reads

```
engine: hero takes a hit — health 2 (t=2.595)
engine: feel: shake fired ...   <- (with the demonstration removed, only the hit's firing)
engine: feel: hitstop rested — full speed again
engine: feel: shake rested at 0,0
```

The hit and the feedback are on the same frame — the slowdown and the
shake start with the hit's own `hero takes a hit` line, not after it.
That is the whole rule: cause and effect read as one moment because they
*are* one frame. (The hit here is still the Space stand-in; when combat
lands in lesson 087, the same two lines move from the key to the real hit
and the rule holds unchanged.)
