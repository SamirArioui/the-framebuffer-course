# Solution: exercise 2 — Table statistics

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 012 — multi-file builds: translation units and linking](../../lessons/part-0/lesson-012-multi-file.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

The change crosses all three layers of the header contract: the declaration
in `hashtable.h`, the definition in `hashtable.c` (which walks the chains —
its translation unit is the one that includes `dynarray.h` and can touch
`ht->chains[b].len`), and one call in `main.c`. The compiler checks the call
against the declaration and the definition against it too; a mismatch in
either direction is a diagnostic before the linker is ever involved. Real
runs on two files:

```
stats: entries=14 buckets=1024 empty=1010 longest=1
stats: entries=10000 buckets=1024 empty=0 longest=21
```

The first line is the two-line exercise text: 14 keys in 1024 buckets means
1010 empty buckets and chains of at most one — a load factor of 0.014. The
second is the 300,000-word benchmark text of lesson 011's exercise 4: 10,000
entries, no empty bucket left, longest chain 21 against an average near 10 —
the spread you expect when keys hash unevenly but not pathologically. Those
four numbers are lesson 011's load-factor story reduced to one line, and
they are exactly what you check before blaming the hash function. The
report goes to `stderr`, so the sorted counts on `stdout` stay clean —
same habit as every diagnostic since lesson 001.
