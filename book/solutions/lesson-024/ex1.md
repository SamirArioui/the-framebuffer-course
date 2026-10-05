# Solution: exercise 1 — WASD is four rows

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 024 — the function-pointer command table](../../lessons/part-0/lesson-024-command-table.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

Four rows, no branches. `w`, `a`, `s`, `d` are plain keys whose bytes are
their codes, so each is one table entry binding the byte to the *same*
command function the arrow keys use — `CmdUp`, `CmdLeft`, `CmdDown`,
`CmdRight`. The parser, the dispatch scan, the state guard, and the
reversal rule in `CmdTurn` are all untouched: the commands were already
complete; only the bindings were missing.

A timed tour proves it —
`( printf ' w'; sleep 0.4; printf 'a'; sleep 0.4; printf 's'; sleep 0.4;
printf 'dq' ) | ./snek 60` — and the trace narrates the walk: `dir=up`,
then `dir=left`, then `dir=down`, then `dir=right` in the final frames
before `q` quits (`done after 37 frames` in one run, 38 in another — pipe
scheduling, as in lesson 021). Each turn is perpendicular to the last, so
the reversal guard never fires; try `w` then `s` directly and you will see
the guard reject the second key.

This is the lesson's whole argument. In the old branch dispatch, word keys
would have meant four more `else if` arms inside `OnByte`; in the table they
mean four rows — data — and the dispatch code is exactly as long as it was
before the feature existed.
