# Solution: exercise 3 — Pack the state

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 019 — the game loop](../../lessons/part-0/lesson-019-game-loop.md).*

## The diff

```diff
{{#include ex3.patch}}
```

## Walkthrough

The three file-scope variables become the three fields of `struct Game`, the
phases grow a `struct Game *g` parameter and reach state through `g->`, and
`main` owns the single instance — `&game` in the call sites is the `this`
pointer of the C++ lesson to come. Behavior is supposed to be unchanged, and
the first run after the refactor will *not* be: `frame` starts as garbage like
`133509038754545` and the loop counts toward nowhere. That is the one real
lesson hiding in this drill. File-scope variables are zero-initialized by
default; a local like `game` is not — its fields are whatever the stack
happened to hold. `struct Game game = {0};` zero-initializes every field
(including `frame`), which is the C idiom worth memorizing. `ProcessInput`'s
`(void)g;` keeps `-Wextra`'s unused-parameter warning quiet until lesson 021
fills the function in. With that, `./snek 3` prints the same three frame
lines and the same exit summary as before the refactor.
