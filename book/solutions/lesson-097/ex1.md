# Solution: exercise 1 — the banners come in from the cold

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 097 — pay the debt](../../lessons/part-5/lesson-097-debt.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

The five banners are the run's reports like any other — lines a
checklist's demonstration reads — so the move follows the lesson's own
rule: the printfs travel whole, `report.*` gains the name they lacked
(`ReportBanners`, taking the `World` they describe), and `main.cpp`
drops the two includes (`font.h`, `tiles.h`) the banners were the last
to need. The loop's file now holds the window, the loop, and the close
— and the run's identity is printed from the report pair beside every
other report.

The run still introduces itself exactly as it always has — same five
lines, same words, same order, from a real run of the patched build:

```
engine: part 4 done — the vertical slice: a hero walks the tilemap, the camera follows
engine: world 48x32 cells (768x512 px), 3 kinds; 96 glyphs; hero 32x16
engine: sound 132300-frame music looping on channel 0; effects of 3528/6615/17640 frames on the pool; one mixer of 16 channels
engine: arrows move the hero, 1 and 2 arm the weapons, space fires; close the window to stop
engine: hero at 312,232
```

And the transcript's shape does not move at all: the run reduced to its
reports (the method of exercise 2) is the same set of templates as the
unpatched build's — the banners' five templates among them, in the same
first-appearance order. One detail worth naming for your own refactors:
the banners sit between the world's start and the audio's open *on
purpose* — `WorldStart` prints the world's checks, the banners name the
run, and only then does the seam open its output — and the move keeps
that order, so the failure path (`no audio output on this machine`)
still prints *after* the identity, exactly where a reader expects it.
