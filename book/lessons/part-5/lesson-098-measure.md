# Lesson 098 — pass 1: measure

{{#include ../../stability-horizon.md}}

## Prose

The game is done; the optimization begins; and the frozen menu
(`target-game`, design D11) says exactly how it begins — **measure**.
Not "look at the code and guess what is slow". Not "optimize the parts
that feel heavy". Instrument, profile, and *name* the top-2 hotspots —
with numbers from real frames — because the next two lessons fix
exactly those two and nothing else. Anything this pass does not name is
recorded as future work and left alone.

### The instruments, and this machine's honest limits

Two instruments, and only two, because this machine has only two. The
**frame account** — `FrameRecord`, `FrameStats`, `PrintFrameBudget`
(`frame.h`, `frame.cpp`) — measures every frame's phases on the
platform clock and keeps the running sums; it is the built-in
instrument and the source the final report grows from. And **`gprof`
via a `-pg` build** — the function-level profiler, the one the deep
dives' tooling was built to read beside.

What this machine does **not** have, named so the measurement never
pretends otherwise: `perf` (and `perf_event_paranoid` is `2`, so it
would be restricted even if installed), `valgrind`/callgrind, `ltrace`,
`strace`. What it is: WSL2, Xvfb on `:99`, no sound hardware — so the
run mixes its audio in silence and the display's copy goes through the
X server (design D12: every number in this lesson carries that
machine's name). And one shape fact that governs the scenario: the
loop is **event-driven** — it wakes on X news or on the audio feed's
deadline — so a run measures as many frames as the pacing gives it.
The runs below are paced at ~25 fps by streaming alternating
window-move events (`xdotool windowmove`), as the earlier batches did.

### The instrument's one gap, closed first

Before profiling, the account itself gets its instrument. The frame
log attributes render's sub-phases — sprites, text, tilemap — but the
render's *first* work, the framebuffer's clear, was never named: it was
the unnamed remainder sitting between the `render` row and its
sub-phases' sum. A measure pass that leaves a cost unnamed cannot hold
it against anyone. So the code step names it: `FrameRecord` grows
`clear`, the loop times the clear where it runs, and the account's
table and the frame log carry the row. The run says what it always
said, plus one name:

```
engine: frame budget — 1551 frames, avg 1.857 ms, worst 3.137 ms (frame 646)
engine:   subsystem   avg ms    share
engine:   update       0.013       1%
engine:     entities   0.005       0%
engine:   audio        0.032       2%
engine:   render       1.370      74%
engine:     clear      0.458      25%
engine:     sprites    0.006       0%
engine:     text       0.012       1%
engine:     tilemap    0.895      48%
engine:   present      0.442      24%
engine:   total        1.857     100%
```

### The measurement run

The scenario is the game being played, and playing is not a straight
line: the hero dies, and a dead game is a title screen. So the
measurement run drives real play on purpose — `Return` into play, then
six ten-second legs of walking and firing (right, left, right, …),
each leg followed by two `Return`s that restart the game if the hero
died and do *nothing* if he did not (play ignores `Return`; death →
title → play takes two presses). 1,551 frames at ~25 fps over sixty-odd
seconds, one death, five restarts.

And the state's mix matters, so the run is read twice — once whole, and
once split by the record's own `step` field (a frame that advanced game
time was playing; a frame at `step 0.000` was showing a screen). The
split, computed from the frame log's own lines — 1,415 play frames and
136 panel frames:

```
play  (1415 frames): update 0.013  entities 0.005  audio 0.031
                      render 1.455  clear 0.456  sprites 0.006
                      text 0.012  tilemap 0.981  present 0.444  total 1.944
panel ( 136 frames): update 0.008  entities 0.006  audio 0.037
                      render 0.488  clear 0.473  sprites 0.000
                      text 0.015  tilemap 0.000  present 0.427  total 0.961
```

Read that pair once and the lesson of the mix is learned: the `clear`
costs the same on every frame (every frame starts by painting 307,200
pixels), the `tilemap` only on play frames, and the whole-run table's
`0.895` is just the weighted mix of `0.981` and `0.000`. Averages
answer "what did this run cost", not "what does the game cost" — which
is why the hotspots are named from the play frames.

### The profile

The `-pg` build, the same scenario, ten legs: 2,583 frames, the frame
account reading `avg 1.834 ms (tilemap 0.835, clear 0.443, present
0.433)` from the very run the profile was written on. `gmon.out` on
close; `gprof build-pg/game gmon.out` — 3.53 seconds of CPU sampled in
0.01-second grains:

```
  %   cumulative   self              self     total
 time   seconds   seconds    calls   s/call   s/call  name
 59.77      2.11     2.11  3501190     0.00     0.00  engine::BlitSprite(engine::Framebuffer&, engine::Sprite const&, int, int)
 37.68      3.44     1.33     2584     0.00     0.00  engine::ClearBuffer(engine::Framebuffer&, unsigned char, unsigned char, unsigned char)
  1.13      3.48     0.04     2204     0.00     0.00  engine::DrawTileMap(engine::Framebuffer&, engine::TileMap const&, engine::TileSheet const&, int, int)
  0.57      3.50     0.02    14745     0.00     0.00  engine::BlitSpriteFrame(engine::Framebuffer&, engine::Sprite const&, int, int, int, int)
  0.28      3.51     0.01 29905680     0.00     0.00  engine::(anonymous namespace)::ChannelFrame(engine::Channel&)
  0.28      3.52     0.01     2584     0.00     0.00  platform::Present(platform::Window*, unsigned char const*, int, int)
  0.28      3.53     0.01     2543     0.00     0.00  engine::MixBuffer(engine::Mixer&, short*, int)
```

Two instruments, one verdict — and the call counts say exactly whose
cost is in that first line: `BlitSprite` was called 3,501,190 times,
and `DrawTileMap` walks 2,204 maps × 1,536 cells = 3,385,344 of them —
**96.7%**. The remaining 115,846 are the glyphs (exactly the
`FontGlyph` call count) riding the same loop, 3% of its calls.

### The top-2 hotspots, named

**Hotspot #1 — the map's draw.** `DrawTileMap`'s walk, one `BlitSprite`
per cell. On a play frame it is `tilemap 0.981 ms` of `1.944 ms` —
**half the frame**; in the flat profile `BlitSprite` + `DrawTileMap`
are **2.15 s of the 3.53 s** sampled CPU (60.9%). This is the cost of
drawing 1,536 tiles, every one of them through the sprite blit's
per-pixel loop.

**Hotspot #2 — the frame's clear.** `ClearBuffer`, one call a frame,
every frame. It is `clear 0.456 ms` of a play frame (23%) and `0.473
ms` of a panel frame — **49% of every frame that draws no map**; in the
flat profile it is the second line, **1.33 s (37.7%)**. This is the
cost of painting 307,200 pixels four bytes at a time.

Both hotspots are names, not moods — the next two lessons fix exactly
these two (`BlitSprite`'s copy as the map walks it in 099, `ClearBuffer`
in 100) and nothing else.

### Named, and not the menu's: the seam, and the rest

One measured cost is bigger than hotspot #2's share of a play frame and
is *not* on the menu: `present`, `0.444 ms` a frame (23%). Look at its
profile line — `platform::Present`, `0.01 s` of CPU in 2,583 frames,
0.28%. Both numbers are true: the present's wall time is **wait**, not
computation — the process hands pixels to the X server and waits for
the copy to be taken. It is the seam's cost (the platform layer's
`XPutImage`), not engine work the deep dives' levers can shorten, and a
double-buffered or MIT-SHM presentation is a *platform-layer* change
the frozen menu does not make. It goes on the future-work list, named
and measured:

- **the presentation's copy** — `present` 0.444 ms/frame wall, 0.004
  ms/frame CPU: the seam's wait on the X server (a second OS
  implements this file; a faster present is its change).
- **the audio mix at full load** — `audio 0.031 ms` with the music and
  a handful of effects; a firestorm across all 16 channels is unmeasured.
- **the update's per-entity work at full store** — `entities 0.005 ms`
  at a handful of live entities; a full 64-slot store of sparks is
  unmeasured.
- **the reports' own printing** — the probes and the HUD's report print
  inside `update` and `text`, and the frame log's own line prints
  outside every phase; a quieter run would measure cheaper phases.

### What this run verified, and what it did not

- **The top-2 are named with numbers from real frames** — 1,415 play
  frames of the instrumented build and 2,583 frames of the profiled
  one, both on this machine (WSL2, Xvfb `:99`, no sound hardware), the
  frame account and `gprof` agreeing on the names *and* the order.
- **The instrument's gap is closed** — the `clear` row is measured
  (`0.458 ms` over the mixed run), and the whole-run table's numbers
  now add up: `clear + sprites + text + tilemap` is render, with no
  unnamed remainder.

What this run did **not** verify: the *shipping* build's numbers. Every
measurement here is the course's own build — `-O0`, and the profile's
build adds `-pg`'s bookkeeping on top — so the absolute milliseconds
are this build's, not `-O3`'s (lesson 049 priced that gap: a change of
several times). The hotspots' *names* are what the menu fixes on, and
the deep dives' levers — copy less, copy closer together, copy wider —
are exactly what still applies at any optimization level. Nor is the
profile fine-grained: 353 samples in 0.01-second grains put each line
within a percent or two, and a longer run would firm them up. And
nothing here is 60 fps evidence: a paced headless run at ~25 fps is a
measurement rig, not the modest hardware of the checklist — that claim
is the finale's to check, as far as this machine honestly can.

## Code step

One change: the instrument's gap. `src/frame.h` grows `FrameRecord`
and `FrameStats` by one name — `clear`, the framebuffer's clear, the
render's first work — and `src/frame.cpp`'s `AccountFrame` and
`PrintFrameBudget` sum and print it as the first of render's
sub-phases. The loop times the clear where it already runs and the
frame log's `render` list gains the field, first, where it happens in
the frame. Nothing else moves: the measurement is the change. Its end
state is tagged `lesson-098`.

```diff
diff --git a/src/frame.cpp b/src/frame.cpp
index 444b1c0..349af4f 100644
--- a/src/frame.cpp
+++ b/src/frame.cpp
@@ -19,6 +19,7 @@ void AccountFrame(FrameStats &stats, const FrameRecord &frame)
     stats.sprites_sum += frame.sprites;
     stats.text_sum += frame.text;
     stats.tilemap_sum += frame.tilemap;
+    stats.clear_sum += frame.clear;
     stats.entities_sum += frame.entities;
     if (frame.total > stats.worst) {
         stats.worst = frame.total;
@@ -47,6 +48,7 @@ void PrintFrameBudget(const FrameStats &stats)
     double update = stats.update_sum / n * 1e3;
     double audio = stats.audio_sum / n * 1e3;
     double render = stats.render_sum / n * 1e3;
+    double clear = stats.clear_sum / n * 1e3;
     double sprites = stats.sprites_sum / n * 1e3;
     double text = stats.text_sum / n * 1e3;
     double tilemap = stats.tilemap_sum / n * 1e3;
@@ -63,6 +65,11 @@ void PrintFrameBudget(const FrameStats &stats)
                 100.0 * audio / (avg * 1e3));
     std::printf("engine:   render      %6.3f      %2.0f%%\n", render,
                 100.0 * render / (avg * 1e3));
+    /* Lesson 098: the measure pass's instrument — the clear, named at
+       last. It was always inside render; until now it was the unnamed
+       remainder between render's row and its named sub-phases' sum. */
+    std::printf("engine:     clear     %6.3f      %2.0f%%\n", clear,
+                100.0 * clear / (avg * 1e3));
     std::printf("engine:     sprites   %6.3f      %2.0f%%\n", sprites,
                 100.0 * sprites / (avg * 1e3));
     std::printf("engine:     text      %6.3f      %2.0f%%\n", text,
diff --git a/src/frame.h b/src/frame.h
index b05bf68..1234c17 100644
--- a/src/frame.h
+++ b/src/frame.h
@@ -25,7 +25,11 @@ struct FrameRecord {
        (lesson 058) grows from. The named times are inside render, never
        instead of it: render stays the phase, these say where it went.
        Lesson 081: update grows the same kind of name — the entity work
-       the walk does, attributed inside the phase it lives in. */
+       the walk does, attributed inside the phase it lives in.
+       Lesson 098: the measure pass names the one piece of the frame no
+       row carried — the clear, the render's first work, until now the
+       unnamed remainder of the render row. */
+    double clear;    /* the framebuffer's clear — one color, every pixel */
     double sprites; /* sprite draws through the blit */
     double text;    /* lesson 051: text drawing — glyphs through the blit */
     double tilemap; /* lesson 053: the map's walk — tiles through the blit */
@@ -51,6 +55,7 @@ struct FrameStats {
     double sprites_sum; /* lesson 046's named sub-phase, summed like the rest */
     double text_sum;
     double tilemap_sum;
+    double clear_sum; /* lesson 098: the clear's row, summed like the rest */
     double entities_sum; /* lesson 081: the update's entity work, summed */
     double worst;      /* the longest frame so far */
     long worst_number; /* and which one it was */
diff --git a/src/main.cpp b/src/main.cpp
index f48d76a..112049a 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -240,7 +240,10 @@ int Run(void)
         /* Render: the current state's screen, and only that one (lesson
            082). The backdrop is the state's own — the world's blue in
            play, the panel's darker blue on the panel screens — cleared
-           once here, in the render phase, before the named sub-phases. */
+           once here, in the render phase, before the named sub-phases.
+           Lesson 098: the clear is a named sub-phase now — the measure
+           pass's instrument, and the account's row. */
+        double t_clear = platform::Now();
         if (game.state == GAME_PLAY) {
             ClearBuffer(*world.fb, 32, 32, 64);
         } else {
@@ -251,6 +254,7 @@ int Run(void)
             GameScreenColor(game, screen_r, screen_g, screen_b);
             ClearBuffer(*world.fb, screen_r, screen_g, screen_b);
         }
+        frame.clear = platform::Now() - t_clear;
         if (game.state == GAME_PLAY) {
             /* Lesson 083: the game draws its own world — the scrolling
                map and the live entities, through the game's camera. The
@@ -300,12 +304,14 @@ int Run(void)
            fields, one per subsystem, as the parts name them. The audio
            phase (lesson 060) joins in the record's own order, and
            lesson 079's step leads it: the game's advance beside the
-           machine's durations. */
-        std::printf("frame %ld: step %.3f ms, update %.3f ms (entities %.3f), audio %.3f ms, render %.3f ms (sprites %.3f, text %.3f, tilemap %.3f), present %.3f ms, total %.3f ms\n",
+           machine's durations. Lesson 098: the clear's field joins the
+           render's list, first — where it happens in the frame. */
+        std::printf("frame %ld: step %.3f ms, update %.3f ms (entities %.3f), audio %.3f ms, render %.3f ms (clear %.3f, sprites %.3f, text %.3f, tilemap %.3f), present %.3f ms, total %.3f ms\n",
                     frame.number, frame.step * 1e3, frame.update * 1e3,
                     frame.entities * 1e3,
                     frame.audio * 1e3,
                     frame.render * 1e3,
+                    frame.clear * 1e3,
                     frame.sprites * 1e3, frame.text * 1e3,
                     frame.tilemap * 1e3, frame.present * 1e3,
                     frame.total * 1e3);
```

## Exercises

Two challenges about the measurement itself — one on this machine, one
on yours (design D12 routes the real-hardware check to the learner).
Each ends with its solution — a diff against this lesson's end state
plus a walkthrough — after the prompt.

### Exercise 1 — what the profile cannot see *(measure-the-performance)*

The frame account says the present costs `0.444 ms` a frame; the flat
profile says `platform::Present` used `0.01 s` of CPU across 2,583
frames — around `0.004 ms` each. Both are true, and the gap is the
seam waiting on the X server rather than the process computing. Make
the distinction one line of the run's own report: a probe that prints
the run's engine work per frame beside its seam wait per frame. Then
answer with your run's numbers: how much wall time per frame is the
profiler blind to, and what does that do to the hotspot shares — do the
flat profile's percentages describe the frame, or the CPU?

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-098/ex1.md)

### Exercise 2 — the measure pass on your machine *(port-to-your-own-machine)*

Every number in this lesson carries its machine — WSL2, Xvfb, no sound
hardware, a `-pg` build at `-O0`. Run the same measure pass on yours:
build the `-pg` build, play the game on your own display (sound
device and all), collect the frame account and the flat profile from
the same run, and name **your** machine's top-2 hotspots beside this
one's. Then explain the differences you actually measured — did the
order hold (the map's draw first, the clear second), and if not, what
does your machine do differently: the present's copy, the sound
device's feed, the CPU's width? Report your profile's top lines as one
card you can set beside the book's.

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-098/ex2.md)

---

**Part:** [Part 5 — the game](../../index.md) ·
**Previous:** [Lesson 097 — pay the debt](lesson-097-debt.md) ·
**Next:** [Lesson 099 — pass 2a: fix the map's draw](lesson-099-map-draw.md) ·
**Code tag:** [`lesson-098`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-098)
