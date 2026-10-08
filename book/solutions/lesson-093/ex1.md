# Solution: exercise 1 — the sparks inherit the blow

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 093 — particle bursts and easing](../../lessons/part-5/lesson-093-bursts-easing.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

The burst grows a direction: `FeelBurst` takes the blow's travel
(`dir_x, dir_y`) beside its point and count. A radial burst — the
death's `(0, 0)` — keeps the lesson's even spread, one lane per
particle. A directional one picks its lanes by **dot product**: each
spark takes the unused lane that faces the blow most directly, so with
four sparks and a shot flying east the fan is the east, the two
diagonals beside it, and then whatever faces it least badly:

```cpp
double dot = LANE_X[l] * dir_x + LANE_Y[l] * dir_y;
if (dot > best_dot) { best_dot = dot; best = l; }
```

The hit passes the shot's own motion (a unit lane, or the diagonal's
`1/√2` — the same direction the flight used); the death passes zeros
and stays a shell burst.

The run — the killing blow of the lesson's excerpt, the shot flying
east from the hero into the bag — settles the twelve sparks like this:

```
engine: hit: bolt hits bag — damage 1, health 1 -> 0
engine: burst: spark x4 at 320,232 — 4 made, 0 dropped
engine: burst: spark x8 at 344,240 — 8 made, 0 dropped
engine: spark settled at 384,232 — 64 px out, its row's range 64 (exact)
engine: spark settled at 365,277 — 64 px out, its row's range 64 (exact)
engine: spark settled at 365,186 — 64 px out, its row's range 64 (exact)
engine: spark settled at 320,296 — 64 px out, its row's range 64 (exact)
engine: spark settled at 344,304 — 64 px out, its row's range 64 (exact)
engine: spark settled at 298,285 — 64 px out, its row's range 64 (exact)
engine: spark settled at 280,240 — 64 px out, its row's range 64 (exact)
engine: spark settled at 298,194 — 64 px out, its row's range 64 (exact)
engine: spark settled at 344,176 — 64 px out, its row's range 64 (exact)
engine: spark settled at 389,194 — 64 px out, its row's range 64 (exact)
engine: spark settled at 408,240 — 64 px out, its row's range 64 (exact)
engine: spark settled at 389,285 — 64 px out, its row's range 64 (exact)
```

The hit's four (from the impact at `320,232`) are `384,232` (due east),
`365,277` (south-east), `365,186` (north-east), `320,296` (south) —
three of the four land **forward of the impact**, the shot's way, and
the fourth is the north/south tie (both lanes face the eastbound blow
equally at dot `0`; the scan takes south first). The death's eight (from
the centre at `344,240`) are the full compass again: `408,240` east,
`280,240` west, the diagonals at `45` px out, the cardinals at `64` —
the thing that fell scatters in every direction, as a death should.

The two bursts read differently now — a hit *sprays*, a death *bursts* —
and the difference is one number: the direction the event hands over.
The lanes, the store's policy, and the settle are all exactly as the
lesson shipped them; the four effects are still the four.
