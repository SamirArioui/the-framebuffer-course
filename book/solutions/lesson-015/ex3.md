# Solution: exercise 3 — The addition that ate the clip

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 015 — fill-rect onto a memory buffer](../../lessons/part-0/lesson-015-fill-rect.md).*

## The diff

```diff
{{#include ex3.patch}}
```

## Walkthrough

With the offending call added, the unfixed program dies immediately —
`Segmentation fault (core dumped)`, exit code 139. `x = 2147483640` and
`rw = 100` make `x + rw` overflow `int`; the wrapped sum is negative, the
test `x + rw > w` comes out false, the fold does nothing, and the write
loop drives `PutPixel` at offsets far past the buffer. The fix removes
the addition from the test: `rw > w - x` asks the same question — "does
the rectangle reach past `w`?" — by subtracting instead, and `w - x` is
safe because `x` is already known non-negative (the start fold ran first),
so `w - x` sits well inside `int`. The same call now computes
`rw = 8 - 2147483640`, a negative width, and the empty case returns
without a single write. Hold on to the shape of this bug: "compute
`x + n`, compare, clamp" is overflow-bait whenever a caller controls `x`,
and lesson 018 shows what the optimizer does with overflow-bait even when
it does not crash.
