# Solution: exercise 1 — The window that gets opened twice

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 029 — clean close and error paths](../../lessons/part-1/lesson-029-clean-close.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

The bug first. The platform layer keeps one window's worth of state in
static storage, and `OpenWindow` hands out `&window_state` to anyone who
asks. A second call while the first window is open does not fail — it
*overwrites*: the first display connection's handle is lost without being
closed, and from then on `CloseWindow` cleans up the second window while the
first one's connection leaks for the life of the process. Nothing crashes,
which is exactly why it is a bug worth proving before fixing.

The fix is the ownership rule made enforceable: one window at a time. The
first thing `OpenWindow` does is check whether its state is already taken,
and if so it returns a typed failure — `OPEN_ALREADY_OPEN`, added to the
enum like every other reason the seam can refuse. No resource is touched on
that path, so nothing needs releasing: the failure arrives before the take.

The proof, in the engine:

```
$ DISPLAY=:99 ./build/game &
$ DISPLAY=:99 xdotool windowclose <id>
engine: window 640x480 open — waiting for news
engine: second open refused (platform error 3)
engine: close reported
engine: closed
```

The first window opens and closes normally; the second call reports reason 3
— `OPEN_ALREADY_OPEN` — and takes nothing. The engine's error `switch` grew
the matching case so the refusal is reported by name like the others.

One window is not a limitation to be fixed later — it is the contract, and
the enum says so. When the engine one day needs a second window (a level
editor, a second viewport), the seam grows a real window collection with
real ownership, and the refusal becomes what it was protecting: a well-typed
answer instead of a silent overwrite.
