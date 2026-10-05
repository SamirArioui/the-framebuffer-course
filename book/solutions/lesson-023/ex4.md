# Solution: exercise 4 — Why states, not flags

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 023 — the state machine: title, play, death](../../lessons/part-0/lesson-023-state-machine.md).*

## The diff

```diff
{{#include ex4.patch}}
```

## Walkthrough

The patch wraps every state change in a `SetState` that logs the transition,
and the scripted run of exercise 2 prints the machine's whole story:

```
state title -> play
state play -> dead
state dead -> play
state play -> dead
```

Four transitions, each made in exactly one place, each visible. That is the
argument for explicit states. The alternative — `int playing, dead, paused`
flags — *works* at three states and rots at five: every `if` must ask about
every flag, illegal combinations (`playing && dead`) become expressible, and
"what happens when the player presses space?" turns into a table of cases in
someone's head. A single `state` variable makes the legal combinations only
the declared ones, makes transitions enumerable (as the log shows), and makes
the next feature — exercise 3's pause, or a high-score screen — an enum
value and a branch instead of a new flag threaded through old code.

The walk through `Render` shows the shape: one branch per state, each drawing
a complete screen. Compare with the dispatcher idea the log hints at — one
function per state — and lesson 024's command table is the same pattern
applied to input: data instead of branches.
