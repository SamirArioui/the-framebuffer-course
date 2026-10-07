# Solution: exercise 2 — The base, at the corners

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 083 — the tilemap and camera](../../lessons/part-5/lesson-083-tilemap-camera.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

The probe keeps the base the camera *wanted* (`want_x`, `want_y`) beside
the base it *got* after the clamp, and prints both:

```
engine: camera base %d,%d (wanted %d,%d)
```

So every report is a clamp check: when `base` differs from `wanted`, the
clamp held it; when they match, the view was free to follow.

**The five predictions.** The base is `hero + half sprite − half frame`,
clamped to `x ∈ [0, 768−640] = [0,128]`, `y ∈ [0, 512−480] = [0,32]`.
With a `16×16` sprite and a `640×480` frame, the centered base is
`hero.x − 312`, `hero.y − 232`. Worked out at each spot:

| hero (world px) | wanted base | clamped base | what the clamp did |
| --------------- | ----------- | ------------ | ------------------ |
| `(0, 0)` | `(−312, −232)` | `(0, 0)` | both clamped up |
| `(752, 0)` | `(440, −232)` | `(128, 0)` | x clamped down, y clamped up |
| `(0, 496)` | `(−312, 264)` | `(0, 32)` | x clamped up, y clamped down |
| `(752, 496)` | `(440, 264)` | `(128, 32)` | both clamped down |
| `(312, 232)` | `(0, 0)` | `(0, 0)` | neither — already at the corner |

**Which clamp on both axes?** All four map corners clamp on *both* axes —
and the `(312,232)` row is the giveaway for why. The map is `768×512`
and the frame is `640×480`, so the view can only scroll **128 pixels
across and 32 down** in its whole life. That is a tiny range. The
centered base is `hero.x − 312`, and the only way to be unclamped is for
`hero.x` to sit in `[312, 440]` and `hero.y` in `[232, 264]` — a
128×32-pixel box in the middle of the world. At every map corner the hero
is far outside that box, so both axes clamp. The `(312,232)` start is the
one spot in your list that lands exactly on the unclamped origin: the
camera happens to sit at `(0,0)` there without being forced.

The run's own probe lines confirm the clamp is real, not modeled — here
the hero pushed low enough that `wanted` went negative on `y` and the
clamp held it at zero:

```
engine: camera base 2,0 (wanted 2,-1)
```

`wanted 2,−1`: the free-follow base would have been one pixel above the
map's top edge. `base 2,0`: the clamp refused. Exactly the arithmetic in
the table, measured from the frame that ran.

The insight to carry forward: this map barely scrolls. The camera's
*clamp* is doing a lot of work here because the world is only slightly
larger than the window — on a bigger map the camera would follow freely
over most of the world and clamp only near its edges. The `wanted`/`base`
probe is the tool that shows you which regime you are in at any moment.
