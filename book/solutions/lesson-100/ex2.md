# Solution: exercise 2 — what the word knows

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 100 — pass 2b: fix the clear](../../lessons/part-5/lesson-100-clear.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

The probe checks the argument at startup — the engine answering the
three claims about its own memory, on the real buffer:

```
engine: probe: word fill — pixels aligned to 4: yes, row stride ok; cleared 1,2,3 reads 1,2,3 at the corners and 1,2,3 in the middle; raw bytes 3,2,1,0
```

**Claim 1 — alignment: the word pointer may write this memory.** A
`unsigned int` store wants a 4-aligned address. Two facts guarantee
it, and the probe checks both: `GetFramebuffer` hands the pixel buffer
from the arena with a 4096-byte alignment (larger than the store
needs), and the row stride is `width × 4` — a multiple of 4 whatever
the width is, so every row starts aligned if the buffer does. The
probe prints `pixels aligned to 4: yes, row stride ok`. If either
were false — a framebuffer wrapping memory someone else chose — the
fix would need an aligned staging or a byte-store fallback: the fast
path is a *claim on the allocator*, not a fact of C++.

**Claim 2 — byte order: the word's bytes land where the byte stores
put them.** This is the claim the `memcpy` exists for. The engine
defines a pixel as four *bytes* in order — blue, green, red, zero —
and never says what an integer is. Composing the word by copying those
exact four bytes into an `unsigned int` lets the machine pick its own
representation; storing the word back to the same memory undoes the
representation exactly. On a little-endian machine the word for the
probe's `ClearBuffer(fb, 1, 2, 3)` is `0x00010203`; on a big-endian
one it would be `0x03020100` — and both produce the bytes `3, 2, 1,
0`, which is what the probe reads at the raw level. Had we instead
written `color = r<<16 | g<<8 | b` and assumed the layout, the code
would be faster to read and wrong on every machine that disagrees
with ours. The bytes are the contract; the word is an implementation
detail we let the compiler in on.

**Claim 3 — aliasing: the write may go through a different type.**
The buffer is arena memory — allocated storage with no declared type —
and a store through `unsigned int *` gives it that effective type;
reading bytes back through `unsigned char *` is legal regardless
(character types may inspect any object's representation). The probe
closes the loop by reading the same memory both ways: `GetPixel`
(as pixels: 1,2,3 at two corners and the middle) and the raw bytes
(`3,2,1,0` — exactly the byte stores' output). Reading both ways is
the argument: whatever the optimizer believes about types, the memory
says the same thing through both windows.

For a machine where a claim fails, the change is local: unaligned
memory → align the allocation or keep the byte stores (a correctness
floor below any speed); a byte order the `memcpy` already handles —
that claim *cannot* fail by construction, which is the whole reason
to compose the word that way; and for aliasing, the readback is
already the strongest statement portable C++ can make — a static
assertion about `sizeof(unsigned int) == 4` is the one guard worth
adding if you want the machine to refuse a word that is not four
bytes wide.

One honest footnote: the probe clears the framebuffer at startup —
the next frame's clear overwrites it, so the run is unaffected, but a
probe that writes should say so (this one does, in its comment), and
the transcript's shape check in a later lesson will see the probe's
line as one more template. Probes are allowed to touch; they are not
allowed to be quiet about it.
