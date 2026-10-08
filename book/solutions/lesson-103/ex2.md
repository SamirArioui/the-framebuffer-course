# Solution: exercise 2 — your machine's hand-over card

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 103 — now make YOUR game](../../lessons/part-5/lesson-103-your-game.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

The smallest confirming change there is: the machine's name. The
report already prints it (`RUN_MACHINE` in `src/main.cpp` — the
constant lesson 101's code step introduced) and the exercise's card is
only honest when the name is yours, so the diff is one string. Fill
the placeholder with a real description — the CPU, the OS, the
display, the sound device — because that string is what makes every
other number on the card quotable (D12: a number that lost its
machine is a rumor).

The card itself, in the order to write it:

1. **The report as your machine prints it** — the whole table, plus
   the by-state, budget, and machine lines. Ours, for comparison
   (WSL2, Xvfb `:99`, no sound hardware, `-O0`): `avg 1.283 ms`
   across 2,583 frames, `tilemap 0.538 / clear 0.244 / sprites 0.006
   / text 0.011` inside `render 0.799`, `present 0.438`, `0 of 2583`
   frames over the 16.667 ms budget.
2. **What your machine changed, row by row** — and *why*, which is the
   card's real content. Expect structural, not scalar, differences:
   `present` flips its shape on a local display (our X server charged
   its wait as wall time with almost no CPU; a local desktop may bill
   the process instead), `audio` becomes a real device mix instead of
   silence, and the frame *rate* becomes the audio feed's (~60) rather
   than our jiggles' ~25 — so your frame *count* and pacing differ
   before any cost does. Scalar shifts (every CPU row scales with your
   processor and your build flags) are expected; shape shifts are the
   interesting lines.
3. **The extras ledger's first entry** — one idea your game had while
   you ran it (it will have one; running your own game is an idea
   generator), recorded, not added: a name, a one-line scope, and the
   words *not in scope*. This entry is the discipline starting to
   work. Lesson 102's ledger shows the finished shape.
4. **What the card asks for first** — read off your *own* rows: the
   largest phase on your machine is your hotspot, and the three-pass
   menu applies to it as it applied here (measure again before the fix,
   fix only what measurement named, report what you did not). If your
   machine's answer is "nothing is near the budget", that is a finding
   too — say so, and go build the game before optimizing anything.

One caution the card exists to prevent: **do not compare your
milliseconds to ours as a verdict on either machine.** Different
hardware, different build flags, different run shapes — the comparison
is only meaningful row-shape to row-shape, and every number belongs
with its name. That is what the hand-over card keeps straight, and it
is the whole of design D12 handed to you working.

When the card is filled, the checklist's perf line is yours to answer
— *my machine holds 60 fps* or honestly does not, with the frames that
miss counted (lesson 101's exercise 1 taught the instrument; this card
is the discipline around it) — and the course is out of claims about
your game. Measure well.
