# Solution: exercise 1 — The camera that leads

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 083 — the tilemap and camera](../../lessons/part-5/lesson-083-tilemap-camera.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

The camera already aims to center the hero: `base = hero + half sprite −
half frame`. Leading is one more term — push the aim a little in the
direction the hero is moving:

```cpp
int base_x = (int)hero.x + hero.sprite->width / 2 - FRAME_WIDTH / 2 +
             (int)(hero.move_x * GAME_LOOKAHEAD);
```

`hero.move_x` / `hero.move_y` is the hero's movement request, the same
polled direction the walk uses — `-1`, `0`, or `1` per axis. Multiplied
by `GAME_LOOKAHEAD` (48 pixels), it shifts the camera's aim 48 pixels
toward where the hero is going. Diagonal movement leads diagonally. At
rest the request is zero, so the camera re-centers — it only leads while
the hero is actually moving.

The clamp is untouched and still applied to the *led* base, so the
camera leads and still never shows past the map's edge. And the additive
offset stays at rest — leading moves the base, not the juice hook.

The run, from this lesson's end state plus the patch — the hero holds
Right, then releases:

```
engine: hero at 312,232
engine: hero at 389,232 (t=3.693)
engine: camera base 125,0
engine: camera base 77,0
```

Read it against the arithmetic. Moving right at hero `389`, the centered
base would be `389 + 8 − 320 = 77`. The led base reads `125` — that is
`77 + 48`, the look-ahead pushing the view ahead of the hero. The hero
sits left of center while it runs right: the player sees more of where
they are going. Then the hero stops (`move_x` returns to 0) and the base
falls back to `77` — re-centered. Lead on the move, center at rest.

One judgment call the exercise leaves to you: how far to lead.
`GAME_LOOKAHEAD = 48` is three tiles — enough to see a tile ahead without
feeling like the camera is chasing. Too large and the hero drifts toward
the screen edge; too small and the lead is invisible. Run it, watch the
hero's place on screen, and pick the number your game feels right at —
it is a constant, and it is yours.
