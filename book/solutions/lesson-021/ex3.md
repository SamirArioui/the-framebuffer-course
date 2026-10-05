# Solution: exercise 3 — CSI, SS3, and your terminal

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 021 — raw terminal input with escape codes](../../lessons/part-0/lesson-021-terminal-input.md).*

## The diff

```diff
{{#include ex3.patch}}
```

## Walkthrough

Arrow keys have two byte spellings. `ESC [ A` is a *CSI* sequence (the
control sequence introducer); `ESC O A` is an *SS3* one, sent when the
terminal is in "application cursor keys" mode — a mode full-screen editors
like vim switch on so they can tell the numeric keypad apart. Which one your
arrows produce depends on the terminal *and* on what is running inside it, so
a parser that only knows `[` breaks on somebody's machine — and portability
work in terminal code starts with asking the terminal what it sends:
`cat -v` (or `showkey -a` on Linux) prints `^[[A` for CSI and `^[OA` for
SS3 — `^[` is `cat -v`'s spelling of ESC.

The patch adds the SS3 path to the parser's middle state: either `[` or `O`
moves the machine to "sequence final byte next", and the `A`–`D` mapping is
shared by both spellings. Verified here with `printf '\033ODq' | ./snek 30`
(SS3 left, then quit): `frame=1 tick=0 dir=left`, `done after 1 frames`,
while the CSI form still works unchanged. On Windows Terminal under WSL the
pty speaks the same bytes; a bare Windows console build would need
`ReadConsoleInput` instead of `termios` — a different port, a different
API.
