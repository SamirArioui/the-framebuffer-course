# Solution: exercise 1 — Three hundred characters

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 003 — char buffers: strings by hand](../../lessons/part-0/lesson-003-char-buffers.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

The prediction that trips people: `1 1 301 300 long.txt`. The real rows:

```
$ ./wordcount long.txt
line: 255
1 1 301 255 long.txt
$ wc -l -w -c long.txt
  1   1 301 long.txt
```

`lines`, `words`, and `bytes` are counted straight from the stream and match
`wc` — the file really is one line, one 300-character word, 301 bytes with
the newline. `longest` reports 255 because that is all the buffer can
*hold*, and the instrumenting print shows where the information dies: the
line is measured at `line: 255`, the guard having silently dropped the
characters after the 255th. The rule behind it: `line` reserves one byte of
its 256 for the NUL, so `if (len < sizeof line - 1)` stops storing at 255
characters. The program is following its own rules exactly — its rules just
cannot see past the buffer. The scan itself is innocent: every byte of the
line is read and counted; only the *copy* is capped. Lesson 004 replaces
the cap with a buffer that grows.
