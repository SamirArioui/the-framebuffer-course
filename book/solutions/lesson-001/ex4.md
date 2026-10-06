# Solution: exercise 4 — Bytes, not characters

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 001 — argv and file input: your first `gcc` command](../../lessons/part-0/lesson-001-first-program.md).*

## The diff

```diff
{{#include ex4.patch}}
```

## Walkthrough

The prediction: `./wordcount five.bin` prints `5 five.bin`, and `wc -c
five.bin` agrees — `5 five.bin`. The file holds five bytes, and the program
counts bytes. On this system that is all there is to it.

The instrumented loop is where the lesson hides. It prints, in order:

```
c=72
c=105
c=0
c=10
c=255
```

`72` is `H`, `105` is `i`, `0` is the NUL byte, `10` is the newline, and
`255` is `0xFF` — five values, then the loop stops without printing anything
for the end of the file. That last value is exactly why `fgetc` returns
`int` and not `char`. If it returned `char`, the value `0xFF` would arrive
as `-1` on any system where plain `char` is signed — the same number as
`EOF` — and this five-byte file would be counted as four bytes. The extra
width of `int` keeps the 257 possible results of "one byte" separate from
the one value that means "nothing left".

The loop condition is safe as written for the same reason: `c` is an `int`,
so every byte value — including `0x00` and `0xFF` — survives the comparison
against `EOF` intact. If you ever see C code storing `fgetc`'s result in a
`char`, it is carrying this bug.
