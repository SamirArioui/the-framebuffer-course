# Solution: exercise 1 — the pause over the frozen world

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 096 — screen polish](../../lessons/part-5/lesson-096-screens.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

Three moves make the pause show its world. The framebuffer grows
`ClearRect` — `ClearBuffer` clipped to a rectangle, with the same fold
(a rectangle's part outside the frame is dropped, never wrapped). The
render phase draws the world for the pause exactly as for play (the
map and the sprites sub-phases run; the world is *frozen*, not gone).
And the pause panel paints a rectangle of panel color — faded in with
the screen, so the box arrives with the same ease — behind its lines,
so they stay legible over the scene.

The frame record measures the difference better than any description.
The pause frames now carry the world's sub-phases:

```
engine: state play -> pause (the player paused)
engine: screen: pause: "PAUSED" / "SCORE 000422   TIME 0:02   WAVE 1/3" / "ESCAPE: RESUME"
frame 80: step 0.000 ms, update 0.009 ms (entities 0.002), audio 0.000 ms, render 1.539 ms (sprites 0.004, text 0.160, tilemap 0.961), present 0.340 ms, total 1.888 ms
frame 81: step 0.000 ms, update 0.011 ms (entities 0.003), audio 0.030 ms, render 2.027 ms (sprites 0.180, text 0.158, tilemap 1.157), present 0.495 ms, total 2.562 ms
```

`tilemap 0.961` and `sprites 0.004` — the map and the entities are
drawn — beside `step 0.000 ms`, the world *frozen* under them. The
panel's own cost is `text 0.160` (the box and three lines, more than
the old panel's `0.016` — a rectangle is real work). The other screens
still draw no world at all — the death screen, one page later in the
same run:

```
engine: screen: death: "GAME OVER" / "SCORE 000424   TIME 0:06   WAVE 1/3" / "ENTER: TITLE"
frame 212: step 0.000 ms, update 0.014 ms (entities 0.003), audio 0.037 ms, render 0.436 ms (sprites 0.000, text 0.016, tilemap 0.000), present 0.483 ms, total 0.971 ms
```

`sprites 0.000, tilemap 0.000` — the end screens are their own world,
as the design says.

Two judgment calls in the diff, worth naming. The pause keeps the
*play* backdrop (`32,32,64`) rather than the panel's — the scene is the
screen's subject and the box is the only panel-colored thing. And the
box is drawn *before* the lines, inside the panel's own draw: the
screen owns everything it shows, and the draw order inside it is
theirs — box first, text after, exactly the render phase's own rule
(what is behind draws first) applied one level down.
