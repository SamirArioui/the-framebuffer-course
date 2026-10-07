# Solution: exercise 1 — The score on the end screens

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 082 — the game skeleton](../../lessons/part-5/lesson-082-skeleton.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

The exercise is one idea twice over: the score belongs to the game, and
the end screens are the game's. Both are moves the skeleton was already
set up to make.

**The score moves into `Game`.** It was a local in `Run` — `double
distance`, accumulated by the walk and read by play's HUD. That is the
loop's fact, not the game's, which is exactly why the end screens could
not see it: they are drawn by `GameDrawPanel`, over in `game.cpp`, and a
local in `Run` does not reach there. So the score becomes `Game::score`,
a double that the walk still adds to (`game.score += …`) and that a
fresh game resets to zero on the title's start key — the same restore
the hero's health and the game's waves already get. The game carries its
own result.

**The end screens read it.** A small `PanelScore` helper formats
`SCORE %06d` and centres it under the panel's hint; the death and
victory cases call it with `game.score`. The draw is text like any other
— it lands in the frame's `text` phase, and the panels stay panels (the
world is still not drawn behind them).

**The end states accept Escape too.** The death and victory cases now
read either `KEY_ENTER` or `KEY_ESCAPE`, still through the lesson-033
latch, so one press is one action. This does not break the machine's
rule — only the current state's input acts; the change is that the end
states now list *two* keys among the input they accept. Escape already
leaves play for pause, so a player who reaches for it on the end screen
is not surprised.

The run, from this lesson's end state plus the patch — the hero is hit
three times and dies, then Escape is pressed on the death screen:

```
engine: state title -> play (the player started)
engine: hero takes a hit — health 2 (t=0.020)
engine: hero takes a hit — health 1 (t=0.665)
engine: hero takes a hit — health 0 (t=1.109)
engine: state play -> death (the hero's health reached zero)
engine: state death -> title (the player returned to the title)
```

The last line is the widened input: `death -> title` fired from the
death screen, and it was Escape that fired it. The score itself is a
visual — the death panel now shows `SCORE 000142` (the hero's walk in
that run) under `GAME OVER`; open the window to see it on both end
screens, the same way the panels' text is checked.

One thing to notice about the reset: the score lives in `Game` now, so
`title -> play` restoring it to zero is the same one-line restore as the
health and the waves — the fresh-run facts are all in one place. When the
real combat and waves arrive (lessons 087 and 091), the score they award
writes to the same field, and the end screens keep reporting it without
knowing where it came from.
