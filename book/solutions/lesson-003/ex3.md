# Solution: exercise 3 — The longest word

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 003 — char buffers: strings by hand](../../lessons/part-0/lesson-003-char-buffers.md).*

## The diff

```diff
{{#include ex3.patch}}
```

## Walkthrough

The pass already walks every byte and already knows when a word starts —
`in_word` is half of the machine. The other half is a `word_len` counter
that ticks up on non-whitespace bytes and is compared against `wlongest`
wherever a word can end: at whitespace, at a newline, and once more after
the loop for a trailing word that meets EOF. The struct gains the member
and `printf` gains a column:

```
$ ./wordcount story.txt
2 4 21 11 5 story.txt
$ ./wordcount a.txt b.txt
1 1 3 2 2 a.txt
1 1 6 5 5 b.txt
$ ./wordcount partial.txt
0 2 10 10 7 partial.txt
```

`story.txt`'s longest word is `world` (5); `partial.txt`'s is `newline` (7)
— including a word that ends at EOF, the case the after-loop check catches.

Now `long.txt`, the 300-character line: `1 1 301 255 300 long.txt`. The new
column says 300 — the truth — while `longest` says 255. The word counter
ticks on every stream byte; only the line *buffer* has a ceiling. Same pass,
same file, and the two columns disagree because one of them passes through
`line[256]`. That is the ceiling from exercise 1, seen from a second angle.
