# Lesson 070 — the mix's cost in the frame budget

{{#include ../../stability-horizon.md}}

## Prose

The instrumentation lesson 060 planted was for exactly this: the
`audio` phase has been measured inside every frame since the sound
started — summed in the account, named in every `frame N:` line — and
this is where the budget table finally shows it. The row is **measured,
not guessed** — that is this lesson's title and its rule. No number in
the row was estimated, benchmarked apart, or copied from a datasheet;
each one is a sum over the frames this run actually executed, divided
by their count, like every row above and below it.

### The table

The demo runs, and at the end the account prints as a budget — with the
sound's row at last, between `update` and `render`, where the phase
sits in the frame:

```
engine: demo: 580 frames measured, 580 buffers fed, 25 effects fired, 3 music wraps
engine: frame budget — 580 frames, avg 2.302 ms, worst 3.803 ms (frame 538)
engine:   subsystem   avg ms    share
engine:   update       0.001       0%
engine:   audio        0.045       2%
engine:   render       1.523      66%
engine:     sprites    0.001       0%
engine:     text       0.008       0%
engine:     tilemap    1.047      45%
engine:   present      0.732      32%
engine:   total        2.302     100%
```

The row's *place* is the record's own order. The frame is measured
`update`, `audio`, `render`, `present` — that is the order the `frame
N:` line lists them in, and the table now reads the same way. It would
have been just as easy to print the row at the bottom, near `present`,
where the other machine-bound row lives; the table refuses, because the
rows are the frame's phases in the order the frame runs them. A table
that reorders its rows is a table that starts lying about what happens
when.

Everything else about the row is the table's existing rules, exactly:

- **the row is a measured sum** — `audio_sum` over the run's frames
  divided by their count, like `update_sum` and `render_sum` before it;
- **its share is of the average frame** — `0.045 / 2.302` is 1.96%,
  printed as 2%;
- **the header is unchanged** — how many frames were measured, what
  they cost on average, and the worst one by number.

### What the row includes

The `audio` row wraps **the mix *and* its submit** — every clock read
around the run's whole audio step, from the buffer's first channel pull
to the device taking the finished stream. That is why the row is
trustworthy: it measures the phase as the frame pays for it, not the
part that is easy to time.

What the row *reads as* depends on the device, and the two cases are
worth saying plainly:

- **on this machine the submit returns immediately.** ALSA's `null`
  device takes the samples and discards them — measured in the notes
  for this part: five 100 ms buffers accepted in 0.0 ms wall time, at
  every buffer size tried. So the row here reads as **the mix's cost**:
  0.045 ms for 735 output frames across all sixteen channels, about
  11,760 channel pulls — around 4 ns a pull.
- **on real hardware a submit can wait** for room in the device's
  buffer, and the row then carries that wait. This is not a new shape:
  `present` has always included the copy's sync for the same reason —
  the row is the phase as the frame pays it. Exercise 1 splits the row
  so you can see which half is which on your machine.

The phase's worst frames say the same thing from the other side. Over
this run the `audio` numbers read floor 0.026 ms, average 0.045 ms,
worst 0.644 ms — and the worst is not the mix: that frame's render and
update are up too (a whole-frame hiccup of the machine, on a run that
otherwise holds 0.03-0.05 ms). The row is honest about its worst the
same way `present` is — it does not trim its outliers before averaging
them.

### The frame's arithmetic closes again

Lesson 058's prose said the frame's arithmetic closed — `update +
render + present = total`, the three phases accounting for the frame.
Then lesson 060 grew a fourth phase, and the table kept printing three
rows: the frame's own log line showed `audio` in every record while the
budget's rows summed to less than the budget's own total. The gap was
never a mystery — it was named and dated ("the table's rows cover
`update + render + present` and not the whole frame; lesson 070 is the
close's job") — but it was a gap. This row closes it:

```
0.001 + 0.045 + 1.523 + 0.732 = 2.302
```

`update + audio + render + present = total`, to the printed precision.
In the record's own sums the closure is even tighter: the phases
average to 2.302252 ms against the total's 2.302274 ms — a residue of
0.000022 ms, which is the measurement itself (the clock reads around
the phases), exactly the residue lesson 058 named.

And the gap this row closes is visible in the same table with the row
taken back out: `0.001 + 1.523 + 0.732 = 2.256` of the 2.302 — the
rows short of the total by 0.046 ms as printed, 0.045 ms in the sums'
own precision. That difference *is* the audio phase. Three rows did not
describe this frame; four do.

### Checked against the run's own log

Every number above is checkable against the run's `frame N:` lines —
and the authoring check for this lesson did exactly that: averaging the
phases over the run's 580 frame lines reproduces the table row for row.

| Row | Printed | Averaged from the log |
| --- | ------- | --------------------- |
| update | 0.001 | 0.001 |
| **audio** | **0.045** | **0.045** |
| render | 1.523 | 1.523 |
| sprites | 0.001 | 0.001 |
| text | 0.008 | 0.008 |
| tilemap | 1.047 | 1.047 |
| present | 0.732 | 0.732 |
| total | 2.302 | 2.302 |

The frame lines it was reconciled against look like this — the audio
number is the one the new row sums:

```
frame 1: update 0.001 ms, audio 0.033 ms, render 1.738 ms (sprites 0.001, text 0.006, tilemap 0.914), present 0.751 ms, total 2.521 ms
frame 2: update 0.001 ms, audio 0.033 ms, render 1.522 ms (sprites 0.001, text 0.007, tilemap 1.123), present 0.807 ms, total 2.363 ms
frame 3: update 0.001 ms, audio 0.034 ms, render 1.409 ms (sprites 0.001, text 0.008, tilemap 0.977), present 0.872 ms, total 2.316 ms
```

A budget whose numbers disagree with its own ledger is decoration; this
one is the ledger — now for the sound too.

### The format Part 5 grows

Lesson 058 called this table a *first draft* and said the finale grows
it — naming "sound" as the example of a row arriving when its
subsystem exists. It has arrived, and what it adds is exactly what was
promised: **one more named phase, measured like every other**. What
this row does *not* change is the part that matters:

- rows are measured sums over the run's frames — never estimates;
- shares are of the average frame;
- the header carries count, average, and worst;
- a phase's named insides live inside its row, never instead of it.

Those rules are what make the numbers mean what they say, and they are
unchanged since lesson 058 wrote them down. What the finale adds beside
them — before/after columns for the three-pass optimization menu, the
machine and build flags named with the numbers — layers on top. The
sound's row is the proof that a new phase can join this table without
negotiating with it.

## Code step

One change for this lesson: `frame.cpp`'s `PrintFrameBudget` grows the
`audio` row — the phase the record has carried since lesson 060, summed
by the same account, now printed in the record's own order between
`update` and `render`, under the table's unchanged rules. Nothing else
moves: the demo, the record's fields, the log line's format, and the
account are lesson 069's, untouched. Its end state is tagged
`lesson-070`.

```diff
diff --git a/src/frame.cpp b/src/frame.cpp
index d15549b..6868e33 100644
--- a/src/frame.cpp
+++ b/src/frame.cpp
@@ -39,11 +39,12 @@ void PrintFrameBudget(const FrameStats &stats)
                 stats.worst_number);
 
     /* The attribution: every row a measured sum, every share of the
-       average frame. The named phases live inside render — they say
-       where it went, they do not replace it. The audio phase (lesson
-       060) is summed like the rest but gets no row here: the table's
-       sound rows are lesson 070's, and that is deliberate. */
+       average frame. The phases take the record's own order — update,
+       audio, render, present — and the named phases live inside render:
+       they say where it went, they do not replace it. Lesson 070: the
+       audio phase, measured since lesson 060, gets its row at last. */
     double update = stats.update_sum / n * 1e3;
+    double audio = stats.audio_sum / n * 1e3;
     double render = stats.render_sum / n * 1e3;
     double sprites = stats.sprites_sum / n * 1e3;
     double text = stats.text_sum / n * 1e3;
@@ -52,6 +53,8 @@ void PrintFrameBudget(const FrameStats &stats)
     std::printf("engine:   subsystem   avg ms    share\n");
     std::printf("engine:   update      %6.3f      %2.0f%%\n", update,
                 100.0 * update / (avg * 1e3));
+    std::printf("engine:   audio       %6.3f      %2.0f%%\n", audio,
+                100.0 * audio / (avg * 1e3));
     std::printf("engine:   render      %6.3f      %2.0f%%\n", render,
                 100.0 * render / (avg * 1e3));
     std::printf("engine:     sprites   %6.3f      %2.0f%%\n", sprites,
```

## Exercises

Two mixed exercises. Each ends with its solution — a diff against this
lesson's end state plus a walkthrough — after the prompt.

### Exercise 1 — The row's two insides *(extend-the-code)*

The `audio` row wraps two things: the mix and its submit. Split the row
the way render is split — name the two insides in the frame record,
carry them in the `frame N:` line, and print them as indented rows
under `audio`, inside it rather than instead of it. Run the demo and
reconcile: the two rows add up to the audio row exactly the way the
named phases explain render. Then answer the row's question: the
submit's inside reads nearly nothing on this machine — what does it
carry on hardware that pushes back, and which row already in the table
is its structural twin?

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-070/ex1.md)

### Exercise 2 — The row's two populations *(measure-the-performance)*

The table's row is one average over every frame, but the frames are not
all alike: a frame that fed a buffer mixes 735 output frames across all
sixteen channels; a frame that fed nothing mixes none. Separate the two
populations — count them and keep their audio sums apart — and report
each one's average from a real run. Reconcile the feeding frames
against the work one buffer is, and the whole-run average against the
table's row; say why the two agree, in terms of the populations' sizes.
If your run has only one population, say why from the paced wait — and
what it would take to see the other. Finish with the budget's question:
what share of the worst frame did the phase take, how does that differ
from its share of the average frame, and what would make this row the
first one to grow?

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-070/ex2.md)

---

**Part:** [Part 3 — sound](../../index.md) ·
**Previous:** [Lesson 069 — the closing demo](lesson-069-demo.md) ·
**Next:** — ·
**Code tag:** [`lesson-070`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-070)
