# Solution: exercise 4 — One bucket

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 011 — the hashtable: hashing, buckets, lookup](../../lessons/part-0/lesson-011-hashtable.md).*

## The diff

```diff
{{#include ex4.patch}}
```

## Walkthrough

The patch makes the bucket count a command-line argument — default 1024 —
so both sides of the experiment need no recompiling. The text here is a
generated file: 300,000 word tokens drawn from a vocabulary of 10,000
distinct words (the file's `wc -w` and the counter's output-line count give
both numbers directly). Measured on the author's machine:

```
NBUCKETS=1024   real 0.03s
NBUCKETS=64     real 0.08s
NBUCKETS=1      real 4.07s
```

The one-bucket run is about 135 times slower, and the ratio is the chains'
average length made visible. With 10,000 distinct words in one bucket, every
lookup walks a chain of up to 10,000 entries — about 5,000 `strcmp`s on
average, some 1.5 billion for this file. At 1024 buckets the average chain
holds about ten entries and a lookup probes a handful. The wall-clock ratio
is milder than the probe ratio because every run pays the same fixed costs —
tokenizing 300,000 characters, hashing, growing the chains — and only the
walking part scales.

Two footnotes from the measurements. The output was byte-identical across
bucket counts (`diff` clean): buckets change *where* a key lives, never the
answer. And the 64-bucket run in between shows the curve is graded, not
binary — this is a dial from "hash table" to "list", and the load factor
reads the dial.
