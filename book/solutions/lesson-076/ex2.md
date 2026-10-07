# Solution: exercise 2 — The hero on your desktop

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 076 — the hero as an entity](../../lessons/part-4/lesson-076-hero.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

The diff is the hero's account: the ground `distance` already counts,
the run's wall time, and the time the hero was *asked* to move — the
frames whose movement request was non-zero. Two speeds fall out of
three numbers: distance over the run, and distance over the moving
frames alone. That split is the whole point, because the two numbers
answer different questions.

The authoring machine's run — twelve scripted Rights and six Downs,
no audio device, headless display — reports:

```
engine: hero: walked 605 px in 4.099 s (2.520 moving) — 148 px/s over the run, 240 px/s while moving, 240 in the row
```

Read the two speeds. **240 px/s while moving** — the row's number, to
the digit. The model is exact: every frame the key was down, the hero
stepped `speed × dt`, and the sum of those steps over the sum of those
`dt`s is 240. **148 px/s over the run** — the same motion diluted by
the 1.58 seconds nobody was pressing anything and by this machine's
sampling: without a sound device the loop sleeps between input news, so
each scripted press wakes two frames (press and release) and only the
one that finds the key down moves. The hero is not slower than its row;
it is *asked* to move less than the wall clock ran.

That is the difference the port makes. On a real desktop — or with a
working audio output ticking the loop at the buffer horizon — a held
key keeps the movement request non-zero on every frame, the loop runs
at its own cadence instead of waiting for news, and the two numbers
converge on 240. What you should see on your machine:

- **the position lines** at the held key's pace (one per frame, each
  step `240 × dt` — with audio feeding, `dt` is a steady ~16.7 ms and
  the steps are ~4 pixels);
- **the frame log's cadence** — this machine's run is woken by input
  alone (`walk: 301 visits over 38 frames`), yours ticks continuously;
- **the hero's account** with `while moving` at 240 px/s and `over the
  run` at whatever your key-holding and your run's idle time make it.

And the diagonal: hold two directions and measure. With this lesson's
model the two speeds will *not* match — the diagonal is √2 times
faster while moving (exercise 1's answer) — and the measured number
proves it in your own units.

The habit is the instrument's, as always in this course: a claim like
"the hero moves at its row's speed" is a *measurement* — distance over
time, taken on a named machine — and the account above is what makes it
one. Report your machine beside the book's (CPU, display, sound device
or none) the way the part's reviews do.

Nothing here touches the movement model, the walk, or the game's loop:
the account is one line beside the run's own.
