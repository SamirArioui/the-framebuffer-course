# Solution: exercise 2 — The restart that wasn't

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 023 — the state machine: title, play, death](../../lessons/part-0/lesson-023-state-machine.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

Reproduce with a producer that presses space, steers up, and presses space
again only after death — the second key must arrive *while the game is
dead*, so it needs real time between the bytes:

```
( printf ' \033[A'; sleep 3.5; printf ' ' ) | ./snek 150
```

With the bug, the trace shows the first death at tick 9 (`dir=up at=2,20`),
a restart at frame 106 — and the revived snake immediately climbs the same
column and dies at the top wall again at tick 43. `StartGame` reset the body,
the score, and the food, but `dir` is ordinary state like any other and it
still held the direction of the *last life*. The death screen's promise, "press
space to play again", is only true if every field is reset.

The fix adds `dir = DIR_RIGHT;` beside the other resets. Rerun and the restart
at frame 106 reports `state=play … dir=right at=10,21`, and frame 150 finds
the snake still alive at `at=10,36`. The general rule is worth a sentence in
any game's `StartGame`: list what the game *is* — position, direction, score,
timers, mode — and set all of it, every time. State machines make the states
explicit; they do not make stale state impossible.
