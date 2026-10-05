# Solution: exercise 2 — The leaderboard

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 011 — the hashtable: hashing, buckets, lookup](../../lessons/part-0/lesson-011-hashtable.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

The mode adds a second ordering policy and a stopping point. The new
comparator, `CmpByCountThenKey`, orders by count — most frequent first —
and falls back to `strcmp` on the keys whenever counts tie. That fallback is
what makes the output reproducible: two entries can never be equal on *both*
keys, so the comparator never returns zero and the sorted permutation is
unique — `qsort`'s instability cannot leak through. Real runs:

```
$ ./ds-kit --top 3 shakespeare.txt
to 3
be 2
the 2
```

`be` beats `the` because both have count 2 and `be` comes first
alphabetically. On a file where everything ties at 1, the leaderboard is
alphabetical:

```
$ ./ds-kit --top 3 two-lines.txt
the 3
and 1
bird 1
```

The printing stop is a plain loop over `DaAt` up to `N` (clamped to `len`),
because `DaEach` walks everything — iteration hooks with early exit would
need a "keep going?" return, another callback-shaped idea. The plain mode is
untouched: same comparator, same `DaEach`, same lines. One parsing note:
`strtol` earns its keep over `atoi` by being able to reject bad input — the
exercise keeps the simple form, but anything longer-lived should check where
the number ended.
