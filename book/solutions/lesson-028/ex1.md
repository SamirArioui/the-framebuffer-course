# Solution: exercise 1 — Resize is news too

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 028 — the event pump](../../lessons/part-1/lesson-028-event-pump.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

The pump is a switch over news, and a resize is one more case. The OS reports
the window's new geometry in a `ConfigureNotify` event carrying its new width
and height, and the fold is two assignments into the window's state. The
state side of the seam grows one query, `WindowSize`, and the engine reads it
before closing.

The engine's state starts where the window started (`OpenWindow` records the
size it requested), so the answer is always meaningful — before any resize
news it is simply the size the engine asked for.

Verified under the headless check: ask the OS for a different size, then
close:

```
$ DISPLAY=:99 xdotool windowsize 2097153 800 600
$ DISPLAY=:99 xdotool windowclose 2097153
engine: window 640x480 open — waiting for news
engine: close reported
engine: closed at 800x600
```

The engine never saw an event object — it polled the platform layer's state
at the end and found the size the OS last reported. That is the polled-state
contract of lesson 032 arriving early: news goes *into* state, the engine
reads *state*.

One boundary to keep: `WindowSize` returns plain `int`s, not an X geometry
structure, not a display handle — the seam stays free of OS idioms even in
the new function. A second OS reports its own window's size and the engine's
call does not change.
