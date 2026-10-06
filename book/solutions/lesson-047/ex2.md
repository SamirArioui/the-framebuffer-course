# Solution: exercise 2 — The stride that doesn't fit the line

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 047 — the caches deep dive](../../lessons/part-2/lesson-047-caches.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

First the prediction, which is where the exercise actually lives. Stride
60 against 64-byte lines: touches land at bytes 0, 60, 120, 180, 240 …
and lines cover 0–63, 64–127, 128–191 … The first touch (0) and the
second (60) share line 0; the third (120) is in line 1; the fourth (180)
is in line 2. Some touches reuse the line the previous touch fetched,
some fetch a new one — and over any long run the arithmetic settles:
`size / 60` touches spread over `size / 64` lines is **1.07 touches per
line**. Stride 64 is exactly 1. So the prediction is: stride 60 lands
*on top of* stride 64 — both use about one useful byte per fetched line,
and neither approaches sequential's 64.

The run at `-O3`:

```
engine: cache probe — copy walk, useful GB/s per stride
engine: working set   sequential    stride 64    stride 60
engine:       4 KB        108.6          3.3          3.0
engine:      64 KB         57.3          1.2          1.3
engine:    512 KB         45.5          1.0          1.1
engine:   4096 KB         14.0          0.4          0.4
engine:   8192 KB         16.7          0.3          0.4
engine:  12288 KB         15.3          0.3          0.3
```

Stride 60 and stride 64 are indistinguishable at every working set —
sometimes a hair slower (4 KB: 3.0 vs 3.3), sometimes a hair faster
(64 KB: 1.3 vs 1.2), always inside the noise floor. The prediction
holds.

So the question the numbers ask — stride value or useful bytes per
line? — has its answer in the table. If the *stride value* were the cost,
60 would sit between 64 and sequential, closer to the middle. It does
not; it sits on 64. What both strides share is the same waste: roughly
one useful byte per 64-byte line fetched. The machine's cost is per
*line*, and the only question a memory pattern can answer well is "how
many useful bytes does each fetched line carry?"

The corollary is worth writing down, because it is the whole locality
lesson in one sentence: **the number that matters is `useful bytes / 64`,
and anything that improves it — wider types, tighter rows, walking the
memory the way it is laid out — moves you up the table toward sequential;
anything that does not, does not.** A stride of 8 would carry 8 useful
bytes per line and land between the columns. The blit's own copy walks
whole lines as it goes — which is the best this table's numbers can be,
and the reason no amount of striding discipline in a renderer beats just
copying memory in the order memory lives.
