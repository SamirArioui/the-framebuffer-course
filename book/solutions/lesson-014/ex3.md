# Solution: exercise 3 — Structs do not make files

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 014 — endianness and image-header layout](../../lessons/part-0/lesson-014-image-headers.md).*

## The diff

```diff
{{#include ex3.patch}}
```

## Walkthrough

The probe prints, on this machine:

```
sizeof(struct)  56
offset type     0
offset file_size 4
offset data_offset 12
offset info_size 16
offset planes   28
offset bpp      30
```

Two disasters are visible. First, `sizeof` is 56, not 54: the compiler
inserted two padding bytes after the leading `unsigned short type` so the
`unsigned int` fields land on 4-byte boundaries. A raw `fwrite` of the
struct would therefore write 56 bytes and every field after `type` would
sit two bytes off — a decoder would read `data_offset` from byte 10 and
find the struct's reserved field there. Second, even without padding, the
*values* inside the struct are stored in the host's byte order, so a
big-endian machine would write big-endian fields into a little-endian
format. `#pragma pack` or `__attribute__((packed))` removes the padding,
but the byte-order problem and the dependence on compiler-specific magic
remain — the format's layout is the only layout that is allowed to matter,
and hand-rolled encoders are how you honor it. The extra credit answer:
the two missing bytes would shift the data offset field, so a decoder would
seek to the wrong place and interpret pixel data as header, or vice versa.
