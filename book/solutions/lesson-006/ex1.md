# Solution: exercise 1 — The value that is not promised

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 006 — undefined behavior and buffer overflows](../../lessons/part-0/lesson-006-undefined-behavior.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

The predictions: the checker says something; both builds print the same
number. Reality:

```
$ ./wordcount-san story.txt
wordcount.c:136:5: runtime error: signed integer overflow: 2147483647 + 1 cannot be represented in type 'int'
big = -2147483648
2 4 21 11 story.txt
$ ./wordcount-plain story.txt
big = -2147483648
2 4 21 11 story.txt
```

`-fsanitize=undefined` names the operation and the type that cannot hold
the result — that line is the language talking, not the hardware. The
printed value is what *this* build does: the machine's add wraps, and `-O0`
hands you the wrapped bits. The plain build's silence is the real lesson:
nothing crashed, nothing warned, and if you shipped a program that relied
on `big` being `-2147483648`, it would be relying on an observation with
no contract behind it. Undefined behavior is not a kind of wrong answer —
it is the absence of an answer, and a compiler is entitled to assume
`++big` never overflows (no valid program does that) and to transform the
code around it accordingly. The fix for real code is never "count on the
wrap": use a wider type, check before you add, or make the limit explicit.
