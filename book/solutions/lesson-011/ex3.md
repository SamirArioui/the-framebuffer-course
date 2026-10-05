# Solution: exercise 3 — The walk's length

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 011 — the hashtable: hashing, buckets, lookup](../../lessons/part-0/lesson-011-hashtable.md).*

## The diff

```diff
{{#include ex3.patch}}
```

## Walkthrough

(a) A lookup walks one chain, and the expected chain length is the load
factor — `len / nbuckets`. A hit walks about half of it on average, a miss
all of it. With `len` words in `nbuckets` buckets, the expected probes are
therefore `len / (2 * nbuckets)` for hits — a constant as long as the load
factor is, which is what "O(1) on average" actually says. (b) A collision is
two different keys hashing to the same bucket; the chain stores both and
`strcmp` sorts out which is which. Hashes are a *routing* decision, never a
proof: two equal hashes say "look here", only equal keys say "found it" —
comparing hashes instead of keys merges distinct words forever. (c) The
degenerate case is every key colliding in one bucket — bad luck with few
buckets or an attacker choosing keys — and then every lookup is O(n). Real
tables keep the load factor bounded by growing the bucket array and
rehashing when it crosses a threshold, and some use stronger hash functions
so ordinary input cannot aim at one bucket.

The confirming check logs every lookup; over the exercise's two-line file the
get-or-put pattern is visible in the pairs of misses before each insert:

```
lookup the: miss after 0 probe(s)
lookup the: miss after 0 probe(s)
lookup cat: miss after 0 probe(s)
...
lookup the: 1 probe(s)
```

Two misses for a new word — `HtGet` looks, `HtPut` looks again before
inserting — and later hits cost one probe. Your explanation should predict
exactly this: probes scale with the load factor, and 8 keys in 1024 buckets
is a load factor of 0.008 — chains are barely chains at all.
