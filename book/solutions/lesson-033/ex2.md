# Solution: exercise 2 — The key that will not let go

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 033 — latching brief presses and tracking focus](../../lessons/part-1/lesson-033-latching.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

The prediction first: alt-tab away from a game while holding the movement
key, and without focus handling the marker keeps moving — forever. The
release happens in *another* window; our window never sees it, so
`keys[KEY_LEFT]` stays true until something else changes it. A stuck key is
the classic symptom, and the cause is always the same: **key state without
focus awareness**.

With the instrumenting print applied, the fold shows the moment that would
produce it. Hold `left`, then take focus away:

```
platform: focus out — dropping held keys
platform: focus in
platform: focus out — dropping held keys
engine: framebuffer 640x480, 1228800 bytes, stride 2560
engine: pixel (0,0) = 255 0 0
engine: pixel (639,479) = 0 255 0
engine: pixel (60,101) = 32 32 64
engine: presented
engine: polled: -
engine: focus gained
engine: polled: left
engine: pressed left
engine: polled: -
engine: focus lost
engine: polled: -
```

The key was held (`polled: left`), focus left, and the very next poll is
empty: the platform layer dropped every held key on `FocusOut`. The release
that arrived while unfocused changed nothing — by then there was nothing to
release. Refocusing does not resurrect the key: focus is not input state.

The instrument prints are the `platform:` lines; the engine's own `focus
gained`/`focus lost` lines come from `HasFocus`, the state the fold
maintains alongside the keys. On your own desktop, do it with a real
alt-tab — click away while holding a movement key and watch the drop
happen in the report instead of watching your marker drift into a wall.
