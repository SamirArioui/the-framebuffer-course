# Solution: exercise 2 — The speed that belongs to the keyboard

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 034 — the first interactive frame](../../lessons/part-1/lesson-034-first-frame.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

Hold an arrow key on your desktop and the marker moves — but *how fast* is
not the engine's answer yet. Frames arrive when news arrives, and while a
key is held the news is auto-repeat: so the marker's speed is your
keyboard's repeat rate, with your keyboard's initial delay. Different
machine, different speed. The engine owns its pixels and not yet its time.

The instrumenting frame counter makes it measurable. Each movement line
carries the frame it happened on:

```
$ DISPLAY=:99 xdotool key --delay 50 --repeat 4 --window <id> Left
engine: arrow keys move the marker; close the window to stop
engine: marker at 308,228
engine: frame 2: marker at 300,228
engine: frame 4: marker at 292,228
engine: frame 6: marker at 284,228
engine: frame 8: marker at 276,228
```

Count the frames between steps: every second frame moved the marker. The
steps are eight pixels each and they land when the polled state says a key
is down — the *distance* is the engine's (`MARKER_STEP`), the *rate* is the
news. Hold the key for one second on your machine and count the steps:
they are your auto-repeat rate, minus whatever your window manager ate.

What must change for the engine to own its speed is exactly what the next
two lessons build: a **clock**. When a frame knows how long it took, the
step becomes `speed × elapsed` and the movement is pixels-per-second no
matter who wakes the loop — lesson 035 gives the engine that clock, and
lesson 036 makes the frame's cost visible while it is there.
