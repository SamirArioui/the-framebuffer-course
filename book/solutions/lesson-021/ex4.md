# Solution: exercise 4 — WASD

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 021 — raw terminal input with escape codes](../../lessons/part-0/lesson-021-terminal-input.md).*

## The diff

```diff
{{#include ex4.patch}}
```

## Walkthrough

Four `else if` rows in the plain-key branch — `w`, `a`, `s`, `d` — each
assigning the same direction enum the arrow keys use. Nothing else changes,
which is the pleasing part: the escape-sequence parser is untouched because
word keys are single bytes that never enter it. `printf 'wasdq' | ./snek 30`
runs one frame and the trace ends `dir=right` — all five bytes were parsed in
one read, the last of the four word keys leaving its mark before `q` quit.
Mixing the two spellings works too: `printf 'w\033[Csq' | ./snek 30` goes up,
right, down, quit.

Worth noticing how much code this took compared to the escape parser. Adding
a plain key is one comparison against one byte; adding a *named* key — a
function key, say, which sends a longer sequence — means extending the state
machine instead. That asymmetry between "keys are bytes" and "keys are
sequences" is exactly what lesson 024's command table will absorb: there the
arrow keys arrive as symbolic codes like any other key, and every binding —
word key or arrow — becomes one table row.
