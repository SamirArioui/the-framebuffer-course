# Lesson 069 — the closing demo

{{#include ../../stability-horizon.md}}

## Prose

Eleven lessons ago the engine could draw a world and play nothing; ten
lessons ago sound was bytes. This is what the promise looks like
running: **one measured frame loop doing the whole engine's work** —
the Part 2 world drawn through the engine's renderer beside the Part 3
sound played through the engine's mixer, side by side, every phase
timed. Nothing new is invented today; today the parts *fit*, and the
fit is the point. This is "sound done".

### The demo

```
$ DISPLAY=:99 ALSA_DEVICE=null timeout -s INT 10 ./build/game
engine: music: 132300 frames at 44100 Hz, 1 channel, peak 10442, first frames: 0 277 554 831 1107 1381 1655 1927, last frame 0
engine: effect: 8820 frames at 44100 Hz, 1 channel, peak 9770, first frames: 0 1229 2438 3607 4719 5757 6703 7544, last frame 0
engine: mix: music   -> channel  0 (looping, volume 256 of 256)
engine: part 3 done — the world draws and the sound plays
engine: world 48x32 cells (768x512 px), 3 kinds; 96 glyphs; sprite 16x16
engine: sound 132300-frame music looping on channel 0, 8820-frame effect on the pool; one mixer of 16 channels
engine: arrow keys move the sprite, space shakes the camera; close the window to stop
engine: sprite at 312,232
engine: stream: 735-frame buffers, horizon 16.7 ms; the loop feeds one when it is due
engine: mix: effect  1 -> channel  1 (volume 64 of 256)
engine: mix: first frames (music + effect 1, summed): 0 584 1163 1732 2286 2820 3330 3813
engine: mix: effect  2 -> channel  2 (volume 64 of 256)
engine: mix: effect  3 -> channel  3 (volume 64 of 256)
engine: mix: effect  4 -> channel  1 (volume 64 of 256)   <- the pool returned it
...
engine: sprite at 442,232 (t=1.754)
engine: camera base 128,0 (t=1.754)
...
engine: sprite blocked at 603,479 (t=3.490)               <- the mover meets the wall
engine: sprite unblocked at 603,480 (t=3.507)
...
engine: camera additive 6,0 (shake starts)
engine: camera additive 0,0 (at rest)
...
engine: loop: music wrapped on channel 0 — wrap 1, 133035 frames played, cursor 735 of 132300
engine: loop: music wrapped on channel 0 — wrap 2, 265335 frames played, cursor 735 of 132300
engine: loop: music wrapped on channel 0 — wrap 3, 397635 frames played, cursor 735 of 132300
...
frame 1: update 0.001 ms, audio 0.028 ms, render 1.613 ms (sprites 0.001, text 0.006, tilemap 0.912), present 0.834 ms, total 2.476 ms
frame 2: update 0.001 ms, audio 0.033 ms, render 1.307 ms (sprites 0.002, text 0.009, tilemap 0.904), present 0.838 ms, total 2.180 ms
...
engine: demo: 656 frames measured, 582 buffers fed, 25 effects fired, 3 music wraps
engine: frame budget — 656 frames, avg 2.080 ms, worst 3.620 ms (frame 596)
engine:   subsystem   avg ms    share
engine:   update       0.003       0%
engine:   render       1.374      66%
engine:     sprites    0.002       0%
engine:     text       0.007       0%
engine:     tilemap    0.951      46%
engine:   present      0.671      32%
engine:   total        2.080     100%
engine: arena: 1534080 of 33554432 bytes used
engine: close reported
engine: closed
```

The quoted run was driven with scripted input — arrow keys and one
space tap sent to the window — so the world's own reports are in the
log beside the sound's. An undriven run has the same shape, minus the
motion lines. Read the run as a tour of the part:

- **The world** is Part 2's, untouched (lessons 044-056): the map,
  tiles, font, and sprite loaded whole at startup, the mover walking
  the sprite through the world until the wall wins — `sprite blocked at
  603,479` is lesson 056's report, exactly where the world says no —
  the camera following through its base (`camera base 128,0` is the
  view sliding across the map) and shaking through its additive offset
  on the space key, the HUD text laid over it all.
- **The sound** is Part 3's (lessons 059-068): two files' bytes loaded
  whole and printed at the top — the music's 132300 frames and the
  effect's 8820, both mono, 16-bit, at the engine's rate — routed as
  channels through one mixer: the music looping on channel 0 at full
  volume, effects on the pool's channels at a quarter volume each. The
  mixed bytes are the same sum lesson 068 showed — `0 584 1163 1732 …`
  is the music's `0 277 554 831 …` plus the effect's `0 1229 2438 …`
  at its volume.
- **Both at once** is the demo's point. The music wrapped three times —
  133035, 265335, 397635 frames played, cursor 735 at each observed
  wrap — while the world was walked and shaken and blocked, and
  twenty-five effects fired over all of it. Nothing paused one half to
  run the other; the loop's phases carry both, and the pool never had
  the option of touching the music's channel.
- **The measurement** (lessons 036, 046, 051, 053, 060) is the frame
  record's named phases, one line per frame — `update, audio, render
  (sprites, text, tilemap), present, total` — and the account at the
  end: the demo's own count of what the run did, then lesson 058's
  frame-budget table. Note what the table does *not* show yet: the
  `audio` phase is in every frame line — `0.028`, `0.033`, `0.034` —
  but the budget's rows still cover only `update + render + present`.
  The sound row is lesson 070's, on purpose, and next lesson closes the
  gap this table leaves open.

### What "done" means

Done is not finished — the engine has no game in it. Done is the MVD's
Part 3 obligations **delivered and demonstrable**:

| Obligation | Delivered in | Demonstrated by |
| ---------- | ------------ | --------------- |
| O7 — per-channel playback | lessons 063-065 | channels with cursors, volumes, loop flags; the mix's own bytes per output frame |
| music and effects routed as channels | lessons 066-068 | channel 0 looping through three wraps while the pool's one-shots come and go |
| sound loaded through the seam's whole-file I/O | lessons 059, 061-062 | the WAV parsed by hand into the arena; typed failures for missing or malformed files |
| the seam's audio output | lesson 060 | open at the engine's format, submit, close — and the paced wait that keeps the device fed |
| the mix measured, not guessed | lesson 060 | the `audio` phase in every frame record, on the platform clock |

and the limits are named, as they were at Part 2's close:

- **no speaker has made a sound.** Every run here goes to ALSA's `null`
  device, which accepts the samples and discards them. The submission
  path is verified; what the sound *is* — whether a quarter-volume
  effect reads over full-volume music — is not, and cannot be from
  this machine. The hearing is exercise 1's.
- **the submit's back-pressure is unverified.** `null` takes samples
  instantly; real hardware can make a submit wait for room in its
  buffer. The frame record measures the phase either way — lesson 070
  says what the row therefore includes.
- **the mixer's non-goals hold** — volumes, loops, and clamping, and
  nothing else. No effects suite, no resampling, no positional audio;
  that restraint is why one mixer was enough (lesson 068's rule).
- **no optimization landed in Part 3** — the deep dives measured and
  named the costs; the fixing is Part 5's three-pass menu.

### The close, and the dead code it leaves behind

The demo is the part's last look at the run, and dead code is not what
the close should leave behind. `DrawScene` — the helper lesson 054
built the camera checks on — has been defined but never used since
lesson 057 replaced the check blocks with the demo, and it was the
build's only warning. This lesson removes it: the scene is drawn where
the demo draws it, in the frame loop, through the same calls the
helper wrapped. A closing demo should show what the engine *is*, and a
function nothing calls is not part of that. The build is now
warning-free — the one warning the compiler has carried since lesson
057 retired its check blocks is gone — and it stays that way.

### The shape the rest inherits

The loop is the same four moves lesson 043 settled — pump, update,
draw, present — with lesson 060's audio step between update and draw,
and Part 4's services live beside them, not inside them. What Part 5
inherits is deliberately already in place: the frame record's named
phases (the profiler's raw data and the frame-budget table's rows, the
sound's row included from next lesson), one drawing path to optimize
and one mixer to keep — not three of either — asset formats that
cannot churn (PPM, TXT, and the WAV container lesson 061 defined), and
three deep dives whose measurements Part 5 re-runs before it changes
anything. The next lesson closes the part the way Part 2 closed: with
the table of what the frames actually cost, at last complete.

## Code step

One change for this lesson: `main.cpp` becomes the closing demo —
the startup account names the world *and* its sound as one run, the
demo's own account closes it (frames measured, buffers fed, effects
fired, music wraps), and the dead `DrawScene` is removed, taking the
build's one warning with it. The engine's libraries — sprite, blit,
font, text, tilemap, tiles, camera, and the whole of `audio` — are
untouched; the demo is the fit, and the fit is made of what is already
there. Its end state is tagged `lesson-069`.

```diff
diff --git a/src/main.cpp b/src/main.cpp
index a8264b5..455b331 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -1,11 +1,13 @@
-// main.cpp — the engine: one measured frame loop, drawing the world.
+// main.cpp — the engine: one measured frame loop, the world and its sound.
 //
-// Lesson 057: the Part 2 closing demo. Every capability of the software
-// renderer at once — the map drawn through the camera, the sprite moved
-// by polled input and stopped by the map, text laid out over it all —
-// and every phase measured, one record per frame. Nothing is invented
-// here; today the parts fit, and the fit is what the demo shows. The
-// language law of lesson 026 still holds over all of it.
+// Lesson 069: the Part 3 closing demo. Every capability of the engine at
+// once — the Part 2 world drawn through the renderer (the map through the
+// camera, the sprite moved by polled input and stopped by the map, text
+// laid out over it all) beside the Part 3 sound through the mixer (music
+// looping on its channel, effects over it on the pool's, one MixBuffer
+// into one stream) — every phase measured, one record per frame. Nothing
+// is invented here; today the parts fit, and the fit is what the demo
+// shows. The language law of lesson 026 still holds over all of it.
 
 #include <cstdio>
 
@@ -44,19 +46,6 @@ constexpr int CHUNK_FRAMES = AUDIO_RATE / 60;   /* 735 */
    keeps allocation out of the run. */
 static short stream[CHUNK_FRAMES];
 
-/* Lesson 054: the scene, drawn through the camera. The camera's summed
-   offset is applied once, at each draw's origin — the map's and the
-   sprite's. The HUD is not scene and does not pass through here. */
-static void DrawScene(Framebuffer &fb, const TileMap &map,
-                      const TileSheet &sheet, const Sprite &sprite,
-                      int sprite_x, int sprite_y, const Camera &camera)
-{
-    int x = CameraX(camera);
-    int y = CameraY(camera);
-    DrawTileMap(fb, map, sheet, -x, -y);
-    BlitSprite(fb, sprite, sprite_x - x, sprite_y - y);
-}
-
 /* Lesson 066: a loaded sample's facts, printed — the run's byte-level
    check on its two sounds. The peak is the largest frame the sample
    holds, and it is what says how much room the format still has above
@@ -217,11 +206,16 @@ int Run(void)
     int shake_frames = 0; /* lesson 054: the additive hook's demo */
     bool was_blocked = false; /* lesson 056: the mover's state report */
 
-    std::printf("engine: part 2 done — the software renderer draws the world\n");
+    /* The demo's identity: what the run is, named at once — the world
+       and its sound, one measured frame loop. */
+    std::printf("engine: part 3 done — the world draws and the sound plays\n");
     std::printf("engine: world %dx%d cells (%dx%d px), %d kinds; %d glyphs; sprite %dx%d\n",
                 map.width, map.height, map.width * TILE_SIZE,
                 map.height * TILE_SIZE, map.kind_count, FONT_COUNT,
                 sprite.width, sprite.height);
+    std::printf("engine: sound %d-frame music looping on channel %d, %d-frame effect on the pool; one mixer of %d channels\n",
+                music.frame_count, AUDIO_MUSIC_CHANNEL, effect.frame_count,
+                AUDIO_MIXER_CHANNELS);
     std::printf("engine: arrow keys move the sprite, space shakes the camera; close the window to stop\n");
     std::printf("engine: sprite at %d,%d\n", (int)sprite_x, (int)sprite_y);
 
@@ -493,6 +487,11 @@ int Run(void)
                     frame.total * 1e3);
     }
 
+    /* The demo's account: what the run did — the world's frames and the
+       sound's buffers, together — before the cost's table below. */
+    std::printf("engine: demo: %ld frames measured, %d buffers fed, %d effects fired, %d music wraps\n",
+                frame_number, feeds, effect_count, music_wraps);
+
     /* The account as the frame-budget table (lesson 058): the frame
        count, the average, the worst frame — and the render attributed to
        its subsystems, the report Part 5's finale grows. */
```

## Exercises

Two mixed exercises. Each ends with its solution — a diff against this
lesson's end state plus a walkthrough — after the prompt.

### Exercise 1 — The Part 3 demo on your machine *(port-to-your-own-machine)*

The book's demo was verified headless: Xvfb for the window, ALSA's
`null` for the sound — the pixels computed and thrown away, the samples
accepted and discarded. Run the closing demo on your own machine and
let both halves reach their real endpoints: the window on your
desktop, the samples to your speakers. Drive the run with your own
hands — walk the sprite into a wall, shake the camera, leave the music
running through a wrap — and report the demo's account and the
frame-budget table beside the book's, naming the machine with the
numbers. Then answer the two questions this machine could not: what
does the audio phase's numbers look like on hardware whose submit can
push back, and what do you actually *hear* when the effects fire over
the wrap? A measurement without its machine is a rumor; a demo without
its sound is half a demo.

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-069/ex1.md)

### Exercise 2 — The part's acceptance table *(explain-in-prose)*

"Sound done" is a claim, and a claim wants a table. Make the demo name
the capabilities it uses — one line per group: drawing, text, world,
input, sound, measurement — then fill the acceptance table: for every
scenario in the part's sound specs (samples, the WAV container,
playback, one channel, the mix, channel allocation, music, effects),
the lesson that built it and the *evidence* from your own run (the
command and what it printed). Finish with the question that makes the
table worth keeping: which rows break first when the engine changes,
and how would you notice?

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-069/ex2.md)

---

**Part:** [Part 3 — sound](../../index.md) ·
**Previous:** [Lesson 068 — music and effects together](lesson-068-together.md) ·
**Next:** [Lesson 070 — the mix's cost in the frame budget](lesson-070-audio-row.md) ·
**Code tag:** [`lesson-069`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-069)
