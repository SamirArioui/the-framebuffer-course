# Solution: exercise 1 — Rows on disk

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 017 — writing a real image file by hand](../../lessons/part-0/lesson-017-image-file.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

The rows land bottom-up, one 24-byte row each, starting at the data
offset 54 the header promised — and `ftell` confirms the order:

```
row 5 -> file offset 54
row 4 -> file offset 78
row 3 -> file offset 102
row 2 -> file offset 126
row 1 -> file offset 150
row 0 -> file offset 174
```

`174 + 24 = 198`: the last row ends exactly at the file's end, and no
padding bytes were needed because 24 is already a multiple of 4. File
offset 54 is therefore image pixel `(0, 5)` — the bottom-left, gray
background — whose three bytes are `20 20 20` (gray reads the same in
BGR). Offset 75 is the eighth pixel of that same first row: image pixel
`(7, 5)`, the yellow end of the diagonal, stored BGR as `00 FF FF`. If
you predicted `FF FF 00`, you wrote RGB into the file — the per-pixel
swap in `WriteBmp` is exactly what turns our buffer's order into the
file's, and exercise 2 checks that swap from the reading side.
