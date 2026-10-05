# Solution: exercise 2 — The half-freed array

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 008 — dynarray growth: realloc and capacity](../../lessons/part-0/lesson-008-dynarray.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

The failing line is `free(da->items)` paired with `da->len = 0` and *nothing
else*: it is half of `DaFree`. The block is gone, but `cap` still says slots
are available, so the next `DaPush` sees `len < cap`, skips growth, and
writes 24 bytes into freed memory. Under ASan the write is caught where it
happens:

```
ERROR: AddressSanitizer: heap-use-after-free ... WRITE of size 24
    #0 ... in DaPush sandbox/ds-kit/ds-kit.c:38
freed by thread T0 here:
    #1 ... in DaClear sandbox/ds-kit/ds-kit.c:43
```

Built plain, the same program never crashes — it prints a lie
(`first=date last=`) from stale memory, which is worse.

The fix picks a meaning for *clear*: drop the contents, keep the block —
`da->len = 0` and nothing more. Reusing the array now needs no allocation,
and the memory is still released later by `DaFree`. If you want a clear that
also releases memory, that is precisely `DaFree`, not a third thing. One more
wreckage from the clear: the driver's final read of `da.items[9]` is out of
bounds once only five items are present — ASan flagged it as a
heap-buffer-overflow. Indexing `da.items[da.len - 1]` instead of a literal
keeps reads inside the length. With both fixes ASan exits clean and the
output reads `first=date last=lemon`.
