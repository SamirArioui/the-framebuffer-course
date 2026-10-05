# Solution: exercise 2 — Enter is a carriage return

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 024 — the function-pointer command table](../../lessons/part-0/lesson-024-command-table.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

The trap is a translation. `printf '\n' | ./snek 20` starts the game, so the
`'\n'` row looks right — but a real terminal's Enter key sends CR (`0x0D`),
not LF (`0x0A`), and this lesson's raw mode clears `ICRNL`, the line
discipline flag that would quietly rewrite CR into NL. The program now sees
what the keyboard actually sent, and the table has no row for it: Enter does
nothing. (Feed the pipe with `printf '\r'` and you see the same failure
without any terminal at all.)

Reproduce through a pty with input arriving *after* raw mode is on — pre-fed
bytes are translated before the program can turn the translation off:

```
( sleep 0.5; printf '\r' ) | script -qec './snek 20' /dev/null
```

Without the fix, frame 20 still reports `state=title`. The fix is one row —
`{ '\r', CmdStart }` — and the same run reaches `state=play`, with `'\n'`
kept so pipes and old habits keep working. Both spellings of "confirm" live
in the table now, which is the command table earning its keep: a
compatibility quirk costs one line of data. The deeper lesson is that "raw"
means the program sees bytes as the hardware sent them — every translation
the terminal used to do (CR→NL, XON/XOFF, backspace quirks) is now your
contract with the keyboard.
