# Solution: exercise 1 — Your own keys

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 032 — polled input state](../../lessons/part-1/lesson-032-polled-input.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

Adding keys to the seam is three small, disciplined edits and nothing else.
The enum grows the four new values before `KEY_COUNT` — the count is the
size of the state array and the poll loop, so order matters. The mapping
grows four cases in `KeyIndex`, which is the one place in the codebase where
an OS key code (`XK_w`) becomes a seam key (`KEY_W`). The report grows four
names. No engine logic changed, because the engine never saw a key code in
the first place.

Verified with held keys, the same way the lesson did the arrows:

```
$ DISPLAY=:99 xdotool keydown --window <id> w
$ DISPLAY=:99 xdotool keydown --window <id> d
$ DISPLAY=:99 xdotool keyup --window <id> w
$ DISPLAY=:99 xdotool keyup --window <id> d
engine: polled: w
engine: polled: w d
engine: polled: d
engine: polled: -
```

Both new keys track independently: `w` held while `d` comes and goes, then
both up.

One detail the mapping shows: `XK_w` is the *lowercase* w keysym. What a
Shift+W produces is `XK_W`, which `KeyIndex` does not track — the seam's
keys are physical-ish keys, not characters, which is exactly right for
movement input (lesson 024's `snek` mapped characters and had to care about
case; an engine's movement keys should not). If you want shifted input
later, that is a second kind of state — text, not keys — and a different
contract.
