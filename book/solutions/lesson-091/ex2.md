# Solution: exercise 2 — the wave plan

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 091 — waves](../../lessons/part-5/lesson-091-waves.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

**The prediction, from the rows alone.** The roster says:

| Kind | `wave` | `count` |
| ---- | ------ | ------- |
| bat | 1 | 2 |
| wisp | 2 | 1 |
| spitter | 2 | 2 |
| golem | 3 | 1 |

and a wave spawns every kind whose `wave` has come — that wave and
every wave after it. So:

- **Wave 1** brings the bats: **2 enemies**.
- **Wave 2** brings the bats again, plus the wisp and the spitters:
  2 + 1 + 2 = **5 enemies**.
- **Wave 3** brings everything, including the golem: 2 + 1 + 2 + 1 =
  **6 enemies** — the three types and the boss on the field together.
  That is the wave the game is built around: the last stage is the
  whole roster at once, and the boss is not a solo encounter but the
  cap of a battlefield that is already full.

The probe prints exactly that plan before anything spawns:

```
engine: plan: wave 1 brings bat x2
engine: plan: wave 1 total 2
engine: plan: wave 2 brings bat x2
engine: plan: wave 2 brings wisp x1
engine: plan: wave 2 brings spitter x2
engine: plan: wave 2 total 5
engine: plan: wave 3 brings bat x2
engine: plan: wave 3 brings wisp x1
engine: plan: wave 3 brings spitter x2
engine: plan: wave 3 brings golem x1
engine: plan: wave 3 total 6
```

and the lesson's runs spawn precisely those: `wave 1 begins — 2
enemies`, `wave 2 begins — 5 enemies`, `wave 3 begins — 6 enemies`.

**Move one row.** Change the golem's `wave` from `3` to `1` and the
plan inverts: wave 1 becomes the boss *and* its escort (3 enemies:
golem ×1, bat ×2), and wave 3 is just the bats and the wisp and the
spitters — the climax arrives first. That is the whole point of the
composition living in the rows: the shape of the contest is a
diffable, editable fact, and the code that fights it never changes.
(The probe above is throwaway measurement — the real fight prints the
spawns, not the plan.)
