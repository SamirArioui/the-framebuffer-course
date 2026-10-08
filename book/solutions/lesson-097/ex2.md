# Solution: exercise 2 — the transcript's bill

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 097 — pay the debt](../../lessons/part-5/lesson-097-debt.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

The diff is the tool, and the tool is the whole point: `git apply` it at
any state of the tree and it reduces a run's log to the shape a
refactor may change nothing of — the harness's chatter dropped, the
per-frame log lines dropped (their count is the pace's, not the game's;
the closing frame-budget table keeps the frame's cost), every digit
replaced so measured values stop differing, and repeated lines
collapsed so a report that fired once and one that fired a hundred
times read the same.

The measurement, in the order the exercise demands — the noise floor
**first**. Two runs of `lesson-096`'s build of the same scripted
scenario (built in a worktree at the tag, `git worktree add ../wt096
lesson-096`):

```
$ tools/transcript-normalize.sh before-1.log > before-1.shape
$ tools/transcript-normalize.sh before-2.log > before-2.shape
$ wc -l before-1.shape before-2.shape
  105 before-1.shape
  105 before-2.shape
$ diff before-1.shape before-2.shape
33d32
< engine: fire: spitter -> shell (damage N, range N)
42d40
< engine: hero velocity -N,N (t=N.N)
54a53,54
> engine: screen: death fade arrived at N,N,N (its own color)
> engine: screen: death: "GAME OVER" / "SCORE N TIME N:N WAVE N/N" / "ENTER: TITLE"
59d58
< engine: shell at N,N — N px of the hero (t=N.N)
64d62
< engine: shot shell retired — wall
77a76
> engine: state play -> death (the hero's health reached zero)
103a103
> engine: world: spark ends at N,N — N px of the hero
```

Eight templates apart — four in each direction — and every one of them
is the *fight* deciding itself: in the second run the hero died, so the
death screen appeared and the spitter's shell never flew. Two runs of
the same binary, and they differ. That is the floor; a refactor's bill
must be read against it.

Then the refactored build, the same scenario twice:

```
$ diff before-1.shape after-1.shape
68a69
> engine: hero unblocked at N,N (t=N.N)
77d77
< engine: hero unblocked at N,N (t=N.N)
$ for f in before-1 before-2 after-1 after-2; do
      sort $f.shape | md5sum; done
42c2b6b10349238e7840774698d52958  before-1.shape
30a2f5f2b7660514ecad5ab1d2268faf  before-2.shape
42c2b6b10349238e7840774698d52958  after-1.shape
42c2b6b10349238e7840774698d52958  after-2.shape
```

The verdict, with the floor in view: **the bill is zero.** The
refactored runs' report set is byte-identical to the first old run's —
the same 105 templates, every report the game knows how to give firing
in both — and the only sequential difference is one `hero unblocked`
probe landing a few lines earlier in the fight's sequence, which is
jitter of exactly the kind the old runs show against each other. The
old runs are eight templates from each other; the new runs are zero
templates from the old one. A refactor that changed behavior would not
survive this comparison — the fight's noise is not a hiding place,
because the *set* of reports (a state transition, a screen, a fade, a
wave, a hit, a burst) is the game's vocabulary, and the vocabulary does
not depend on timing.

One method note for the write-up you owe this exercise: report the
floor next to the verdict, always. "The transcripts matched" means
nothing until the reader knows what "matched" had to beat.
