# Solution: exercise 3 — Two rows, one key

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 024 — the function-pointer command table](../../lessons/part-0/lesson-024-command-table.md).*

## The diff

```diff
{{#include ex3.patch}}
```

## Walkthrough

The prediction: the message never prints. `RunCommand` scans the table from
the top and `return`s after the first key match, so the original `{'q',
CmdQuit}` row — near the top — swallows every `q`, and the new `CmdQuitSecond`
row at the bottom is dead data. The run confirms it: `printf 'q' | ./snek 30`
ends `done after 1 frames` with no `second q row ran` anywhere in the output.
Delete the *first* `q` row and the message appears instantly — proof that the
scan, not the key, decided the outcome.

First-match-wins is the contract, and it is the same contract `switch` has —
except here the "cases" are data you can reorder, extend, and even load from
somewhere else at runtime. The contract has teeth: a shadowed row is
silently unreachable, and no compiler warns you, because a duplicate `int` in
an array is perfectly legal C. That is the trade of data-driven dispatch —
you trade compiler-checked branches for a table whose rules are yours to
keep: one row per key, first match wins, and the scan's `return` is what
makes it so. Exercise 1's WASD rows are safe from shadowing precisely
because every row there binds a distinct key.
