# Solution: exercise 2 — The key that vanished

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 021 — raw terminal input with escape codes](../../lessons/part-0/lesson-021-terminal-input.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

`printf '\033q' | ./snek 30` running all thirty frames is the whole symptom:
after a lone ESC, the parser's middle state tests the next byte against `'['`
and, on mismatch, quietly drops it — the `q` is eaten and the game never
quits. The same hole swallows any key pressed right after the Escape key.

The fix restructures `OnByte` around one rule: **an unexpected byte is not a
sequence byte, so it must be reprocessed as a plain key**. The old state
checks move to the top and return only when they genuinely consume a byte;
when state 1 sees something other than `[`, it resets and falls through to
the plain-key handling below — where the same byte is tested against `0x1b`
and `q` like any other. State 2 still consumes one byte unconditionally (it
is the sequence's final byte) and maps `A`–`D` to directions.

After the fix `ESC q` quits at frame 1, `q` and `ESC [ A q` behave exactly as
before, a lone ESC still leaves the game running, and even `ESC ESC [ A`
recovers: the second ESC falls through and restarts the sequence cleanly.
