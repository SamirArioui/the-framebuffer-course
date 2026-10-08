# Lesson 097 — pay the debt

{{#include ../../stability-horizon.md}}

## Prose

Fifteen lessons built this game, one capability at a time, and every
one of them was honest about where it put its code: next to the last
lesson's code, in the file that already did something like it. That is
how assembly works, and it is also how debt accumulates — not through
mistakes but through momentum. This lesson is the one the plan reserved
for it (design D10): **refactor is curriculum**. Nothing about the
game's behavior changes today. What changes is the shape it lives in,
and the proof that nothing changed is the lesson's real work.

### What the assembly accreted

The debt is not one thing; it is four, and each has an address.

**`Run()` did five jobs.** The loop's function had become the run's
whole story: it opened the window, wired the world's assets and checked
their bytes, ran the frame's phases, printed the probes, and closed
with the account — some six hundred lines, and every lesson that added
a report or a load added it *here*, in the middle of the frame's
per-frame story. By the assembly's end the loop itself — the part that
runs once per frame — was the smallest thing in it.

**The probes were tangled through the loop.** One probe per lesson that
taught a measurement: the mover's state on transition (077), the hero's
eased velocity (085), the walk cycle's frame (086), the world's travel
(089). Each grew its own bookkeeping in the loop's body — `was_blocked`,
`was_vx`, `was_vy`, `was_frame`, `seen_x[]`, `seen_y[]` — anonymous
locals living between the phases they had nothing to do with.

**The wiring reached through the seam's neighbors.** The loop's audio
step read the mixer's music channel cursor to see the wrap, scheduled
the next buffer, reported the stream's first bytes, named a refusing
device, and submitted to the seam — five concerns of the game's sound,
inline in `main.cpp`, reaching into the mixer's internals the game
promised never to touch.

**Names had stopped fitting.** `main.cpp`'s own header still announced
itself "the Part 3 closing demo" — a demo's closing script that had
become the game's composition root. The bookkeeping names described
their birth lesson rather than their job. And the files had grown
together: `main.cpp` was loading dock, probe log, and loop at once.

Four debts, one shape. And one thing this lesson looked at and **kept**:
`GameWalk`'s explicit parameter list — the store, the map, the hero,
the shots, the toolkit, the burst kind, the step, the sound, named at
the call site. That signature is design D6's contract ("per-entity work
is expressed once"), not an accident of growth: the walk is where the
game's modules meet *on purpose*, and hiding them behind one bundling
struct would make the meeting less visible, not more. Debt is what
accident accumulated; a contract is what a decision defended.

### What this paid

The run now has four homes and one line each.

**`load.*` — the run's asset wiring.** `LoadRunSample`, `LoadRunTable`,
`LoadRunArt`, and the byte-level checks `PrintSample`, `PrintDefs`, one
failure path per kind of load, moved whole from `main.cpp`. The run
loads through this pair and nothing else.

**`world.*` — the world, started whole.** `World` is everything the
loop touches — the arena, the frame's pixels, the font, the map, the
sheet, the five tables, the store, the hero, the sound — named in one
struct, and `WorldStart` builds it in the order the run has always
reported it. The order matters and the code says why: the game's
machine still starts between the hero's creation and the world's fill,
so the transcript's lines appear in the transcript's order.

**`report.*` — the run's reports.** `RunReport` is the probes'
bookkeeping, named (`blocked`, `vx`, `vy`, `walk_frame`, `seen_x`,
`seen_y`); `ReportFrame` prints the frame's probes in their original
order; `ReportEnd` closes the run with the sound's, the world's, and
the walk's accounts. The printfs are the same printfs. A report line is
a contract with every lesson that quoted it.

**`sound.*` — the feed, where the sound is.** The mixer's chunk
(`CHUNK_FRAMES`), the feeding schedule (`Feed`), and the feed itself
(`SoundFeed`) moved to the game's sound module. The loop times the
audio phase and calls one function; it no longer knows the mixer has
channels.

And `main.cpp` reads as the run again: open the window, start the
world, say what it is, run the measured loop — each phase timing what
it always timed — hand over the account, close. This matters beyond
readability, and the next lesson is where the bill comes due: **a
profiler can only name what has a name.** Before this step, per-frame
probe work and the loop's wiring sampled inside `engine::Run()`, and
the flat profile would have answered "the loop is hot" — a fact no
optimization can address. After it, the frame's work samples under
`ReportFrame`, `SoundFeed`, `GameWalk`, `GameDrawMap`, `ClearBuffer`:
the names the measure pass reads.

One honest sentence about what remains: `Run()` is still a few hundred
lines. It is the composition and the loop — a long straight line with
named phases, not a long tangle. The debt was never the line count; it
was the responsibilities crossing.

### What this run verified, and what it did not

The claim "behavior unchanged" is measured the only way a run can
measure it: the reports re-run as documented. Four runs of the same
scripted scenario — title, `Return` into play, four seconds of walking
right and firing, `Escape` into pause, `Escape` back, two seconds of
walking left, close — paced at ~25 fps by window-move jiggles against
Xvfb `:99`, twice at `lesson-096`'s build and twice at this one. Each
run was reduced to the shape a refactor may change nothing of — the
reports that fired, what each says with its measured numbers stripped,
repeated lines collapsed to their first appearance — and the shapes were
diffed (building that checker is exercise 2's business):

Each run reports **105 distinct report templates** — the reports that
fired, what each says, without their measured numbers. The refactored
runs' set is *identical* to the pre-refactor run's:

```
42c2b6b10349238e7840774698d52958  before-1.shape
42c2b6b10349238e7840774698d52958  after-1.shape
42c2b6b10349238e7840774698d52958  after-2.shape
```

And the noise floor — a *second run of the pre-refactor build* — is
louder than the refactor. The two old runs differ in **eight report
templates**, four in each direction (an excerpt of the diff):

```
33d32
< engine: fire: spitter -> shell (damage N, range N)
54a53,54
> engine: screen: death fade arrived at N,N,N (its own color)
> engine: screen: death: "GAME OVER" / "SCORE N TIME N:N WAVE N/N" / "ENTER: TITLE"
77a76
> engine: state play -> death (the hero's health reached zero)
```

— in that run the hero died early, the death screen appeared, and the
spitter's shell never flew. Two runs of the *same* binary differ in
seven templates because the fight is real; two runs across the
refactor differ in none. (The only before/after difference in the
first-appearance *order* is one `hero unblocked` probe landing a few
lines earlier — inside the same jitter.)

The checklist's demonstrations re-run as documented — the state
machine, the screens and their fades, the waves, the combat and the
toolkit, the sound, the account:

```
engine: state title -> play (the player started)
engine: wave 1 begins — 2 enemies
engine: hit: bolt hits hero — damage 1, health 3 -> 2
engine: feel: hitstop fired (0.25x, 0.15s)
engine: feel: shake fired (5 px, 0.25s)
engine: burst: spark x4 at 429,218 — 4 made, 0 dropped
engine: spark settled at 429,282 — 64 px out, its row's range 64 (exact)
engine: state play -> pause (the player paused)
engine: screen: pause: "PAUSED" / "SCORE 000424   TIME 0:04   WAVE 2/3" / "ESCAPE: RESUME"
engine: screen: pause fade arrived at 24,24,40 (its own color)
engine: state pause -> play (the player resumed)
```

The frame account says the cost is unmoved (the same scenario; the
numbers' third decimal is noise — see the noise floor above):

```
engine: frame budget — 163 frames, avg 1.936 ms, worst 2.677 ms (frame 9)   [lesson-096]
engine:   update 0.012 · entities 0.004 · audio 0.032
engine:   render 1.465 (sprites 0.008, text 0.012, tilemap 0.981) · present 0.427
engine: frame budget — 163 frames, avg 1.947 ms, worst 3.197 ms (frame 25)  [lesson-097]
engine:   update 0.014 · entities 0.005 · audio 0.031
engine:   render 1.425 (sprites 0.007, text 0.012, tilemap 0.957) · present 0.477
```

Same shape, same phases, and — the strictest witness — the same arena
bill: `1580894 of 33554432 bytes used`, byte for byte, in every run.
The refactor moved code; it created nothing.

What this run did **not** verify is that this split is the *right*
split. A transcript can prove a refactor moved no behavior; it cannot
prove no better shape exists — that is a judgment, and it is the
learner's to disagree with. One exercise below takes the other side of
the one judgment call named above.

## Code step

One change: the run gets its shape. `src/load.h/.cpp` and
`src/world.h/.cpp` grow as the run's asset wiring and world
construction — `World` is the loop's whole state, `WorldStart` the
startup's exact sequence; `src/report.h/.cpp` takes the probes and the
closing account, their bookkeeping renamed from the loop's `was_*`
locals to `RunReport`'s named fields; `src/sound.h/.cpp` grows `Feed`
and `SoundFeed`, and the mixer's chunk constant moves with them, so
`main.cpp`'s loop never looks inside a channel. `main.cpp` keeps the
window, the banners, the loop, and the close — and its header finally
says what it is. The report lines are the same lines, in the same
order. Its end state is tagged `lesson-097`.

```diff
diff --git a/src/load.cpp b/src/load.cpp
new file mode 100644
index 0000000..19f3b6d
--- /dev/null
+++ b/src/load.cpp
@@ -0,0 +1,114 @@
+// load.cpp — the run's asset wiring: the loads, and the checks.
+//
+// Lesson 097: moved whole from main.cpp — the failure paths and the
+// byte-level prints are the same code this run has always run, in the
+// same order. A move, nothing more.
+
+#include "load.h"
+
+#include <cstdio>
+
+#include "sprite.h"
+
+namespace engine {
+
+void PrintSample(const char *name, const Sample &sample)
+{
+    int peak = 0;
+    for (int i = 0; i < sample.frame_count; ++i) {
+        int v = sample.frames[i * sample.channels];
+        if (v < 0)
+            v = -v;
+        if (v > peak)
+            peak = v;
+    }
+    std::printf("engine: %s: %d frames at %d Hz, %d channel%s, peak %d, first frames:",
+                name, sample.frame_count, sample.rate, sample.channels,
+                sample.channels == 1 ? "" : "s", peak);
+    for (int i = 0; i < 8 && i < sample.frame_count; ++i)
+        std::printf(" %d", (int)sample.frames[i]);
+    std::printf(", last frame %d\n",
+                sample.frame_count ? (int)sample.frames[sample.frame_count - 1]
+                                   : 0);
+}
+
+bool LoadRunSample(Arena &arena, const char *path, Sample &into)
+{
+    SampleResult loaded = LoadSample(arena, path);
+    if (loaded.error == SAMPLE_OK) {
+        into = loaded.sample;
+        return true;
+    }
+    switch (loaded.error) {
+    case SAMPLE_MISSING:
+        std::fprintf(stderr, "engine: %s: could not load (missing)\n", path);
+        break;
+    case SAMPLE_MALFORMED:
+        std::fprintf(stderr, "engine: %s: could not load (malformed)\n", path);
+        break;
+    default:
+        std::fprintf(stderr, "engine: %s: could not load (no room)\n", path);
+        break;
+    }
+    return false;
+}
+
+bool LoadRunTable(Arena &arena, const char *path, EntityTable &into)
+{
+    TableResult loaded = LoadTable(arena, path);
+    if (loaded.error == TABLE_OK) {
+        into = loaded.table;
+        return true;
+    }
+    switch (loaded.error) {
+    case TABLE_MISSING:
+        std::fprintf(stderr, "engine: %s: could not load (missing)\n", path);
+        break;
+    case TABLE_MALFORMED:
+        std::fprintf(stderr, "engine: %s: could not load (malformed)\n", path);
+        break;
+    default:
+        std::fprintf(stderr, "engine: %s: could not load (no room)\n", path);
+        break;
+    }
+    return false;
+}
+
+bool LoadRunArt(Arena &arena, EntityTable &table)
+{
+    Sprite *images = (Sprite *)ArenaAlloc(
+        arena, (size_t)table.count * sizeof(Sprite), 4);
+    if (!images) {
+        std::fprintf(stderr, "engine: no room for the definitions' art\n");
+        return false;
+    }
+    for (int i = 0; i < table.count; ++i) {
+        EntityDef &def = table.rows[i];
+        if (!def.sprite[0])
+            continue;
+        SpriteResult art = LoadSprite(arena, def.sprite);
+        if (art.error != SPRITE_OK) {
+            std::fprintf(stderr, "engine: %s: could not load\n", def.sprite);
+            return false;
+        }
+        images[i] = art.sprite;
+        def.image = &images[i];
+    }
+    return true;
+}
+
+void PrintDefs(const char *path, const EntityTable &table)
+{
+    std::printf("engine: table %s: %d definition%s\n", path, table.count,
+                table.count == 1 ? "" : "s");
+    for (int i = 0; i < table.count; ++i) {
+        const EntityDef &def = table.rows[i];
+        std::printf("engine: def %s: x %d y %d facing %d speed %d health %d sprite %s accel %d damage %d rate %d fires %s range %d behavior %s wave %d count %d\n",
+                    def.name, def.x, def.y, def.facing, def.speed, def.health,
+                    def.sprite[0] ? def.sprite : "none", def.accel, def.damage,
+                    def.rate, def.fires[0] ? def.fires : "none", def.range,
+                    BehaviorName(def.behavior), def.wave, def.count);
+    }
+}
+
+} /* namespace engine */
diff --git a/src/load.h b/src/load.h
new file mode 100644
index 0000000..5cd4dd8
--- /dev/null
+++ b/src/load.h
@@ -0,0 +1,51 @@
+// load.h — the run's asset wiring: what it loads, and the byte-level
+// checks the loads print.
+//
+// Lesson 097: these five were born inside main.cpp, one per lesson that
+// grew the run's data (066's samples, 073's art, 087's tables) — the
+// loading dock with no door of its own. They are the run's wiring of the
+// finished loaders (LoadSample, LoadTable, LoadSprite): which failure
+// ends the run by name, and what the run prints to check its bytes
+// before anything uses them. The services below stay exactly what they
+// are; this pair only says how the run uses them.
+#ifndef LOAD_H
+#define LOAD_H
+
+#include "arena.h"
+#include "audio.h"
+#include "sprite.h"
+#include "table.h"
+
+namespace engine {
+
+/* Lesson 066: a loaded sample's facts, printed — the run's byte-level
+   check on its sounds. The peak is the largest frame the sample holds,
+   and it is what says how much room the format still has above the
+   sound. */
+void PrintSample(const char *name, const Sample &sample);
+
+/* Lesson 066: one asset load's whole failure path — a failed load is
+   named typed and ends the run by name, exactly like every other load
+   the run makes. */
+bool LoadRunSample(Arena &arena, const char *path, Sample &into);
+
+/* Lesson 087: one table load's whole failure path, the same shape — the
+   load either hands over every definition or names what went wrong typed
+   and the run ends by name. Used for every table file the game loads. */
+bool LoadRunTable(Arena &arena, const char *path, EntityTable &into);
+
+/* Lesson 073/087: the definitions' art, loaded at startup. The sprite
+   column names the file; the run loads each one and hands the definition
+   its image, so an entity created from the definition is answered from
+   the definition alone. A row that names no sprite (a weapon row) has no
+   art and needs none. */
+bool LoadRunArt(Arena &arena, EntityTable &table);
+
+/* Lesson 087: the byte-level check on a table, before anything uses it —
+   every definition, carrying every field: the values its row states and
+   the format's defaults for the columns its file did not name. */
+void PrintDefs(const char *path, const EntityTable &table);
+
+} /* namespace engine */
+
+#endif
diff --git a/src/main.cpp b/src/main.cpp
index f85e2f2..f48d76a 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -1,23 +1,19 @@
-// main.cpp — the engine: one measured frame loop, the world and its sound.
+// main.cpp — the engine: the run.
 //
-// Lesson 069: the Part 3 closing demo. Every capability of the engine at
-// once — the Part 2 world drawn through the renderer (the map through the
-// camera, the sprite moved by polled input and stopped by the map, text
-// laid out over it all) beside the Part 3 sound through the mixer (music
-// looping on its channel, effects over it on the pool's, one MixBuffer
-// into one stream) — every phase measured, one record per frame. Nothing
-// is invented here; today the parts fit, and the fit is what the demo
-// shows. The language law of lesson 026 still holds over all of it.
-
-#include <cmath>
+// The run is four things and nothing else: open the window, start the
+// world, run the measured frame loop, hand over the account. Lesson 097
+// paid the debt fifteen lessons of assembly accumulated here — the
+// asset wiring moved to load.*, the world's construction to world.*,
+// the probes and the closing account to report.*, the stream's feed to
+// the sound it belongs to — so what stays in this file is the loop and
+// its phases, and a profiler reading this run sees the frame's work in
+// the functions that do it. The language law of lesson 026 still holds
+// over all of it.
+
 #include <cstdio>
 
-#include "arena.h"
 #include "audio.h"
-#include "blit.h"
-#include "combat.h"
 #include "entity.h"
-#include "feel.h"
 #include "font.h"
 #include "framebuffer.h"
 #include "frame.h"
@@ -26,145 +22,13 @@
 #include "hero.h"
 #include "hud.h"
 #include "platform.h"
-#include "sprite.h"
-#include "table.h"
-#include "text.h"
-#include "tilemap.h"
+#include "report.h"
+#include "sound.h"
 #include "tiles.h"
+#include "world.h"
 
 namespace engine {
 
-/* Lesson 060: one buffer of stream per feed — one sixtieth of a second,
-   the horizon the loop keeps queued. Lesson 062: a feed is always
-   exactly this much stream — the sample's frames where the sample has
-   them, silence beyond its end — so the horizon arithmetic is untouched
-   whatever the sample's length is. The sample's own length is the file's
-   fact: playback stops where its frame_count says it stops, not where a
-   constant here would. */
-constexpr int CHUNK_FRAMES = AUDIO_RATE / 60;   /* 735 */
-
-/* Lesson 062: the buffer of stream one feed hands the device, filled
-   from the sample (or with silence) as the feed is due. Static, like the
-   platform layer's own staging buffers — the language law of lesson 026
-   keeps allocation out of the run. */
-static short stream[CHUNK_FRAMES];
-
-/* Lesson 066: a loaded sample's facts, printed — the run's byte-level
-   check on its two sounds. The peak is the largest frame the sample
-   holds, and it is what says how much room the format still has above
-   the sound. */
-static void PrintSample(const char *name, const Sample &sample)
-{
-    int peak = 0;
-    for (int i = 0; i < sample.frame_count; ++i) {
-        int v = sample.frames[i * sample.channels];
-        if (v < 0)
-            v = -v;
-        if (v > peak)
-            peak = v;
-    }
-    std::printf("engine: %s: %d frames at %d Hz, %d channel%s, peak %d, first frames:",
-                name, sample.frame_count, sample.rate, sample.channels,
-                sample.channels == 1 ? "" : "s", peak);
-    for (int i = 0; i < 8 && i < sample.frame_count; ++i)
-        std::printf(" %d", (int)sample.frames[i]);
-    std::printf(", last frame %d\n",
-                sample.frame_count ? (int)sample.frames[sample.frame_count - 1]
-                                   : 0);
-}
-
-/* Lesson 066: one asset load's whole failure path — a failed load is
-   named typed and ends the run by name, exactly like the loads above it. */
-static bool LoadRunSample(Arena &arena, const char *path, Sample &into)
-{
-    SampleResult loaded = LoadSample(arena, path);
-    if (loaded.error == SAMPLE_OK) {
-        into = loaded.sample;
-        return true;
-    }
-    switch (loaded.error) {
-    case SAMPLE_MISSING:
-        std::fprintf(stderr, "engine: %s: could not load (missing)\n", path);
-        break;
-    case SAMPLE_MALFORMED:
-        std::fprintf(stderr, "engine: %s: could not load (malformed)\n", path);
-        break;
-    default:
-        std::fprintf(stderr, "engine: %s: could not load (no room)\n", path);
-        break;
-    }
-    return false;
-}
-
-/* Lesson 087: one table load's whole failure path, the same shape — the
-   load either hands over every definition or names what went wrong typed
-   and the run ends by name. Used for every table file the game loads. */
-static bool LoadRunTable(Arena &arena, const char *path, EntityTable &into)
-{
-    TableResult loaded = LoadTable(arena, path);
-    if (loaded.error == TABLE_OK) {
-        into = loaded.table;
-        return true;
-    }
-    switch (loaded.error) {
-    case TABLE_MISSING:
-        std::fprintf(stderr, "engine: %s: could not load (missing)\n", path);
-        break;
-    case TABLE_MALFORMED:
-        std::fprintf(stderr, "engine: %s: could not load (malformed)\n", path);
-        break;
-    default:
-        std::fprintf(stderr, "engine: %s: could not load (no room)\n", path);
-        break;
-    }
-    return false;
-}
-
-/* Lesson 073/087: the definitions' art, loaded at startup. The sprite
-   column names the file; the run loads each one and hands the definition
-   its image, so an entity created from the definition is answered from
-   the definition alone. A row that names no sprite (a weapon row) has no
-   art and needs none. */
-static bool LoadRunArt(Arena &arena, EntityTable &table)
-{
-    Sprite *images = (Sprite *)ArenaAlloc(
-        arena, (size_t)table.count * sizeof(Sprite), 4);
-    if (!images) {
-        std::fprintf(stderr, "engine: no room for the definitions' art\n");
-        return false;
-    }
-    for (int i = 0; i < table.count; ++i) {
-        EntityDef &def = table.rows[i];
-        if (!def.sprite[0])
-            continue;
-        SpriteResult art = LoadSprite(arena, def.sprite);
-        if (art.error != SPRITE_OK) {
-            std::fprintf(stderr, "engine: %s: could not load\n", def.sprite);
-            return false;
-        }
-        images[i] = art.sprite;
-        def.image = &images[i];
-    }
-    return true;
-}
-
-/* Lesson 087: the byte-level check on a table, before anything uses it —
-   every definition, carrying every field: the values its row states and
-   the format's defaults for the columns its file did not name. */
-static void PrintDefs(const char *path, const EntityTable &table)
-{
-    std::printf("engine: table %s: %d definition%s\n", path, table.count,
-                table.count == 1 ? "" : "s");
-    for (int i = 0; i < table.count; ++i) {
-        const EntityDef &def = table.rows[i];
-        std::printf("engine: def %s: x %d y %d facing %d speed %d health %d sprite %s accel %d damage %d rate %d fires %s range %d behavior %s wave %d count %d\n",
-                    def.name, def.x, def.y, def.facing, def.speed, def.health,
-                    def.sprite[0] ? def.sprite : "none", def.accel, def.damage,
-                    def.rate, def.fires[0] ? def.fires : "none", def.range,
-                    BehaviorName(def.behavior), def.wave, def.count);
-    }
-}
-
 int Run(void)
 {
     platform::WindowResult opened =
@@ -187,255 +51,39 @@ int Run(void)
         return 1;
     }
 
-    /* The engine's memory: one arena over one reservation. Everything the
-       engine allocates lives in here and is released together. */
-    Arena arena;
-    ArenaInit(arena, 32 * 1024 * 1024);
-    Framebuffer *fb = GetFramebuffer(arena);
-
-    /* The world's assets, loaded whole at startup (lessons 044-053):
-       a font, a map, and the map's tile art — and, since lesson 073,
-       the art each definition names. Every load is a typed failure or a
-       complete asset — and a failure ends the run by name. */
-    FontResult font_loaded = LoadFont(arena, "assets/font.ppm");
-    if (font_loaded.error != FONT_OK) {
-        std::fprintf(stderr, "engine: assets/font.ppm: could not load\n");
-        platform::CloseWindow(opened.window);
-        ArenaRelease(arena);
-        return 1;
-    }
-    Font &font = font_loaded.font;
-
-    TileResult map_loaded = LoadTileMap(arena, "assets/map.txt");
-    if (map_loaded.error != TILE_OK) {
-        std::fprintf(stderr, "engine: assets/map.txt: could not load\n");
-        platform::CloseWindow(opened.window);
-        ArenaRelease(arena);
-        return 1;
-    }
-    TileMap &map = map_loaded.map;
-
-    TileSheetResult tiles_loaded = LoadTileSheet(arena, "assets/tiles.ppm",
-                                                 map.kind_count);
-    if (tiles_loaded.error != TILES_OK) {
-        std::fprintf(stderr, "engine: assets/tiles.ppm: could not load\n");
-        platform::CloseWindow(opened.window);
-        ArenaRelease(arena);
-        return 1;
-    }
-    TileSheet &sheet = tiles_loaded.sheet;
-
-    /* Lesson 071: the run's entities are data. A table file holds one
-       row per definition — its columns named by its header — and the load
-       either hands over every definition or names what went wrong, like
-       every asset above. Lesson 072: the rows are the arena's, and a
-       refused load keeps none of them. Lesson 087: the format grew by
-       named columns — and this file keeps loading byte-for-byte, its
-       seven columns exactly as lesson 071 wrote them, every field it
-       never named at the format's default. */
-    EntityTable table;
-    if (!LoadRunTable(arena, "assets/entities.txt", table)) {
-        platform::CloseWindow(opened.window);
-        ArenaRelease(arena);
-        return 1;
-    }
-
-    /* Lesson 087: the game's own data, in the grown format. The weapons
-       are rows that name the projectile kind they fire and carry their
-       rate and damage; the projectile kinds are rows a fired shot is an
-       entity of. Each file's header names the columns it uses — and only
-       those; what it leaves unnamed sits at the format's defaults.
-       Lesson 088: and the enemy roster — the three types and the boss,
-       every per-type fact its own row's value. Lesson 093: and the
-       toolkit's particle kinds — cosmetic entities from rows like every
-       other kind, the burst's art and settle in the table's columns. */
-    EntityTable weapons, shots, foes, particles;
-    if (!LoadRunTable(arena, "assets/weapons.txt", weapons) ||
-        !LoadRunTable(arena, "assets/projectiles.txt", shots) ||
-        !LoadRunTable(arena, "assets/enemies.txt", foes) ||
-        !LoadRunTable(arena, "assets/particles.txt", particles)) {
-        platform::CloseWindow(opened.window);
-        ArenaRelease(arena);
-        return 1;
-    }
-
-    /* The byte-level check, before anything uses the tables: every
-       definition of every table, carrying every field — the values its
-       row states and the format's defaults for the columns its file did
-       not name. */
-    std::printf("engine: table: unnamed fields at their defaults — accel %d, damage 0, rate 0, fires none, range 0, behavior none, wave 0, count 1\n",
-                TABLE_ACCEL_DEFAULT);
-    PrintDefs("assets/entities.txt", table);
-    PrintDefs("assets/weapons.txt", weapons);
-    PrintDefs("assets/projectiles.txt", shots);
-    PrintDefs("assets/enemies.txt", foes);
-    PrintDefs("assets/particles.txt", particles);
-
-    /* Lesson 073: the definitions' art, loaded at startup. A row that
-       names no sprite (a weapon row) has no art and needs none. */
-    if (!LoadRunArt(arena, table) || !LoadRunArt(arena, shots) ||
-        !LoadRunArt(arena, foes) || !LoadRunArt(arena, particles)) {
-        platform::CloseWindow(opened.window);
-        ArenaRelease(arena);
-        return 1;
-    }
-
-    /* Lesson 073: the game's first entity — created from the hero's
-       definition, carrying the values its row states in named fields the
-       game reads directly. Lesson 074: it lives in the store now, in a
-       slot of the capacity decided up front. */
-    DefResult hero_def = TableFind(table, "hero");
-    if (hero_def.error != DEF_OK) {
-        std::fprintf(stderr,
-                     "engine: assets/entities.txt: no definition named \"hero\"\n");
-        platform::CloseWindow(opened.window);
-        ArenaRelease(arena);
-        return 1;
-    }
-    EntityStore store = {};
-    EntityResult hero_made = EntityCreate(store, *hero_def.def);
-    if (hero_made.error != ENTITY_OK) {
-        std::fprintf(stderr, "engine: the store refused the hero\n");
-        platform::CloseWindow(opened.window);
-        ArenaRelease(arena);
-        return 1;
-    }
-    Entity &hero = *hero_made.entity;
-    std::printf("engine: entity %s: x %.0f y %.0f facing %d speed %d health %d sprite %dx%d\n",
-                hero.name, hero.x, hero.y, hero.facing, hero.speed, hero.health,
-                hero.sprite->width, hero.sprite->height);
-
-    /* Lesson 082: the game-state machine. The game is a state now, not a
-       loop with flags — it starts on the title screen, each state owns
-       its screen and its input, and the transitions are named conditions
-       (D7). The hero's starting health is the row's fact, handed to the
-       machine so a fresh game can restore it. */
+    /* The world, started whole (lesson 097): every asset loaded or a
+       named typed failure, every byte-level check printed, the hero
+       created, the game's machine started, the world's rows filled, the
+       sound started. A failure here has already said its name — the run
+       closes what it opened and ends. */
+    World world = {};
     Game game;
-    GameInit(game, hero.health);
-
-    /* Lesson 086: the feedback hooks — a screenshake and a hitstop, both
-       at rest. Lesson 092: the juice toolkit fires them from the game's
-       own events now — a hit lands, a death falls — in the event's own
-       frame (the walk's flight, in game.cpp/combat.cpp); the wall-time
-       demonstration that used to fire them here is gone. */
     Feedback feel;
-    FeelInit(feel);
-
-    /* Lesson 080: the vertical slice — the game's shape, and nothing
-       else. The hero is the row the game asks for by name (it is the
-       one the player controls); the world's other kinds come from the
-       same table, one entity per row. A new row is a new entity; the
-       run has no per-kind code to grow. */
-    int created = 1;
-    for (int i = 0; i < table.count; ++i) {
-        if (&table.rows[i] == hero_def.def)
-            continue;
-        EntityResult made = EntityCreate(store, table.rows[i]);
-        if (made.error != ENTITY_OK) {
-            std::fprintf(stderr, "engine: the store refused %s\n",
-                         table.rows[i].name);
-            platform::CloseWindow(opened.window);
-            ArenaRelease(arena);
-            return 1;
-        }
-        /* Lesson 084: a non-hero entity walked (down-right) here — a
-           stand-in for the AI. Lesson 089 replaced it: the behaviors
-           are real now, and the world's kinds move the ways their rows
-           say (the slime's row says `none`, so it stands). */
-        created += 1;
-    }
-    std::printf("engine: world: %d entities from the table's rows, live %d of %d\n",
-                created, store.live, ENTITY_CAP);
-
-    /* Lesson 091: the enemy roster is the waves' now — lesson 088's
-       standing spawn gave way to the wave fight (GameWaves), which
-       spawns the same rows wave by wave. A new row is still a new
-       enemy: no per-kind code has appeared since. */
-
-    /* Lesson 087: weapons are rows. The hero starts armed with the
-       weapons table's first row; the number keys arm the rest
-       (HeroFire). Lesson 090: the enemy-fire stand-in and its key are
-       gone — the enemy rows carry their own weapons and the walk's
-       attack fires them. */
-    if (weapons.count > 0)
-        CombatArm(hero, weapons.rows[0]);
-
-    /* Lesson 093: the burst kind — the particles table's first row. The
-       game bursts what the table puts first, the way the hero arms with
-       the weapons table's first row; a table with no particle kind is a
-       named failure, never a burst of assumed attributes. */
-    if (particles.count == 0) {
-        std::fprintf(stderr,
-                     "engine: assets/particles.txt: no particle kind\n");
+    if (!WorldStart(world, game, feel)) {
         platform::CloseWindow(opened.window);
-        ArenaRelease(arena);
+        ArenaRelease(world.arena);
         return 1;
     }
-    const EntityDef &spark = particles.rows[0];
-
-    /* The lookup's typed failure, checked on purpose: a definition the
-       table does not hold is a value — never an entity with assumed
-       attributes. */
-    DefResult unknown = TableFind(table, "dragon");
-    std::printf("engine: table: \"dragon\" -> %s\n",
-                unknown.error == DEF_OK ? "found" : "unknown");
-
-    /* Lesson 095: the game's sound — its music and one effect per
-       event, as files' bytes. Lesson 061's tone and lesson 066's
-       demonstration effect leave the run here (both stay on disk: the
-       files lessons 059-068 were built on); the game's own sounds are
-       these four. Each load either yields the complete sample or names
-       what went wrong, and a failure ends the run by name — like every
-       asset above. */
-    Sound sound = {};
-    if (!LoadRunSample(arena, "assets/music.wav", sound.music) ||
-        !LoadRunSample(arena, "assets/shot.wav", sound.shot) ||
-        !LoadRunSample(arena, "assets/hit.wav", sound.hit) ||
-        !LoadRunSample(arena, "assets/death.wav", sound.death)) {
-        platform::CloseWindow(opened.window);
-        ArenaRelease(arena);
-        return 1;
-    }
-
-    /* The byte-level check, before anything is played: each sound's
-       facts, its peak, and its first frames — the same check lesson 066
-       made on its two files, now on the game's four. */
-    PrintSample("music", sound.music);
-    PrintSample("shot", sound.shot);
-    PrintSample("hit", sound.hit);
-    PrintSample("death", sound.death);
-
-    /* Lesson 068: the music loops on the music channel and the effects
-       fire over it on the pool's channels, every one of them summed by
-       the same MixBuffer into the one stream. Lesson 095: what fires
-       them is the game now — the events of lesson 092's toolkit, each
-       with its own sound — and the demonstration rhythm that used to
-       fire them on a clock is gone. Nothing in the mix knows which
-       sound is which. */
-    SoundStart(sound);
-    int music_wraps = 0;
+    Entity &hero = *world.hero;
 
     double started = platform::Now();
     double last = started;
     GameTime game_time = { GAMETIME_FULL }; /* lesson 078: the scale — set by the state now */
-    bool was_blocked = false; /* lesson 077: the mover's state report */
-    int was_vx = 0, was_vy = 0; /* lesson 085: the hero's velocity, as it eases */
-    int was_frame = 0;          /* lesson 086: the hero's walk-cycle frame */
-    double seen_x[ENTITY_CAP] = {}, seen_y[ENTITY_CAP] = {}; /* lesson 089:
-                                  where each entity was last reported */
+    RunReport report = {};  /* lesson 097: the probes' bookkeeping, named */
+    long walk_visits = 0;   /* lesson 075: entities visited by the walk */
 
     /* The slice's identity: what the run is, named at once — L0*, the
        gate this part closes on. Every service it uses was finished
        before this lesson; the lesson is the fit. */
     std::printf("engine: part 4 done — the vertical slice: a hero walks the tilemap, the camera follows\n");
     std::printf("engine: world %dx%d cells (%dx%d px), %d kinds; %d glyphs; hero %dx%d\n",
-                map.width, map.height, map.width * TILE_SIZE,
-                map.height * TILE_SIZE, map.kind_count, FONT_COUNT,
+                world.map.width, world.map.height, world.map.width * TILE_SIZE,
+                world.map.height * TILE_SIZE, world.map.kind_count, FONT_COUNT,
                 hero.sprite->width, hero.sprite->height);
     std::printf("engine: sound %d-frame music looping on channel %d; effects of %d/%d/%d frames on the pool; one mixer of %d channels\n",
-                sound.music.frame_count, AUDIO_MUSIC_CHANNEL,
-                sound.shot.frame_count, sound.hit.frame_count,
-                sound.death.frame_count, AUDIO_MIXER_CHANNELS);
+                world.sound.music.frame_count, AUDIO_MUSIC_CHANNEL,
+                world.sound.shot.frame_count, world.sound.hit.frame_count,
+                world.sound.death.frame_count, AUDIO_MIXER_CHANNELS);
     std::printf("engine: arrows move the hero, 1 and 2 arm the weapons, space fires; close the window to stop\n");
     std::printf("engine: hero at %.0f,%.0f\n", hero.x, hero.y);
 
@@ -465,7 +113,7 @@ int Run(void)
        stream per mix — the horizon the paced wait keeps queued. The
        buffer's length in time is the sample's own rate answering. */
     std::printf("engine: stream: %d-frame buffers, horizon %.1f ms; the loop mixes one when it is due\n",
-                CHUNK_FRAMES, 1e3 * CHUNK_FRAMES / sound.music.rate);
+                CHUNK_FRAMES, 1e3 * CHUNK_FRAMES / world.sound.music.rate);
 
     /* The frame step: read news, update from polled state, feed the
        stream, draw, present — every phase measured, one record per
@@ -473,19 +121,9 @@ int Run(void)
     int exit_code = 0;
     long frame_number = 0;
     FrameStats stats = {};
+    Feed feed = { platform::Now(), 0, 0, false }; /* lesson 060: the
+                       feeding schedule; lesson 097: the sound's own */
 
-    /* Lesson 060: the run's own feeding schedule. The device consumes at
-       the engine's rate, so the next buffer is due one horizon from the
-       last one — and the loop knows that without asking the platform. */
-    double next_feed = platform::Now();
-
-    /* Lesson 068: the run's bookkeeping — how many buffers have been
-       handed to the device, which drives the rhythm above. The mix's own
-       account of what it carried is the frame record's audio phase now,
-       measured like every other phase of the frame. */
-    int feeds = 0;         /* buffers of stream mixed */
-    bool reported_effects = false; /* the stream's bytes with effects in */
-    long walk_visits = 0;  /* lesson 075: entities visited by the walk */
     while (!platform::CloseRequested(opened.window)) {
         platform::PumpEvents(opened.window);
         if (platform::CloseRequested(opened.window))
@@ -532,97 +170,49 @@ int Run(void)
            flight. */
         if (game.state == GAME_PLAY) {
             HeroMove(hero, opened.window, dt);
-            HeroFire(hero, opened.window, weapons, shots, store, dt,
-                     sound);
-
+            HeroFire(hero, opened.window, world.weapons, world.shots,
+                     world.store, dt, world.sound);
         }
 
-        /* Lesson 084: the game resolves its movement against its map —
-           the walk is the game's now (GameWalk, in game.cpp), turning
-           every live entity's request into motion through the mover (and
-           every projectile into its flight). The loop times it as the
-           frame record's entity sub-phase. */
         /* Lesson 091: the waves — the fight's shape. A fresh game
            clears the last fight; a wave spawns its composition from the
            table's rows; the next begins when the last enemy of the
            current one is retired; and the last wave's clear is the
            game's completion. */
         if (game.state == GAME_PLAY)
-            GameWaves(game, store, foes);
+            GameWaves(game, world.store, world.foes);
 
+        /* Lesson 084: the game resolves its movement against its map —
+           the walk is the game's now (GameWalk, in game.cpp), turning
+           every live entity's request into motion through the mover (and
+           every projectile into its flight). The loop times it as the
+           frame record's entity sub-phase. */
         double t_entities = platform::Now();
-        int visited = GameWalk(store, map, hero, shots, feel, spark, dt,
-                               sound);
+        int visited = GameWalk(world.store, world.map, hero, world.shots,
+                               feel, world.particles.rows[0], dt,
+                               world.sound);
         frame.entities = platform::Now() - t_entities;
         walk_visits += visited;
 
-        /* Lesson 089: the world's motion, as the behaviors produce it —
-           every non-hero entity reported as it travels about a tile, its
-           distance to the hero beside it (the number all three behaviors
-           are about: chase shrinks it, flee grows it, keep holds it). */
-        for (int i = 0; i < ENTITY_CAP; ++i) {
-            Entity &e = store.slots[i];
-            if (!e.live || &e == &hero)
-                continue;
-            double dx = e.x - seen_x[i], dy = e.y - seen_y[i];
-            if (dx * dx + dy * dy < 24.0 * 24.0)
-                continue;
-            seen_x[i] = e.x;
-            seen_y[i] = e.y;
-            double to_x = hero.x - e.x, to_y = hero.y - e.y;
-            std::printf("engine: %s at %d,%d — %d px of the hero (t=%.3f)\n",
-                        e.name, (int)e.x, (int)e.y,
-                        (int)std::sqrt(to_x * to_x + to_y * to_y),
-                        platform::Now() - started);
-        }
-
-        /* The score, and the hero's own report: where the entity the
+        /* The score, and the hero's own reports: where the entity the
            game moves has got to. Lesson 094: the score is the game's
            own state now — the HUD reads it where the states can. */
         game.score += (hero.x > was_x ? hero.x - was_x : was_x - hero.x) +
                       (hero.y > was_y ? hero.y - was_y : was_y - hero.y);
 
-        /* Lesson 077: the mover's state report, on transitions — the
-           hero moving, or pushed against something that will not move. */
-        bool blocked = (hero.move_x != 0.0 || hero.move_y != 0.0) &&
-                       hero.x == was_x && hero.y == was_y;
-        if (blocked != was_blocked) {
-            std::printf("engine: hero %s at %d,%d (t=%.3f)\n",
-                        blocked ? "blocked" : "unblocked", (int)hero.x,
-                        (int)hero.y, platform::Now() - started);
-            was_blocked = blocked;
-        }
-        if ((int)hero.x != (int)was_x || (int)hero.y != (int)was_y)
-            std::printf("engine: hero at %d,%d (t=%.3f)\n", (int)hero.x,
-                        (int)hero.y, platform::Now() - started);
-
-        /* Lesson 085: the hero's velocity, as it eases — the accel (the
-           speed rising over frames) and the decel (falling to rest) are
-           what the player feels, and this is the measurement of it. */
-        {
-            int vx = (int)(hero.move_x * hero.speed);
-            int vy = (int)(hero.move_y * hero.speed);
-            if (vx != was_vx || vy != was_vy) {
-                std::printf("engine: hero velocity %d,%d (t=%.3f)\n", vx, vy,
-                            platform::Now() - started);
-                was_vx = vx;
-                was_vy = vy;
-            }
-        }
-
-        /* Lesson 086: the walk cycle — the frame advances while the hero
-           steps, and this is the measurement of it advancing. */
-        if (hero.frame != was_frame) {
-            std::printf("engine: hero frame %d (t=%.3f)\n", hero.frame,
-                        platform::Now() - started);
-            was_frame = hero.frame;
-        }
+        /* Lesson 089/085/086/077: the run's probes — the world's travel,
+           the mover's state, the eased velocity, the walk cycle — each
+           reporting on change, each measured against the frame's start.
+           Lesson 097: their home is report.*, their bookkeeping one
+           named state; the loop calls them where it always printed
+           them. */
+        ReportFrame(report, world.store, hero, was_x, was_y, started);
 
         /* Lesson 083: the game's world-view — the camera's base follows
            the hero, clamped to the map's bounds, and its additive offset
            rests at exactly zero. The game owns the camera now (GameFollow,
            in game.cpp); the loop keeps no camera of its own. */
-        GameFollow(game, hero, map);
+        GameFollow(game, hero, world.map);
 
         /* Lesson 086: the feedback hooks run on their own wall-time —
            each fires, decays, and rests. Lesson 092 moved the run to
@@ -636,88 +226,13 @@ int Run(void)
         frame.update = platform::Now() - t0;
 
         /* Lesson 060: the audio step — the loop feeds the device the next
-           buffer of the stream, and only when the buffer is due. Input
-           news can wake a frame early; a frame woken early must not queue
-           extra audio, or the run would bury the device in buffers instead
-           of pacing them. The step is measured on every frame — it is ~0
-           where no buffer was due — so the phase accounts for all of the
-           frame's audio work.
-
-           Lesson 064: the stream is the mix. One buffer is every active
-           channel's next frames summed and clamped — silence where no
-           channel has anything to say. Lesson 066: frame_count is still
-           the fact that says where a sample ends; a channel that loops
-           wraps there instead of ending, and the mix does not know the
-           difference. */
+           buffer of the stream, and only when the buffer is due. The step
+           is measured on every frame — it is ~0 where no buffer was due —
+           so the phase accounts for all of the frame's audio work.
+           Lesson 097: the feed itself is the sound's own work now
+           (SoundFeed, in sound.cpp); the loop times the phase. */
         double t_audio = platform::Now();
-        if (t_audio >= next_feed) {
-            /* Lesson 095: the mix is the game's work; the device is the
-               seam's. The stream is mixed on the engine's rate whether
-               or not a device exists — with no output this machine runs
-               the whole mix in silence, and the reports still say what
-               the channels and the stream carried. */
-            int music_before =
-                sound.mixer.channels[AUDIO_MUSIC_CHANNEL].cursor;
-
-            MixBuffer(sound.mixer, stream, CHUNK_FRAMES);
-
-            if (feeds == 0) {
-                /* The stream's own bytes — the first buffer, the music
-                   alone at this point. */
-                std::printf("engine: mix: first frames (music alone):");
-                for (int i = 0; i < 8 && i < CHUNK_FRAMES; ++i)
-                    std::printf(" %d", (int)stream[i]);
-                std::printf("\n");
-            }
-
-            if (!reported_effects) {
-                /* And the stream with the game's sounds in it: the first
-                   buffer any fired effect reaches — every frame the sum
-                   of the music's next frame and the effects'. */
-                bool any = false;
-                for (int c = AUDIO_MUSIC_CHANNEL + 1;
-                     c < AUDIO_MIXER_CHANNELS && !any; ++c)
-                    any = sound.mixer.channels[c].active;
-                if (any) {
-                    std::printf("engine: mix: first frames with the effects in:");
-                    for (int i = 0; i < 8 && i < CHUNK_FRAMES; ++i)
-                        std::printf(" %d", (int)stream[i]);
-                    std::printf("\n");
-                    reported_effects = true;
-                }
-            }
-
-            if (sound.mixer.channels[AUDIO_MUSIC_CHANNEL].active &&
-                sound.mixer.channels[AUDIO_MUSIC_CHANNEL].cursor <
-                    music_before) {
-                /* The wrap: the cursor went backwards — the loop's own
-                   arithmetic, visible from outside the mixer. */
-                music_wraps += 1;
-                int cursor = sound.mixer.channels[AUDIO_MUSIC_CHANNEL].cursor;
-                std::printf("engine: loop: music wrapped on channel %d — wrap %d, %ld frames played, cursor %d of %d\n",
-                            AUDIO_MUSIC_CHANNEL, music_wraps,
-                            (long)music_wraps * sound.music.frame_count +
-                                cursor,
-                            cursor, sound.music.frame_count);
-            }
-
-            if (audio.output &&
-                !platform::SubmitSamples(audio.output, stream,
-                                         CHUNK_FRAMES)) {
-                /* A device that will not take the samples is named once,
-                   not once per frame: the run closes the output and carries
-                   on in silence — its wait unbounded again. */
-                std::fprintf(stderr,
-                             "engine: the output would not take the samples\n");
-                platform::CloseAudioOutput(audio.output);
-                audio.output = 0;
-            }
-            feeds += 1;
-            /* The schedule restarts from now, not from the missed slot: a
-               long frame is caught up by one buffer, never by a backlog. */
-            next_feed = platform::Now() +
-                        (double)CHUNK_FRAMES / (double)sound.music.rate;
-        }
+        SoundFeed(world.sound, feed, audio.output);
         frame.audio = platform::Now() - t_audio;
 
         double t1 = platform::Now();
@@ -727,14 +242,14 @@ int Run(void)
            play, the panel's darker blue on the panel screens — cleared
            once here, in the render phase, before the named sub-phases. */
         if (game.state == GAME_PLAY) {
-            ClearBuffer(*fb, 32, 32, 64);
+            ClearBuffer(*world.fb, 32, 32, 64);
         } else {
             /* Lesson 096: the screen's own backdrop, faded in from
                black by its ease — the clear stays the render phase's
                work, its color the screen's (GameScreenColor). */
             int screen_r, screen_g, screen_b;
             GameScreenColor(game, screen_r, screen_g, screen_b);
-            ClearBuffer(*fb, screen_r, screen_g, screen_b);
+            ClearBuffer(*world.fb, screen_r, screen_g, screen_b);
         }
         if (game.state == GAME_PLAY) {
             /* Lesson 083: the game draws its own world — the scrolling
@@ -742,31 +257,31 @@ int Run(void)
                loop times the two the way it always has, as the frame
                record's named sub-phases. */
             double t_tilemap = platform::Now();
-            GameDrawMap(game, *fb, map, sheet);
+            GameDrawMap(game, *world.fb, world.map, world.sheet);
             frame.tilemap = platform::Now() - t_tilemap;
             double t_sprites = platform::Now();
-            GameDrawSprites(game, *fb, store);
+            GameDrawSprites(game, *world.fb, world.store);
             frame.sprites = platform::Now() - t_sprites;
             /* Lesson 094: the play screen's readouts are the HUD's
                (HudDraw, in hud.cpp) — the game's own state in text,
                drawn over the world and never with the camera. */
             double t_text = platform::Now();
-            HudDraw(game, hero, *fb, font);
+            HudDraw(game, hero, *world.fb, world.font);
             frame.text = platform::Now() - t_text;
         } else {
             /* The state's own screen. The world is frozen outside play —
                the simulation stands still — and the panel is what the
                window shows. */
             double t_text = platform::Now();
-            GameDrawPanel(game, *fb, font);
+            GameDrawPanel(game, *world.fb, world.font);
             frame.text = platform::Now() - t_text;
         }
 
         frame.render = platform::Now() - t1;
         double t2 = platform::Now();
 
-        if (!platform::Present(opened.window, fb->pixels, fb->width,
-                               fb->height)) {
+        if (!platform::Present(opened.window, world.fb->pixels,
+                               world.fb->width, world.fb->height)) {
             /* A present can fail because the window died mid-copy — that
                is close news and the fold already said so. Anything else is
                a real failure and is reported as one. */
@@ -796,44 +311,19 @@ int Run(void)
                     frame.total * 1e3);
     }
 
-    /* The demo's account: what the run did — the world's frames and the
-       sound's buffers, together — before the cost's table below. */
-    std::printf("engine: sound: %ld frames measured, %d buffers of stream mixed (%d frames), %d effects fired, %d music wraps\n",
-                frame_number, feeds, feeds * CHUNK_FRAMES, sound.fired,
-                music_wraps);
-
-    /* Lesson 089: where the behaviors left the world — every live
-       entity's position and its distance to the hero, the number all
-       three behaviors are about (chase shrinks it, flee grows it, keep
-       holds it). The report above samples a moving world; this one
-       states where it ended. */
-    for (int i = 0; i < ENTITY_CAP; ++i) {
-        Entity &e = store.slots[i];
-        if (!e.live || &e == &hero)
-            continue;
-        double to_x = hero.x - e.x, to_y = hero.y - e.y;
-        std::printf("engine: world: %s ends at %d,%d — %d px of the hero\n",
-                    e.name, (int)e.x, (int)e.y,
-                    (int)std::sqrt(to_x * to_x + to_y * to_y));
-    }
-
-    /* Lesson 075: the walk's account — one visit per live entity per
-       frame, and nothing else. */
-    std::printf("engine: walk: %ld visits over %ld frames — one per live entity per frame\n",
-                walk_visits, frame_number);
-
-    /* The account as the frame-budget table (lesson 058): the frame
-       count, the average, the worst frame — and the render attributed to
-       its subsystems, the report Part 5's finale grows. */
+    /* The run's account (lesson 097: report.*), then the frame's cost
+       (the frame account's own table) and the arena's. */
+    ReportEnd(world.sound, feed, world.store, hero, walk_visits,
+              frame_number);
     PrintFrameBudget(stats);
-    std::printf("engine: arena: %zu of %zu bytes used\n", arena.used,
-                arena.memory.size);
+    std::printf("engine: arena: %zu of %zu bytes used\n", world.arena.used,
+                world.arena.memory.size);
 
     if (platform::CloseRequested(opened.window))
         std::printf("engine: close reported\n");
     platform::CloseAudioOutput(audio.output);
     platform::CloseWindow(opened.window);
-    ArenaRelease(arena);
+    ArenaRelease(world.arena);
     std::printf("engine: closed\n");
     return exit_code;
 }
diff --git a/src/report.cpp b/src/report.cpp
new file mode 100644
index 0000000..425fe4d
--- /dev/null
+++ b/src/report.cpp
@@ -0,0 +1,107 @@
+// report.cpp — the run's reports: the probes, and the closing account.
+//
+// Lesson 097: moved whole from main.cpp. The printfs are the same, in
+// the same order; only their home and their bookkeeping's names are new.
+
+#include "report.h"
+
+#include <cmath>
+#include <cstdio>
+
+#include "platform.h"
+
+namespace engine {
+
+void ReportFrame(RunReport &report, const EntityStore &store,
+                 const Entity &hero, double was_x, double was_y,
+                 double started)
+{
+    /* Lesson 089: the world's motion, as the behaviors produce it —
+       every non-hero entity reported as it travels about a tile, its
+       distance to the hero beside it (the number all three behaviors
+       are about: chase shrinks it, flee grows it, keep holds it). */
+    for (int i = 0; i < ENTITY_CAP; ++i) {
+        const Entity &e = store.slots[i];
+        if (!e.live || &e == &hero)
+            continue;
+        double dx = e.x - report.seen_x[i], dy = e.y - report.seen_y[i];
+        if (dx * dx + dy * dy < 24.0 * 24.0)
+            continue;
+        report.seen_x[i] = e.x;
+        report.seen_y[i] = e.y;
+        double to_x = hero.x - e.x, to_y = hero.y - e.y;
+        std::printf("engine: %s at %d,%d — %d px of the hero (t=%.3f)\n",
+                    e.name, (int)e.x, (int)e.y,
+                    (int)std::sqrt(to_x * to_x + to_y * to_y),
+                    platform::Now() - started);
+    }
+
+    /* Lesson 077: the mover's state report, on transitions — the
+       hero moving, or pushed against something that will not move. */
+    bool blocked = (hero.move_x != 0.0 || hero.move_y != 0.0) &&
+                   hero.x == was_x && hero.y == was_y;
+    if (blocked != report.blocked) {
+        std::printf("engine: hero %s at %d,%d (t=%.3f)\n",
+                    blocked ? "blocked" : "unblocked", (int)hero.x,
+                    (int)hero.y, platform::Now() - started);
+        report.blocked = blocked;
+    }
+    if ((int)hero.x != (int)was_x || (int)hero.y != (int)was_y)
+        std::printf("engine: hero at %d,%d (t=%.3f)\n", (int)hero.x,
+                    (int)hero.y, platform::Now() - started);
+
+    /* Lesson 085: the hero's velocity, as it eases — the accel (the
+       speed rising over frames) and the decel (falling to rest) are
+       what the player feels, and this is the measurement of it. */
+    {
+        int vx = (int)(hero.move_x * hero.speed);
+        int vy = (int)(hero.move_y * hero.speed);
+        if (vx != report.vx || vy != report.vy) {
+            std::printf("engine: hero velocity %d,%d (t=%.3f)\n", vx, vy,
+                        platform::Now() - started);
+            report.vx = vx;
+            report.vy = vy;
+        }
+    }
+
+    /* Lesson 086: the walk cycle — the frame advances while the hero
+       steps, and this is the measurement of it advancing. */
+    if (hero.frame != report.walk_frame) {
+        std::printf("engine: hero frame %d (t=%.3f)\n", hero.frame,
+                    platform::Now() - started);
+        report.walk_frame = hero.frame;
+    }
+}
+
+void ReportEnd(const Sound &sound, const Feed &feed,
+               const EntityStore &store, const Entity &hero,
+               long walk_visits, long frames)
+{
+    /* The demo's account: what the run did — the world's frames and the
+       sound's buffers, together — before the cost's table below. */
+    std::printf("engine: sound: %ld frames measured, %d buffers of stream mixed (%d frames), %d effects fired, %d music wraps\n",
+                frames, feed.feeds, feed.feeds * CHUNK_FRAMES, sound.fired,
+                feed.wraps);
+
+    /* Lesson 089: where the behaviors left the world — every live
+       entity's position and its distance to the hero, the number all
+       three behaviors are about (chase shrinks it, flee grows it, keep
+       holds it). The report above samples a moving world; this one
+       states where it ended. */
+    for (int i = 0; i < ENTITY_CAP; ++i) {
+        const Entity &e = store.slots[i];
+        if (!e.live || &e == &hero)
+            continue;
+        double to_x = hero.x - e.x, to_y = hero.y - e.y;
+        std::printf("engine: world: %s ends at %d,%d — %d px of the hero\n",
+                    e.name, (int)e.x, (int)e.y,
+                    (int)std::sqrt(to_x * to_x + to_y * to_y));
+    }
+
+    /* Lesson 075: the walk's account — one visit per live entity per
+       frame, and nothing else. */
+    std::printf("engine: walk: %ld visits over %ld frames — one per live entity per frame\n",
+                walk_visits, frames);
+}
+
+} /* namespace engine */
diff --git a/src/report.h b/src/report.h
new file mode 100644
index 0000000..df85698
--- /dev/null
+++ b/src/report.h
@@ -0,0 +1,57 @@
+// report.h — the run's reports: the probes that make a run measurable,
+// and the account that closes it.
+//
+// Lesson 097: the probes were born inline in the loop, one per lesson
+// that taught a measurement (077's mover state, 085's eased velocity,
+// 086's walk cycle, 089's traveling world), each with its own private
+// `was_*` bookkeeping in the loop's body. They are the run's eyes — a
+// headless run can only verify what it prints — so they live here now,
+// with one named state (`RunReport`) instead of anonymous locals, and
+// the loop reads as the frame's phases again.
+//
+// Every report line is the same line it has always been: the refactor
+// moves the printfs, it does not rewrite them. The transcript a
+// checklist's demonstration produces is unchanged.
+#ifndef REPORT_H
+#define REPORT_H
+
+#include "entity.h"
+#include "sound.h"
+
+namespace engine {
+
+/* What the probes remember between frames: where each entity was last
+   reported (the world's motion reports only on travel), and what the
+   hero's last report said (the mover's state, the eased velocity, the
+   walk cycle's frame — each prints on change, and the change is only
+   visible against the previous frame). */
+struct RunReport {
+    bool blocked;        /* lesson 077: the mover's state, last reported */
+    int vx, vy;          /* lesson 085: the hero's velocity, last reported */
+    int walk_frame;      /* lesson 086: the walk cycle's frame, last reported */
+    double seen_x[ENTITY_CAP], seen_y[ENTITY_CAP]; /* lesson 089: where
+                       each entity was last reported */
+};
+
+/* The frame's probes, in the order the run has always printed them: the
+   world's motion (every entity that traveled about a tile, its distance
+   to the hero beside it), then the hero's own reports — the mover's
+   state on transition, the position, the eased velocity, the walk
+   cycle's frame. `was_x`/`was_y` are the hero's position at the frame's
+   start — the probes read the walk's result against it. */
+void ReportFrame(RunReport &report, const EntityStore &store,
+                 const Entity &hero, double was_x, double was_y,
+                 double started);
+
+/* The account that closes the run: what the sound carried (frames
+   measured, buffers of stream, effects fired, wraps), where the world's
+   live entities ended beside the hero, and the walk's visit count — one
+   per live entity per frame. The frame-budget table follows this in the
+   loop's own close; it is the frame account's, not the report's. */
+void ReportEnd(const Sound &sound, const Feed &feed,
+               const EntityStore &store, const Entity &hero,
+               long walk_visits, long frames);
+
+} /* namespace engine */
+
+#endif
diff --git a/src/sound.cpp b/src/sound.cpp
index 2b1dff4..34c186f 100644
--- a/src/sound.cpp
+++ b/src/sound.cpp
@@ -12,6 +12,92 @@
 
 namespace engine {
 
+/* Lesson 062: the buffer of stream one feed hands the device, filled
+   from the sample (or with silence) as the feed is due. Static, like the
+   platform layer's own staging buffers — the language law of lesson 026
+   keeps allocation out of the run. Lesson 097: it lives beside the feed
+   that fills it. */
+static short stream[CHUNK_FRAMES];
+
+void SoundFeed(Sound &sound, Feed &feed, platform::AudioOutput *&output)
+{
+    /* The loop feeds the device the next buffer of the stream, and only
+       when the buffer is due. Input news can wake a frame early; a frame
+       woken early must not queue extra audio, or the run would bury the
+       device in buffers instead of pacing them.
+
+       Lesson 064: the stream is the mix. One buffer is every active
+       channel's next frames summed and clamped — silence where no
+       channel has anything to say. Lesson 066: frame_count is still
+       the fact that says where a sample ends; a channel that loops
+       wraps there instead of ending, and the mix does not know the
+       difference. */
+    if (platform::Now() < feed.next)
+        return;
+
+    /* Lesson 095: the mix is the game's work; the device is the
+       seam's. The stream is mixed on the engine's rate whether or not
+       a device exists — with no output this machine runs the whole mix
+       in silence, and the reports still say what the channels and the
+       stream carried. */
+    int music_before = sound.mixer.channels[AUDIO_MUSIC_CHANNEL].cursor;
+
+    MixBuffer(sound.mixer, stream, CHUNK_FRAMES);
+
+    if (feed.feeds == 0) {
+        /* The stream's own bytes — the first buffer, the music
+           alone at this point. */
+        std::printf("engine: mix: first frames (music alone):");
+        for (int i = 0; i < 8 && i < CHUNK_FRAMES; ++i)
+            std::printf(" %d", (int)stream[i]);
+        std::printf("\n");
+    }
+
+    if (!feed.reported) {
+        /* And the stream with the game's sounds in it: the first
+           buffer any fired effect reaches — every frame the sum
+           of the music's next frame and the effects'. */
+        bool any = false;
+        for (int c = AUDIO_MUSIC_CHANNEL + 1;
+             c < AUDIO_MIXER_CHANNELS && !any; ++c)
+            any = sound.mixer.channels[c].active;
+        if (any) {
+            std::printf("engine: mix: first frames with the effects in:");
+            for (int i = 0; i < 8 && i < CHUNK_FRAMES; ++i)
+                std::printf(" %d", (int)stream[i]);
+            std::printf("\n");
+            feed.reported = true;
+        }
+    }
+
+    if (sound.mixer.channels[AUDIO_MUSIC_CHANNEL].active &&
+        sound.mixer.channels[AUDIO_MUSIC_CHANNEL].cursor < music_before) {
+        /* The wrap: the cursor went backwards — the loop's own
+           arithmetic, visible from outside the mixer. */
+        feed.wraps += 1;
+        int cursor = sound.mixer.channels[AUDIO_MUSIC_CHANNEL].cursor;
+        std::printf("engine: loop: music wrapped on channel %d — wrap %d, %ld frames played, cursor %d of %d\n",
+                    AUDIO_MUSIC_CHANNEL, feed.wraps,
+                    (long)feed.wraps * sound.music.frame_count + cursor,
+                    cursor, sound.music.frame_count);
+    }
+
+    if (output && !platform::SubmitSamples(output, stream, CHUNK_FRAMES)) {
+        /* A device that will not take the samples is named once,
+           not once per frame: the run closes the output and carries
+           on in silence — its wait unbounded again. */
+        std::fprintf(stderr,
+                     "engine: the output would not take the samples\n");
+        platform::CloseAudioOutput(output);
+        output = 0;
+    }
+    feed.feeds += 1;
+    /* The schedule restarts from now, not from the missed slot: a
+       long frame is caught up by one buffer, never by a backlog. */
+    feed.next = platform::Now() +
+                (double)CHUNK_FRAMES / (double)sound.music.rate;
+}
+
 void SoundStart(Sound &sound)
 {
     /* The music is the run's music: the music channel, looping, at full
diff --git a/src/sound.h b/src/sound.h
index 457979f..4979371 100644
--- a/src/sound.h
+++ b/src/sound.h
@@ -12,9 +12,20 @@
 #define SOUND_H
 
 #include "audio.h"
+#include "platform.h"
 
 namespace engine {
 
+/* Lesson 060: one buffer of stream per feed — one sixtieth of a second,
+   the horizon the run keeps queued. Lesson 062: a feed is always
+   exactly this much stream — the sample's frames where the sample has
+   them, silence beyond its end — so the horizon arithmetic is untouched
+   whatever the sample's length is. The sample's own length is the file's
+   fact: playback stops where its frame_count says it stops, not where a
+   constant here would. Lesson 097: the constant is the sound's own now —
+   the stream's buffers are this module's business. */
+constexpr int CHUNK_FRAMES = AUDIO_RATE / 60;   /* 735 */
+
 /* The game's sounds: its music — looping on the music channel, under
    everything — and its effects, one per event, fired on the pool's
    channels. Loaded at startup like every asset; nothing is created
@@ -41,6 +52,27 @@ void SoundShot(Sound &sound);
 void SoundHit(Sound &sound);
 void SoundDeath(Sound &sound);
 
+/* Lesson 097: the run's feed — the schedule the loop used to keep in
+   its own locals (when the next buffer is due, how many have been
+   mixed, how often the music has wrapped, whether the stream's bytes
+   with effects in have been reported). The feed is the sound's book-
+   keeping now; the loop keeps the clock that paces it and nothing more. */
+struct Feed {
+    double next;   /* when the next buffer of stream is due */
+    int feeds;     /* buffers of stream mixed */
+    int wraps;     /* the music's wraps */
+    bool reported; /* the stream's bytes with effects in, reported */
+};
+
+/* One buffer of stream, when it is due — the loop's audio step. The
+   mix runs on the engine's rate whether or not a device exists (lesson
+   095): with no output the run mixes in silence and the reports still
+   say what the channels and the stream carried. The seam takes the
+   buffer — or, refusing it once, is closed and the run carries on in
+   silence. `output` is the seam's handle the run opened; the feed sets
+   it to null when the device is gone. */
+void SoundFeed(Sound &sound, Feed &feed, platform::AudioOutput *&output);
+
 } /* namespace engine */
 
 #endif
diff --git a/src/world.cpp b/src/world.cpp
new file mode 100644
index 0000000..d69a88a
--- /dev/null
+++ b/src/world.cpp
@@ -0,0 +1,217 @@
+// world.cpp — the world the run plays in: the startup, whole.
+//
+// Lesson 097: moved whole from Run() — the same sequence, the same
+// typed failures, the same byte-level checks, in the order the run has
+// always reported them. A move, nothing more.
+
+#include "world.h"
+
+#include <cstdio>
+
+#include "combat.h"
+#include "load.h"
+
+namespace engine {
+
+bool WorldStart(World &world, Game &game, Feedback &feel)
+{
+    /* The engine's memory: one arena over one reservation. Everything the
+       engine allocates lives in here and is released together. */
+    ArenaInit(world.arena, 32 * 1024 * 1024);
+    world.fb = GetFramebuffer(world.arena);
+
+    /* The world's assets, loaded whole at startup (lessons 044-053):
+       a font, a map, and the map's tile art — and, since lesson 073,
+       the art each definition names. Every load is a typed failure or a
+       complete asset — and a failure ends the run by name. */
+    FontResult font_loaded = LoadFont(world.arena, "assets/font.ppm");
+    if (font_loaded.error != FONT_OK) {
+        std::fprintf(stderr, "engine: assets/font.ppm: could not load\n");
+        return false;
+    }
+    world.font = font_loaded.font;
+
+    TileResult map_loaded = LoadTileMap(world.arena, "assets/map.txt");
+    if (map_loaded.error != TILE_OK) {
+        std::fprintf(stderr, "engine: assets/map.txt: could not load\n");
+        return false;
+    }
+    world.map = map_loaded.map;
+
+    TileSheetResult tiles_loaded = LoadTileSheet(world.arena, "assets/tiles.ppm",
+                                                world.map.kind_count);
+    if (tiles_loaded.error != TILES_OK) {
+        std::fprintf(stderr, "engine: assets/tiles.ppm: could not load\n");
+        return false;
+    }
+    world.sheet = tiles_loaded.sheet;
+
+    /* Lesson 071: the run's entities are data. A table file holds one
+       row per definition — its columns named by its header — and the load
+       either hands over every definition or names what went wrong, like
+       every asset above. Lesson 072: the rows are the arena's, and a
+       refused load keeps none of them. Lesson 087: the format grew by
+       named columns — and this file keeps loading byte-for-byte, its
+       seven columns exactly as lesson 071 wrote them, every field it
+       never named at the format's default. */
+    if (!LoadRunTable(world.arena, "assets/entities.txt", world.table))
+        return false;
+
+    /* Lesson 087: the game's own data, in the grown format. The weapons
+       are rows that name the projectile kind they fire and carry their
+       rate and damage; the projectile kinds are rows a fired shot is an
+       entity of. Each file's header names the columns it uses — and only
+       those; what it leaves unnamed sits at the format's defaults.
+       Lesson 088: and the enemy roster — the three types and the boss,
+       every per-type fact its own row's value. Lesson 093: and the
+       toolkit's particle kinds — cosmetic entities from rows like every
+       other kind, the burst's art and settle in the table's columns. */
+    if (!LoadRunTable(world.arena, "assets/weapons.txt", world.weapons) ||
+        !LoadRunTable(world.arena, "assets/projectiles.txt", world.shots) ||
+        !LoadRunTable(world.arena, "assets/enemies.txt", world.foes) ||
+        !LoadRunTable(world.arena, "assets/particles.txt", world.particles))
+        return false;
+
+    /* The byte-level check, before anything uses the tables: every
+       definition of every table, carrying every field — the values its
+       row states and the format's defaults for the columns its file did
+       not name. */
+    std::printf("engine: table: unnamed fields at their defaults — accel %d, damage 0, rate 0, fires none, range 0, behavior none, wave 0, count 1\n",
+                TABLE_ACCEL_DEFAULT);
+    PrintDefs("assets/entities.txt", world.table);
+    PrintDefs("assets/weapons.txt", world.weapons);
+    PrintDefs("assets/projectiles.txt", world.shots);
+    PrintDefs("assets/enemies.txt", world.foes);
+    PrintDefs("assets/particles.txt", world.particles);
+
+    /* Lesson 073: the definitions' art, loaded at startup. A row that
+       names no sprite (a weapon row) has no art and needs none. */
+    if (!LoadRunArt(world.arena, world.table) ||
+        !LoadRunArt(world.arena, world.shots) ||
+        !LoadRunArt(world.arena, world.foes) ||
+        !LoadRunArt(world.arena, world.particles))
+        return false;
+
+    /* Lesson 073: the game's first entity — created from the hero's
+       definition, carrying the values its row states in named fields the
+       game reads directly. Lesson 074: it lives in the store now, in a
+       slot of the capacity decided up front. */
+    DefResult hero_def = TableFind(world.table, "hero");
+    if (hero_def.error != DEF_OK) {
+        std::fprintf(stderr,
+                     "engine: assets/entities.txt: no definition named \"hero\"\n");
+        return false;
+    }
+    EntityResult hero_made = EntityCreate(world.store, *hero_def.def);
+    if (hero_made.error != ENTITY_OK) {
+        std::fprintf(stderr, "engine: the store refused the hero\n");
+        return false;
+    }
+    world.hero = hero_made.entity;
+    std::printf("engine: entity %s: x %.0f y %.0f facing %d speed %d health %d sprite %dx%d\n",
+                world.hero->name, world.hero->x, world.hero->y,
+                world.hero->facing, world.hero->speed, world.hero->health,
+                world.hero->sprite->width, world.hero->sprite->height);
+
+    /* Lesson 082: the game-state machine. The game is a state now, not a
+       loop with flags — it starts on the title screen, each state owns
+       its screen and its input, and the transitions are named conditions
+       (D7). The hero's starting health is the row's fact, handed to the
+       machine so a fresh game can restore it. */
+    GameInit(game, world.hero->health);
+
+    /* Lesson 086: the feedback hooks — a screenshake and a hitstop, both
+       at rest. Lesson 092: the juice toolkit fires them from the game's
+       own events now — a hit lands, a death falls — in the event's own
+       frame (the walk's flight, in game.cpp/combat.cpp); the wall-time
+       demonstration that used to fire them here is gone. */
+    FeelInit(feel);
+
+    /* Lesson 080: the vertical slice — the game's shape, and nothing
+       else. The hero is the row the game asks for by name (it is the
+       one the player controls); the world's other kinds come from the
+       same table, one entity per row. A new row is a new entity; the
+       run has no per-kind code to grow. */
+    int created = 1;
+    for (int i = 0; i < world.table.count; ++i) {
+        if (&world.table.rows[i] == hero_def.def)
+            continue;
+        EntityResult made = EntityCreate(world.store, world.table.rows[i]);
+        if (made.error != ENTITY_OK) {
+            std::fprintf(stderr, "engine: the store refused %s\n",
+                         world.table.rows[i].name);
+            return false;
+        }
+        /* Lesson 084: a non-hero entity walked (down-right) here — a
+           stand-in for the AI. Lesson 089 replaced it: the behaviors
+           are real now, and the world's kinds move the ways their rows
+           say (the slime's row says `none`, so it stands). */
+        created += 1;
+    }
+    std::printf("engine: world: %d entities from the table's rows, live %d of %d\n",
+                created, world.store.live, ENTITY_CAP);
+
+    /* Lesson 091: the enemy roster is the waves' now — lesson 088's
+       standing spawn gave way to the wave fight (GameWaves), which
+       spawns the same rows wave by wave. A new row is still a new
+       enemy: no per-kind code has appeared since. */
+
+    /* Lesson 087: weapons are rows. The hero starts armed with the
+       weapons table's first row; the number keys arm the rest
+       (HeroFire). Lesson 090: the enemy-fire stand-in and its key are
+       gone — the enemy rows carry their own weapons and the walk's
+       attack fires them. */
+    if (world.weapons.count > 0)
+        CombatArm(*world.hero, world.weapons.rows[0]);
+
+    /* Lesson 093: the burst kind — the particles table's first row. The
+       game bursts what the table puts first, the way the hero arms with
+       the weapons table's first row; a table with no particle kind is a
+       named failure, never a burst of assumed attributes. */
+    if (world.particles.count == 0) {
+        std::fprintf(stderr,
+                     "engine: assets/particles.txt: no particle kind\n");
+        return false;
+    }
+
+    /* The lookup's typed failure, checked on purpose: a definition the
+       table does not hold is a value — never an entity with assumed
+       attributes. */
+    DefResult unknown = TableFind(world.table, "dragon");
+    std::printf("engine: table: \"dragon\" -> %s\n",
+                unknown.error == DEF_OK ? "found" : "unknown");
+
+    /* Lesson 095: the game's sound — its music and one effect per
+       event, as files' bytes. Lesson 061's tone and lesson 066's
+       demonstration effect leave the run here (both stay on disk: the
+       files lessons 059-068 were built on); the game's own sounds are
+       these four. Each load either yields the complete sample or names
+       what went wrong, and a failure ends the run by name — like every
+       asset above. */
+    world.sound = {};
+    if (!LoadRunSample(world.arena, "assets/music.wav", world.sound.music) ||
+        !LoadRunSample(world.arena, "assets/shot.wav", world.sound.shot) ||
+        !LoadRunSample(world.arena, "assets/hit.wav", world.sound.hit) ||
+        !LoadRunSample(world.arena, "assets/death.wav", world.sound.death))
+        return false;
+
+    /* The byte-level check, before anything is played: each sound's
+       facts, its peak, and its first frames — the same check lesson 066
+       made on its two files, now on the game's four. */
+    PrintSample("music", world.sound.music);
+    PrintSample("shot", world.sound.shot);
+    PrintSample("hit", world.sound.hit);
+    PrintSample("death", world.sound.death);
+
+    /* Lesson 068: the music loops on the music channel and the effects
+       fire over it on the pool's channels, every one of them summed by
+       the same MixBuffer into the one stream. Lesson 095: what fires
+       them is the game now — the events of lesson 092's toolkit, each
+       with its own sound — and the demonstration rhythm that used to
+       fire them on a clock is gone. Nothing in the mix knows which
+       sound is which. */
+    SoundStart(world.sound);
+    return true;
+}
+
+} /* namespace engine */
diff --git a/src/world.h b/src/world.h
new file mode 100644
index 0000000..27d2970
--- /dev/null
+++ b/src/world.h
@@ -0,0 +1,65 @@
+// world.h — the world the run plays in: its assets, its entities, its
+// sound — loaded whole at startup.
+//
+// Lesson 097: the startup was a straight line through Run() — load,
+// check, create, wire, one typed failure per stage — four hundred lines
+// of composition between opening the window and the first frame. It is
+// the world's construction, and it lives here: `World` is everything the
+// loop touches, named in one place, and `WorldStart` builds it in the
+// order the run has always reported it (the game's machine starts
+// between the hero's creation and the world's fill — the reports keep
+// their order because the sequence is the same sequence).
+//
+// Nothing here is a service's redesign (design D2): the arena, the
+// table, the store, the mover, the mixer stay exactly what they are.
+// This pair is the run's way of standing them up together.
+#ifndef WORLD_H
+#define WORLD_H
+
+#include "arena.h"
+#include "entity.h"
+#include "feel.h"
+#include "font.h"
+#include "framebuffer.h"
+#include "game.h"
+#include "sound.h"
+#include "table.h"
+#include "tilemap.h"
+#include "tiles.h"
+
+namespace engine {
+
+/* Everything the loop touches, in one named state: the memory, the
+   frame's pixels, the world's assets (font, map, tile sheet, the five
+   tables), the entities in their store, the hero the player plays, and
+   the game's sound. */
+struct World {
+    Arena arena;
+    Framebuffer *fb;
+    Font font;
+    TileMap map;
+    TileSheet sheet;
+    EntityTable table;     /* assets/entities.txt — the hero and the
+                              world's scenery */
+    EntityTable weapons;   /* lesson 087: weapons are rows */
+    EntityTable shots;     /* the projectile kinds the weapons name */
+    EntityTable foes;      /* lesson 088: the enemy roster */
+    EntityTable particles; /* lesson 093: the toolkit's burst kinds */
+    EntityStore store;
+    Entity *hero;          /* the game's actor, the row the game asks
+                              for by name */
+    Sound sound;           /* lesson 095: the game's music and effects */
+};
+
+/* The run's startup, whole: every asset loaded or a named typed failure
+   (the failure prints, and this answers false — the caller closes what
+   the run opened), every byte-level check printed, the hero created,
+   the game's machine started, the world's rows filled, the hero armed,
+   the burst kind named, and the sound started. The order is the run's
+   own; the reports a checklist's demonstration reads are exactly the
+   reports this sequence has always printed. */
+bool WorldStart(World &world, Game &game, Feedback &feel);
+
+} /* namespace engine */
+
+#endif
```

## Exercises

Two challenges, both about the standard this lesson set: a refactor
pays debt and moves no behavior — and both have teeth. Each ends with
its solution — a diff against this lesson's end state plus a
walkthrough — after the prompt.

### Exercise 1 — the banners come in from the cold *(extend-the-code)*

The five identity banners — the slice's name, the world's shape, the
sound's, the controls', the hero's start — are still printed inline in
`Run()`, between the world's start and the audio's open. They are the
run's reports like any other, and `report.*` is where the run's reports
live now. Give them their own name there and leave `main.cpp` with the
loop and nothing else. Then run the game and show the run still
introduces itself exactly as it always has — the same five lines, the
same words, the same order.

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-097/ex1.md)

### Exercise 2 — the transcript's bill *(measure-the-performance)*

"Behavior unchanged" is a measurable claim and this lesson measured it
by comparing report shapes across builds. Do the whole measurement
yourself, and do it honestly: two runs of the *same* build never
produce the same transcript — the measured values move, the fight
decides itself, the pace jitters. So measure the noise floor first (two
runs of `lesson-096`'s build of the same scenario), then the refactored
build, and compare what a refactor may actually change nothing of: the
reports that fired, what each says once its numbers are stripped, and
their order of first appearance. Write the tool that reduces a
transcript to that shape, use it across the builds, and state the
verdict with the noise floor in view — including any line where the
refactored runs and the old ones differ *less* than the old ones differ
from each other.

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-097/ex2.md)

---

**Part:** [Part 5 — the game](../../index.md) ·
**Previous:** [Lesson 096 — screen polish](lesson-096-screens.md) ·
**Next:** [Lesson 098 — pass 1: measure](lesson-098-measure.md) ·
**Code tag:** [`lesson-097`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-097)
