# Solution: exercise 2 — The load's cost, measured

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 072 — the load, complete or named](../../lessons/part-4/lesson-072-load.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

The diff is the measurement and nothing else: four loads of the same
loader at four table sizes, each timed on the platform clock —
`platform::Now()` around the call, the same instrument the frame record
is built on — and accounted in the arena. The four files are scratch
inputs you generate once, outside the run:

```
$ for n in 2 20 200 2000; do { printf 'name x y facing speed health sprite\n'; i=0; while [ $i -lt $n ]; do printf 'def%d %d %d %d %d %d assets/sprite.ppm\n' $i $((i%480)) $((i%320)) $((i%4)) $((60+i%200)) $((1+i%10)); i=$((i+1)); done; } > assets/rows$n.txt; done
```

(They are yours to delete afterwards; nothing in the engine names them
except this probe.)

The numbers, from a real run of this lesson's end state plus the patch on
the authoring machine:

| file | rows | arena growth | load |
| ---- | ---- | ------------ | ---- |
| `assets/rows2.txt` | 2 | 200 bytes | 0.0041 ms |
| `assets/rows20.txt` | 20 | 2000 bytes | 0.0088 ms |
| `assets/rows200.txt` | 200 | 20000 bytes | 0.1561 ms |
| `assets/rows2000.txt` | 2000 | 200000 bytes | 10.4368 ms |

Two things to read before the third. The **arena growth is exact** at
every size: `rows × sizeof(EntityDef)` — one hundred bytes a
definition, no capacity and no waste. And the **time is not linear**:
ten times the rows costs twice the time at the small end (0.0041 →
0.0088 ms), eighteen times in the middle (0.0088 → 0.1561 ms), and
sixty-seven times at the top (0.1561 → 10.4368 ms). A linear load would
read 10× at every step.

Where the time goes, with the code in front of you. Both walks are
linear — one pass to count, one to fill — and the value parsers touch
each byte once. The part that is not linear is the duplicate-name check:
every row's name is compared against the names of all the rows before it,
so a table of *n* rows makes about *n²/2* string comparisons. At twenty
rows that is a couple of hundred comparisons and the load is done in
microseconds; at two thousand it is two million comparisons, and they are
the load. The step from 0.1561 ms to 10.4368 ms is that check arriving.

Would you change anything for this game's tables? **No — and the numbers
say why.** A table holds a game's *definitions*, not its entities: the
hero, three enemy types, the boss, a weapon or two — a dozen rows, two at
most in the tens. At those sizes the load is a few microseconds and the
quadratic term is not measurable against the file read. The entities
themselves are not rows of this table at all — they are the store's
business, lesson 074 — and *that* is where thousands of live things
belong.

What would your answer be at ten thousand rows? Measured, it would be
roughly 250× the 2000-row number — a quarter of a second spent in the
name check alone — and at that point the check changes, not the format:
sort the names once after the fill and compare neighbors, or check them
in the fill against a small fixed table of hashes. Both keep the rule
(one name, one definition) and drop the cost to linear or linearithmic.
The habit that matters is the one this exercise started with: the
change is justified by a measurement you took, and re-measured after,
never by the shape of the curve on a napkin.

Nothing here touches the loader, the format, or the game: the probe is
four loads beside the run's own, and the files it reads are yours to keep
or delete.
