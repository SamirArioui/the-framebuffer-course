# Solution: exercise 1 — The reversed order

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 009 — function pointers: comparators and hooks](../../lessons/part-0/lesson-009-function-pointers.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

The prediction: `sorted by key:` comes out in reverse alphabetical order —
and that is exactly what the machine prints:

```
sorted by key:
pear 0
lemon 9
kiwi 8
grape 7
fig 2
elder 6
date 5
cherry 4
banana 3
apple 1
```

The subtlety asked about in the prompt: it is *precisely* the reversed block,
because all ten keys are distinct. Swapping the arguments turns the
comparator into its mirror image, which is still a consistent total order —
just a mirrored one — so `qsort`'s contract is satisfied and there is exactly
one valid sorted permutation to produce. The `sorted by value:` block
reverses too, for the same reason: the bridge is shared by every sort.

Had two items shared a key, "merely reversed" would no longer be guaranteed:
`qsort` is not a stable sort, and among equal elements any order that
satisfies the comparator is legal. The lesson inside the prediction: a
comparator fully defines the answer only when its keys are unique.
