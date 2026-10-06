# Solution: exercise 2 — The press that vanished

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 032 — polled input state](../../lessons/part-1/lesson-032-polled-input.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

The prediction to make first: one gesture — press and release with no
waiting — produces two events, and the engine polls *after* the pump drains
the batch. So the poll sees the state as of the end of the batch: the key is
up again. The press and the release cancel before any poll happens.

With the fold instrumented, both events are visible — and so is their
consequence:

```
$ DISPLAY=:99 xdotool key --delay 0 --window <id> space
platform: fold press   keysym=32
platform: fold release keysym=32
engine: presented
engine: polled: -
engine: polled: -
```

(`--delay 0` is what puts both events into one pump batch; the default
delay gives the engine time to poll between them, and then the press shows
up normally — timing decides what the state looks like, which is the whole
problem.)

The two fold lines prove the events arrived; the two `-` polls prove the
contract lost them. The polled state answers "what is down *now*", and a
press that is already over is not down now. Nothing is broken in the fold —
both assignments ran — the *state* is simply not the right shape for news
that has already ended.

What the engine needs is a second piece of state alongside the current one:
not just "is down now" but "went down at least once since the last poll" —
a latch that sticks until the engine has seen it. That is exactly what
lesson 033 builds, and this vanishing press is the reason it exists.
