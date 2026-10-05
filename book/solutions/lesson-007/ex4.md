# Solution: exercise 4 — The other machine's layout

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 007 — structs: sizeof, alignment, and padding](../../lessons/part-0/lesson-007-struct-layout.md).*

## The diff

```diff
{{#include ex4.patch}}
```

## Walkthrough

With a 4-byte `long`, the alignment rule yields 20 bytes for `struct Item`
with `value` at offset 16: the key occupies offsets 0–15, and 16 is already a
multiple of the new alignment (4), so no padding appears. `struct Scattered`
shrinks to 12 — `tag` at 0, `score` at 4, `flag` at 8, size rounded up to the
struct alignment of 4. The 24/16 contrast of this lesson compresses too, but
reordering still wins.

The fingerprint line gives every machine a one-line identity. The author's
x86-64 Linux box prints:

```
fingerprint: char=1 int=4 long=8 ptr=8 item=24
```

A 32-bit build should print `long=4 ptr=4 item=20`, and 64-bit Windows
`long=4 ptr=8 item=20` — `long` stays 4 bytes there even though pointers are
8. This box has no 32-bit headers, so those two lines are predictions, not
runs: your job is to confirm or refute them against the hand calculation
above. That variance is why C programmers reach for `<stdint.h>` when a size
must not vary — lesson 013 does exactly that. If `gcc -m32` works on your
machine, run the program both ways and watch one source print two layouts.
