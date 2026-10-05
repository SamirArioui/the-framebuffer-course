# Solution: exercise 1 — Bytes all the way down

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 021 — raw terminal input with escape codes](../../lessons/part-0/lesson-021-terminal-input.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

With every byte printed in hex, the four runs confirm the prediction that a
terminal sends nothing *but* bytes. `q` arrives as `0x71`, nothing more.
An arrow key is three bytes — `0x1b 0x5b 0x41` is ESC `[` `A`, up — and the
parser assembles them into one direction change. A lone ESC leaves the parser
in its "after ESC" state and no direction ever changes. `ESC [ A q` is four
bytes in one read: direction up, then quit.

One prediction will have been wrong on most machines: exactly *which* frame
the bytes land on. On one run `q` quit at `done after 1 frames`; with the
heavier instrumented build the pipe delivered it in frame 2 — `done after 2
frames`. Input is asynchronous: the bytes sit in the pipe until `poll` sees
them, and which frame's `ProcessInput` catches them is scheduling, not
logic. That is also why one `read` can hold several keys — the loop drains
whatever has arrived in one call, and every byte is parsed in order.

Through a real terminal (`printf '\033[Aq' | script -qec './snek 30'
/dev/null`) the same bytes appear, now traveling through a pty, and the
program behaves identically.
