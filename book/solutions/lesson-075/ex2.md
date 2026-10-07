# Solution: exercise 2 — The store's map

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 075 — the walk and the free slot](../../lessons/part-4/lesson-075-lifetime.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

The diff is one printer and three calls: `PrintStoreMap` walks the
store's slots and writes one character per slot — `#` for a live entity,
`.` for a free slot — and the run prints the map at the demo's three
moments. Sixty-four characters, one per slot, in slot order.

The maps, from a real run of this lesson's end state plus the patch:

```
engine: store map (world created): ########........................................................
engine: store map (after the walk): ##.#.#.#........................................................
engine: store map (after the reuse): ########........................................................
```

The first map is the world: the hero and seven entities from the script,
slots 0-7, everything else free. The second is the walk's work made
visible — slots 2, 4, and 6 have gone from `#` to `.`, exactly the three
the walk retired in passing, and nothing else moved. The third is the
reuse: the three requests landed on the three freed slots and the map is
`########` again — not `#########...` with the new entities out at 8,
9, 10.

Read the second map once more against the walk's report and the two say
the same thing two ways: `visited 8 live entities, once each, in slot
order` over the first map, `##.#.#.#` after. A count that says "5 live"
and a map that says *which* five are two different reports, and the map
is the one that catches the bug where the right number of entities is in
the wrong slots.

Now the question the map cannot answer. Both a slot that was freed and a
slot that has never been used read `.` — the store keeps only `live`,
because that is all the *policy* needs. The distinction comes from the
run's own history: the creation reports name the slot each entity landed
in (`created in slot 2` … `created in slot 6`), and this lesson's demo
knows it only ever created into slots 0-7 — so slots 8-63 have never
been touched. To show that in the map, the store would have to
*remember*: a `used` flag per slot, set at creation and never cleared
(so the three states are `live`, `freed`, `never used`), or a
high-water mark of the highest slot ever taken. Both are cheap and
neither is needed by the rules — the first-free-slot rule already
guarantees the order — but both make the difference *printable*, and a
difference you can print is a difference you can check.

If you added the `used` flag, one state is worth naming: a slot can go
`never used → live → freed → live …`, and **`never used` is the state a
slot never returns to**. That is not a loss: "never used" is a fact
about the past, and the policy only reads the present.

Nothing here touches the store, the walk, or the game's loop: the maps
are three lines beside the reports the lesson already prints.
