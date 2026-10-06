# Solution: exercise 1 — The hitbox

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 056 — the mover that stops at walls](../../lessons/part-2/lesson-056-mover.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

The patch does two things: the mover's queries shrink to a box inset
two pixels on every side (`(int)next_x + 1`, `width - 2`), and a startup
check compares the two boxes at one position — `x = 15`, one pixel into
the wall's last column:

```
engine: hitbox check: at x=15 — full box solid, 2px inset free
```

That pair is the whole idea in one line. At `x = 15` the sprite's *art*
reaches from 15 to 30 — its left column sits on pixel 15, which is the
border wall's last pixel. The full 16×16 box therefore overlaps a wall
cell and the query says **solid**. The inset box covers pixels 16..29 —
entirely floor — and says **free**. The mover with the inset box is
allowed to stand at `x = 15`, and the player sees the sprite's art
fusing one pixel into the wall's edge: no gap, no visible seam.

Now the honest second half of the verification — driving into the wall.
The run stops at `blocked at 18,232`, the same integer position the
full-box mover reported. The hitbox's extra pixel is invisible at the
mover's step size: the sprite moves in ~10-pixel steps (240 px/s × the
frame's dt), and the last step that *fits* lands at 17–18 whatever the
box's one-pixel inset allows. The query difference is real (the check
proves it); the *mover* can't show it without finer steps — which is
exactly exercise 2's subject. A check that isolates the variable (the
startup comparison) earned its place next to the behavioral one.

The design question — why games keep the hitbox smaller than the art:

- **Forgiveness.** Players read the art as the character, but a hitbox
  exactly at the art's edge makes near-misses feel unfair: the sprite
  *looks* like it made it past the pillar and still stops. Two pixels of
  inset turns "I clearly hit that" into "I just barely missed it",
  which is the same physics with better manners.
- **Readability in motion.** At speed, a tight hitbox makes the sprite
  snag on corners the player cannot see; a smaller box slides through
  the art's visual gaps.
- **The wrong choice**: when the art *is* the gameplay surface —
  block-pushing puzzles where the block's face must meet the wall
  exactly, or tile-exact platformers where one pixel is the difference
  between standing and falling — the inset lies about the geometry, and
  the player learns to distrust the picture. Collision should be as
  forgiving as the game's verbs, and no more.

One more habit worth taking: the inset belongs to *the mover's box*, not
to the map. The queries stay exact (lesson 055's contract is
unchanged); the game decides what rectangle it is willing to be.
