# Solution: exercise 1 — Two fills, predicted

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 049 — the SIMD lens](../../lessons/part-2/lesson-049-simd.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

The prediction is where the exercise lives, so take it seriously. Both
loops write bytes to memory in order — the shape `CopySequential` got
vectorized for. But look at *what* they write:

- **`FillPattern`** writes four **different** bytes per pixel — `a`, `b`,
  `c`, `0` — repeating. A 16-byte vector move would need those four bytes
  duplicated four times inside one register: the compiler would have to
  build the pattern in a register (a shuffle or a constant load) and then
  store it. Possible in principle; at `-O3`'s cost model, unlikely. The
  prediction: **stays scalar**, four stores per pixel, like
  `ClearBuffer`.
- **`FillUniform`** writes **one** value, everywhere. That is not a copy
  and not a pattern — it is the textbook definition of `memset`. The
  compiler's pattern matcher looks for exactly this loop. The prediction:
  it will not stay a loop — it will become **a call to `memset`**.

The census, at `-O3`:

```
$ ./tools/disasm.sh --census | grep -E 'Fill|Copy|Blit|Clear'
    2  engine::CopyStrided(unsigned char*, unsigned char const*, unsigned long, unsigned long)
    2  engine::CopySequential(unsigned char*, unsigned char const*, unsigned long, unsigned long)
```

Neither fill is listed — **zero vector-register instructions in both.**
The listings say where each went:

```
$ ./tools/disasm.sh FillPattern
0000000000001c30 <engine::FillPattern(unsigned char*, unsigned long, unsigned char, unsigned char, unsigned char)>:
    1c30:	endbr64
    1c34:	test   %rsi,%rsi
    1c37:	je     1c5a <engine::FillPattern(...)+0x2a>
    1c39:	xor    %eax,%eax
    1c40:	mov    %dl,(%rdi,%rax,1)
    1c43:	mov    %cl,0x1(%rdi,%rax,1)
    1c47:	mov    %r8b,0x2(%rdi,%rax,1)
    1c4c:	movb   $0x0,0x3(%rdi,%rax,1)
    1c51:	add    $0x4,%rax
    1c55:	cmp    %rsi,%rax
    1c58:	jb     1c40 <engine::FillPattern(...)+0x10>
    1c5a:	ret
```

`FillPattern` is lesson 048's anatomy in miniature: four byte stores,
the stride-4 step, the test at the bottom. Scalar, exactly as predicted —
and exactly why `ClearBuffer`, which is this loop with different
arguments, is missing from the vector census too.

```
$ ./tools/disasm.sh FillUniform
0000000000001c60 <engine::FillUniform(unsigned char*, unsigned long, unsigned char)>:
    1c60:	endbr64
    1c64:	mov    %rsi,%rax
    1c67:	test   %rsi,%rsi
    1c6a:	je     1c80 <engine::FillUniform(...)+0x20>
    1c6c:	movzbl %dl,%esi
    1c6f:	mov    %rax,%rdx
    1c72:	jmp    1330 <memset@plt>
    1c80:	ret
```

`FillUniform` has **no loop**. The whole body is "set up three registers
and `jmp memset@plt`" — a tail call into the C library's `memset`, which
is itself hand-vectorized assembly inside glibc, written by people who
read many listings like these. The compiler did not vectorize your loop;
it *deleted* it in favor of a better one that already exists.

That is the fifth outcome, and the table of lesson 049 is now complete:
a loop can be **vectorized**, **partially vectorized**, **scalar but
register-allocated**, **scalar**, or **recognized and replaced**. The
census tells you *whether* vector registers are involved; only the
listing tells you *what actually runs*. Predicting the compiler is a
skill built exactly this way — write the prediction down, read the
listing, and when reality surprises you (as `jmp memset` surprises most
people the first time), the surprise is the lesson.
