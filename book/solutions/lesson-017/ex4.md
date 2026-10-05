# Solution: exercise 4 — Per byte versus per row

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 017 — writing a real image file by hand](../../lessons/part-0/lesson-017-image-file.md).*

## The diff

```diff
{{#include ex4.patch}}
```

## Walkthrough

`WriteBmpFast` does the same work — BGR swap and padding included — but
into a row buffer emitted with one `fwrite` per row. On the 1024×1024
gradient (3.1 MB of pixels), five repetitions each, this machine reports

```
putc per byte: 7.7 ms/write
fwrite rows:   4.6 ms/write
```

with runs landing in the 6.8–8.7 and 4.3–4.9 ms bands: the row writer is
around 1.6× faster, and `cmp bench1.bmp bench2.bmp` confirms the files
are byte-identical. Where does the time go? Not on disk — the page cache
absorbs both. The per-byte writer makes 3.1 million `putc` calls, each a
function call with an argument check and a buffer bookkeeping step; the
row writer makes a thousand `fwrite`s plus the same swap loop. The gap is
call overhead, not memory bandwidth — which is also why it is a modest
1.6× rather than the 10×-plus folklore number from the unbuffered stdio
days. Measure on your machine; if `fwrite` is not ahead there, the
measurement is worth reading twice.
