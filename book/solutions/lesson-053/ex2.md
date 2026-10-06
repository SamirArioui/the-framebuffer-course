# Solution: exercise 2 — The empty draw

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 053 — tilemap drawing](../../lessons/part-2/lesson-053-tiles.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

The prediction, from what the walk does at each placement. The loop
visits all 48 × 32 = 1,536 cells at *every* origin — it computes a
position per cell and calls `BlitSprite`, and the blitter's clip decides
what actually copies. So the three cases differ only in how many blits
end up doing real work:

- **`(0, 0)`** — every cell lands inside the 640×480 frame except the
  right and bottom margins (the map is 768×512): nearly full copying.
- **`(-400, -300)`** — the frame shows the map's region from (400,300);
  roughly a quarter of the cells are visible. The rest hit the clip and
  cost almost nothing.
- **`(-4096, -4096)`** — every cell is far outside; every blit's clip
  rectangle is empty. The walk still runs; the copying is zero.

Predicted shape: the first number is the full cost, the second a
fraction of it proportional to visible cells, the third a floor — the
walk's own overhead with no copying at all. The run:

```
engine: map walk at 0,0: 0.977 ms
engine: map walk at -400,-300: 0.247 ms
engine: map walk at -4096,-4096: 0.005 ms
```

The gradient matches, and the numbers split the cost cleanly:

- **Walking: ~0.005 ms** — visiting 1,536 cells, computing 1,536
  positions, calling 1,536 blits that immediately clip to nothing. That
  is 0.5% of the full draw.
- **Copying: ~0.97 ms** — the pixels. Nine-hundred microseconds that
  scale with *visible pixels*, which is why the mid-placement draw
  (0.247 ms) tracks the visible fraction so closely.

So the clip already does the heavy lifting: off-screen tiles are cheap,
and the walk's fixed overhead is negligible at this map size. But notice
what "cheap" is not: it is not **free**, and it scales with *total*
cells, not *visible* cells. A 256×256 map (65,536 cells) would spend
~0.2 ms per frame purely on the walk even fully scrolled away.

Skipping instead of clipping — the prompt's last question — is
**culling**: compute the rectangle of cells that intersect the frame
(`-x / TILE_SIZE` through `(FRAME_WIDTH - x) / TILE_SIZE`, clamped to
the map) and loop only those. The inner code does not change; the loop
bounds do. It is the "copy less" lever from lesson 047's list, applied
at the walk level — and Part 5's optimization pass is where a version of
it lands if the profiler says this row is hot. The measurement above is
exactly the evidence that decision will need, already in the frame
record's `tilemap` phase.
