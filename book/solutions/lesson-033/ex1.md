# Solution: exercise 1 — The press count

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 033 — latching brief presses and tracking focus](../../lessons/part-1/lesson-033-latching.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

The latch answers "did it go down?" — *at least once*. Two taps in one
batch light it exactly like one tap does, and anything counting taps (a
double-jump, a menu repeat, a typing rhythm) cannot tell the difference.
The general form is a **count** instead of a flag: `presses[key]` is
incremented on every up→down edge and read back-and-cleared by
`KeyPressCount`. `KeyPressed` becomes what it always was — `count > 0` —
so the old call sites still mean the same thing, and the engine's report
now shows the difference:

```
$ DISPLAY=:99 xdotool key --delay 0 --repeat 2 --window <id> space
engine: polled: -
engine: pressed space x2
engine: polled: -
```

Two taps, one batch, one poll — and the count says two. (Note the same
one-batch situation lesson 032's exercise 2 showed losing the press
entirely: the *state* is still `-`.)

The latch still lights on the **edge**, not on every press event — a held
key auto-repeating is one press and one increment. If you want raw repeat
events (for text input, say), that is a third kind of state with its own
contract: repeats are not presses and presses are not characters.
