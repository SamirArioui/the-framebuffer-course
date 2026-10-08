# Solution: exercise 2 — the fade's clock

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 096 — screen polish](../../lessons/part-5/lesson-096-screens.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

**The prediction, written down first.** The death screen's world is
frozen — the state's scale is zero, so the step reads `0.000 ms` on
every frame of the screen. The fade runs on wall time (the lesson's
claim), so it climbs regardless: at the run's paced frames (~43 ms
each) it should gain `0.043` a frame, reach its `0.30 s` in about
seven frames, and land exactly on the screen's color. The two clocks
side by side — one frozen, one running.

**The run** matches, line for line:

```
engine: state play -> death (the hero's health reached zero)
engine: probe: fade 0.000 — step 0.000 ms, state death
engine: screen: death: "GAME OVER" / "SCORE 000424   TIME 0:06   WAVE 1/3" / "ENTER: TITLE"
engine: probe: fade 0.043 — step 0.000 ms, state death
engine: probe: fade 0.086 — step 0.000 ms, state death
engine: probe: fade 0.129 — step 0.000 ms, state death
…
engine: probe: fade 0.259 — step 0.000 ms, state death
engine: screen: death fade arrived at 56,16,16 (its own color)
engine: probe: fade 0.302 — step 0.000 ms, state death
```

`0.000, 0.043, 0.086, 0.129 … 0.259` — exactly the `0.043`-a-frame
climb the paced frames give — against `step 0.000 ms` on every line.
Seven frames after the screen appeared the fade arrives (`fade arrived
at 56,16,16`), and the probe's `0.302` after it is the clock's
overshoot clamped at the target — the value *sits* at the target once
it arrives; it does not wander past it.

**The counterfactual.** If the fade ran on the game's step, every line
would read `fade 0.000`: the death screen's game time is zero — the
same freeze that holds the world holds anything on that clock — and the
screen would hang at black forever, its fade never started. Lesson
078's split, in full:

- **Game time** is for the *simulation* — the world, the waves, the
  sparks, the fights' breaths. It freezes with the pause, slows with a
  hitstop, and that is what makes it right for everything the game
  *is*.
- **Wall time** is for the *machinery and the presentation* — the feel
  hooks (which must *end* while the game is stopped), and now the
  screens' fades (which must *begin* while the game is stopped).

The rule of thumb the pair teaches: ask what must happen while the
world is frozen. If the answer is "it must keep moving" — an effect
ending, a screen arriving — it is wall time. If the answer is "it must
wait for the player" — a wave, a cooldown, a settling spark — it is
game time. Getting this wrong is invisible in a playtest and obvious in
a pause.
