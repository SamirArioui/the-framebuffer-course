# Solution: exercise 2 — The ledger

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 029 — clean close and error paths](../../lessons/part-1/lesson-029-clean-close.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

The ledger is one line per take and one per release, printed from inside the
implementation where each of them actually happens. Before running anything,
the prediction to make is a balance for each exit: how many takes, and whose
releases balance them.

**The user's close — the request (`ClientMessage`):** three takes (display,
window, signal handler), three matching releases.

```
platform: take display
platform: take window
platform: take signal handler
platform: release window
platform: release display
platform: release signal handler
```

**The destroyed window (`DestroyNotify`):** the same three takes — but no
`release window` line. The surprise the prediction should catch: the OS
released that resource itself when it destroyed the window. The ledger says
so (`window released by the OS`), and `CloseWindow` skips the destroy it no
longer owns. The balance still closes — three takes, three releases — one of
them performed by the OS.

```
platform: take display
platform: take window
platform: take signal handler
platform: window released by the OS
platform: release display
platform: release signal handler
```

**Ctrl+C (`SIGINT`):** identical to the clean close — the interrupt is news
that turns into the same close, and the same three releases run. This is the
exit lesson 028 could not handle at all.

```
platform: take display
platform: take window
platform: take signal handler
platform: release window
platform: release display
platform: release signal handler
```

**The missing display:** the ledger is *empty*. The failure happens before
the first take, so there is nothing to release — the rule holds trivially,
and the engine's error line is the only output.

```
engine: no display to open a window on
```

The pattern across all four: releases balance takes on every path, and the
only line that shifts is *who* released the window. That is what "OS
resources released on every exit" means when it is measured instead of
hoped — and the ledger is the smallest instrument that measures it.
