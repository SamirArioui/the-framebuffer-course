# Solution: exercise 2 — The wrong size

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 010 — void*: genericity and its pain](../../lessons/part-0/lesson-010-void-pointer.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

The sanitizer names the crime immediately — a read far bigger than the
variable it reads from:

```
ERROR: AddressSanitizer: stack-buffer-overflow ...
READ of size 32 ...
    #0 ... in memcpy ...
    #1 ... in DaPush sandbox/ds-kit/ds-kit.c:43
    #2 ... in DemoLongs sandbox/ds-kit/ds-kit.c:107
```

`sizeof nums` is the size of `struct DynArray` — 32 bytes on this machine —
so the array declared its element stride to be 32, and the very first
`DaPush` copied 32 bytes out of the eight-byte `long v`, reading 24 bytes of
whatever followed it on the stack. Built plain, the same code often *appears*
to work: the overread finds plausible stack bytes and the program prints
garbage quietly.

The fix is one expression: `DaInit(&nums, sizeof(long))` — or, better,
`sizeof v` or `sizeof *p` style, which track the element's real type. And the
compiler was never consulted because there was nothing to consult: `DaInit`
takes a `size_t`, `sizeof nums` *is* a `size_t`, and the generic array has no
element type in its signature for the mismatch to violate. The stride is a
runtime number, checked by nothing. This is the `elem_size` failure mode of
the lesson's prose: silent, cheap to make, and only visible under a sanitizer
or a very bad day.
