# Target game: the frozen MVD contract

Frozen now, before lesson 1 exists. This contract propagates backward into
Parts 1-4 design, and Part 5's scope cannot expand mid-course: an idea outside
this contract is recorded as extras or post-course material and never enters
the part's scope.
*(target-game: Frozen feature checklist / Out-of-scope ideas are recorded, not added)*

## Frozen feature checklist

```
  [ ] hero: 8-dir movement with accel/decel feel
  [ ] combat: projectiles, 2 weapon types
  [ ] enemies: 3 types + 1 boss (chase / keep-distance / flee AI)
  [ ] world: single scrolling map, tile collision
  [ ] states: title, play, pause, death, victory
  [ ] juice toolkit: hitstop, screenshake, particle burst, easing
  [ ] audio: music + sfx through our own mixer
  [ ] perf: 60 fps on modest hardware + final frame-budget report
```

The game is declared complete only when every line is implemented and
demonstrable.
*(target-game: Frozen feature checklist — Checklist completion)*

## The feel toolkit: four effects, one principle

Game-feel treatment is limited to exactly four effects:

1. **hitstop**
2. **screenshake**
3. **particle bursts**
4. **easing**

— grounded in one principle: **immediate and readable feedback**. Any feel
effect added to the game is one of these four; a fifth is out of scope by
definition. This is a toolkit, never an open-ended tour.
*(target-game: Feel toolkit bounded)*

## Optimization: the fixed three-pass menu

1. **Measure** — the profiler lesson: instrument, measure, and name the
   top-2 hotspots.
2. **Fix top-2** — fix those two hotspots, applying the techniques the deep
   dives taught (cache layout, SIMD, allocation). Every optimization change
   addresses one of the two measured hotspots.
3. **Report** — produce the final frame-budget report: the course finale. It
   accompanies the finished game and attributes per-frame time to each major
   subsystem.

*(target-game: Optimization follows the three-pass menu)*

## Definition of done

The finished game is **done** when all three hold:

1. every checklist line is implemented and demonstrable,
2. the game sustains **60 fps on modest hardware**, and
3. the **final frame-budget report** accounts for the per-frame time of each
   major subsystem.

"More features" is an extras chapter or post-course work — never part of done.
*(target-game: Performance definition of done)*
