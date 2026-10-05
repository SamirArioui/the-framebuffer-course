# Solution: exercise 2 — The string that never ends

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 019 — the game loop](../../lessons/part-0/lesson-019-game-loop.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

The two strings fail for two different reasons, and neither is visible to
`*end != '\0'`. `strtoul` is documented to accept an optional sign, so `-5`
is *parsed*, not rejected: it comes back as `18446744073709551611` — the
unsigned wrap-around of −5 — with `end` at the terminator and `errno`
untouched. The overflow string converts to `18446744073709551615` (`ULONG_MAX`)
and is the one case that sets `errno` to `ERANGE`. Both strings are fully
consumed, so the end-pointer test passes and `Update` counts up toward a
number the machine will never reach — that is the "hang".

The fix adds two checks: a leading `-` is refused outright, and `errno ==
ERANGE` catches overflow. Both are needed — the `-5` run leaves `errno` at
zero — and `errno = 0` before the call is what makes the second check sound,
since `errno` is sticky and may hold some older failure. After the fix both
runs print the usual `FRAMES must be a positive integer` message and exit 1,
while `./snek 3` behaves exactly as before.
