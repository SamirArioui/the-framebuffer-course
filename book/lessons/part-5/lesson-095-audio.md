# Lesson 095 — audio integration

{{#include ../../stability-horizon.md}}

## Prose

The game has been silent through every fight so far — or rather, it
has been *loud in a log*: the mixer has been summing channels into a
stream since lesson 064, but what fired the effects was a demonstration
rhythm on a clock, the same kind of stand-in lesson 092 removed from
the feel toolkit. This lesson is the audio half of that removal: **the
game's own events fire the game's sounds** — a shot is heard as it is
fired, a hit as it lands, a death as it falls — and everything routes
through the mixer's channels exactly as Parts 3 built them. The mixer
is finished engine work; this lesson *uses* it.

### The game's sound is the game's events

The sounds are four files: `music.wav` (the soundtrack, looping
forever) and three effects — `shot.wav`, `hit.wav`, `death.wav`, small
synthesized tones — loaded like every asset and reported at the byte
level before anything plays. A new game-layer pair (`sound.h`/`sound.cpp`,
design D2) holds them and knows which sound belongs to which moment:
`SoundShot`, `SoundHit`, `SoundDeath` — each one line of routing to the
mixer's pool, plus the report of the channel it landed on.

The events fire them in their own frame, like the feel toolkit fires
its hooks — the scratch roster's killing blow again:

```
engine: hit: bolt hits bag — damage 1, health 1 -> 0
engine: sound: hit -> channel 2 (volume 128 of 256)
engine: bag retired — zero health
engine: sound: death -> channel 3 (volume 128 of 256)
frame 5: step 14.007 ms, …
```

A shot's blip comes with the shot (`fire: hero -> bolt` → `sound: shot
-> channel 1`), the hit's thud with the hit, the death's boom with the
death — all before the frame's `frame` line. The channels are the
pool's (1 through 15; channel 0 is the music's, never stolen) and the
volumes are the sounds' own — a quiet blip for every shot, a heavier
thud for a hit — so the mix stays inside the format while the fight
gets loud.

### The music and the effects reach the same stream

The music starts with the run and never stops — `SoundStart` routes it
to the music channel, looping, at full volume. Its first frames in the
stream are its own file's frames, byte for byte — the run prints both
and they are the same eight numbers:

```
engine: music: 132300 frames at 44100 Hz, 1 channel, peak 10442, first frames: 0 277 554 831 1107 1381 1655 1927, last frame 0
engine: sound: music -> channel 0 (looping, volume 256 of 256)
engine: mix: first frames (music alone): 0 277 554 831 1107 1381 1655 1927
```

And when the effects arrive, they arrive in the *same* stream — the mix
is one sum. The first buffer carrying a fired effect is reported too,
and its bytes account for every sound in it:

```
engine: sound: shot -> channel 1 (volume 64 of 256)
engine: mix: first frames with the effects in: -8810 -8634 -8456 -8280 -8110 -7950 -7803 -7672
```

Take the first number apart: `-8810` is the music's frame at the cursor
that buffer read (`-8810`) plus the shot's first frame scaled by its
volume (`0 × 64/256 = 0`) — and every frame after it is the same sum:
the music's next frame (`-8889, -8961, -9026, …`) plus the shot's next
(`255, 505, 746, …` at a quarter volume). The music, the shot, and
every later hit and death ride the same stream to the same place.

The music keeps playing and wraps its sample on schedule — the loop's
own arithmetic, visible from outside the mixer:

```
engine: loop: music wrapped on channel 0 — wrap 1, 133035 frames played, cursor 735 of 132300
engine: sound: 304 frames measured, 302 buffers of stream mixed (221970 frames), 12 effects fired, 1 music wraps
```

### The device is the seam's business

This machine has **no sound hardware** — the run says so at startup
(`no audio output on this machine` / `continuing without sound`) — and
every number above was produced *anyway*. That is the design: the mix
is the game's work and the device is the seam's. The loop mixes its
buffer on the engine's rate whether or not a device exists; only the
hand-off (`SubmitSamples`) is gated on one. A machine with no output
runs the whole game's sound in silence and the reports still say what
the channels and the stream carried — which is exactly what this
lesson's verification can measure, and exactly what it cannot.

### What this run verified, and what it did not

- **The effects reach the channels** — every event's sound reports its
  channel (`shot -> channel 1`, `hit -> channel 2`, `death -> channel
  3`) in the event's own frame, on the pool's channels (1–15), the
  music's channel 0 untouched.
- **The music and the effects reach the stream** — the stream's first
  frames *are* the music's file bytes (`0 277 554 …` twice), and the
  first buffer carrying an effect is the music's frames at its cursor
  plus the shot's at its volume, added frame by frame (the arithmetic
  above). The counts close the account: `302 buffers of stream mixed
  (221970 frames), 12 effects fired, 1 music wraps`.

What this run did **not** verify is a single *audible* thing — this
machine has no speakers and the numbers above are all it can honestly
produce. The stream's bytes say what the sound *is*; whether it sounds
like a fight is heard only on a machine with a device (and the ears are
yours — that judgment cannot be measured here, and the course does not
pretend otherwise).

## Code step

One change: the game's sound. `src/sound.h/.cpp` are the game's use of
the mixer (design D2): the `Sound` struct holds the music and the three
effects, `SoundStart` routes the music to its channel, and
`SoundShot`/`SoundHit`/`SoundDeath` fire the events' effects on the
pool — each reported with the channel it landed on. `assets/shot.wav`,
`assets/hit.wav`, `assets/death.wav` are the effects (small synthesized
tones; the demonstration files of lessons 061 and 066 stay on disk and
leave the run). `src/combat.*` fire the sounds at the shot, the hit,
and the death — the same event lines the feel toolkit fires from — and
`src/hero.*`/`src/game.*` carry the sound along. `src/main.cpp` drops
the demonstration rhythm and its audio step now mixes the stream
**whether or not a device exists** — the mix is the game's work, the
device the seam's — reporting the channels and the stream's bytes and
counts. Its end state is tagged `lesson-095`.

```diff
diff --git a/src/combat.cpp b/src/combat.cpp
index e040bb5..2b3e69d 100644
--- a/src/combat.cpp
+++ b/src/combat.cpp
@@ -75,7 +75,7 @@ void CombatArm(Entity &shooter, const EntityDef &weapon)
 }
 
 bool CombatFire(EntityStore &store, const EntityTable &shots,
-                Entity &shooter, double dir_x, double dir_y)
+                Entity &shooter, double dir_x, double dir_y, Sound &sound)
 {
     if (!shooter.fires[0])
         return false; /* an unarmed shooter fires nothing */
@@ -115,12 +115,15 @@ bool CombatFire(EntityStore &store, const EntityTable &shots,
         shot.facing = 3;
     std::printf("engine: fire: %s -> %s (damage %d, range %d)\n", shooter.name,
                 shot.name, shot.damage, shot.range);
+    /* Lesson 095: the shot is heard as it is fired — the game's sound,
+       in the event's own frame. */
+    SoundShot(sound);
     return true;
 }
 
 void CombatFly(const TileMap &map, EntityStore &store, const Entity &hero,
                Entity &shot, Feedback &feel, const EntityDef &spark,
-               double dt)
+               double dt, Sound &sound)
 {
     /* The flight, in game time: the shot's speed over dt, sub-stepped
        through the mover so each sub-step is small. What is checked at
@@ -169,8 +172,10 @@ void CombatFly(const TileMap &map, EntityStore &store, const Entity &hero,
             FeelShake(feel, 5.0, 0.25);
 
             /* Lesson 093: and the impact scatters sparks — a burst of
-               particles from the same event's own frame. */
+               particles from the same event's own frame. Lesson 095:
+               and the impact is heard — the hit's sound, same frame. */
             FeelBurst(store, spark, shot.x, shot.y, 4);
+            SoundHit(sound);
 
             std::printf("engine: shot %s retired — hit %s\n", shot.name,
                         target->name);
@@ -193,10 +198,13 @@ void CombatFly(const TileMap &map, EntityStore &store, const Entity &hero,
                 FeelShake(feel, 10.0, 0.50);
 
                 /* Lesson 093: and the death bursts harder — the thing
-                   that fell scatters its sparks from its own centre. */
+                   that fell scatters its sparks from its own centre.
+                   Lesson 095: and the fall is heard — the death's
+                   sound, same frame. */
                 FeelBurst(store, spark,
                           target->x + ANIM_FRAME_W / 2.0,
                           target->y + target->sprite->height / 2.0, 8);
+                SoundDeath(sound);
             }
             return;
         }
@@ -230,7 +238,7 @@ void CombatFly(const TileMap &map, EntityStore &store, const Entity &hero,
 }
 
 void CombatAttack(EntityStore &store, const EntityTable &shots, Entity &e,
-                  const Entity &hero, double dt)
+                  const Entity &hero, double dt, Sound &sound)
 {
     /* An unarmed entity never fires — and the hero is never its own
        attacker: its trigger is the player's (HeroFire). */
@@ -257,7 +265,7 @@ void CombatAttack(EntityStore &store, const EntityTable &shots, Entity &e,
     CombatAim(dx, dy, dir_x, dir_y);
     if (dir_x == 0.0 && dir_y == 0.0)
         return;
-    if (CombatFire(store, shots, e, dir_x, dir_y))
+    if (CombatFire(store, shots, e, dir_x, dir_y, sound))
         e.cooldown = 60.0 / (double)e.rate;
 }
 
diff --git a/src/combat.h b/src/combat.h
index abb52c9..a2e8eff 100644
--- a/src/combat.h
+++ b/src/combat.h
@@ -17,6 +17,7 @@
 
 #include "entity.h"
 #include "feel.h"
+#include "sound.h"
 #include "table.h"
 
 namespace engine {
@@ -49,7 +50,8 @@ void CombatArm(Entity &shooter, const EntityDef &weapon);
    store has no slot: never a stolen entity, never a shot with assumed
    attributes. */
 bool CombatFire(EntityStore &store, const EntityTable &shots,
-                Entity &shooter, double dir_x, double dir_y);
+                Entity &shooter, double dir_x, double dir_y,
+                Sound &sound);
 
 /* One projectile's flight, once a frame of game time: sub-stepped
    through the mover, retiring at a wall (the step refused), at its
@@ -64,7 +66,7 @@ bool CombatFire(EntityStore &store, const EntityTable &shots,
    same frame from the same event. */
 void CombatFly(const TileMap &map, EntityStore &store, const Entity &hero,
                Entity &shot, Feedback &feel, const EntityDef &spark,
-               double dt);
+               double dt, Sound &sound);
 
 /* Lesson 090: the enemy attack, once per frame of game time. An armed
    entity — one whose row names a projectile kind — fires it at the
@@ -72,7 +74,7 @@ void CombatFly(const TileMap &map, EntityStore &store, const Entity &hero,
    The hero itself is never its own attacker: its trigger is the
    player's. */
 void CombatAttack(EntityStore &store, const EntityTable &shots, Entity &e,
-                  const Entity &hero, double dt);
+                  const Entity &hero, double dt, Sound &sound);
 
 } /* namespace engine */
 
diff --git a/src/game.cpp b/src/game.cpp
index e9ce2f0..add2def 100644
--- a/src/game.cpp
+++ b/src/game.cpp
@@ -221,7 +221,7 @@ void GameDrawSprites(const Game &game, Framebuffer &fb,
 
 int GameWalk(EntityStore &store, const TileMap &map, const Entity &hero,
              const EntityTable &shots, Feedback &feel, const EntityDef &spark,
-             double dt)
+             double dt, Sound &sound)
 {
     /* Lesson 084: the walk — every live entity, once per frame, in slot
        order, its movement resolved against the tilemap. The per-entity
@@ -253,7 +253,7 @@ int GameWalk(EntityStore &store, const TileMap &map, const Entity &hero,
         Entity &e = store.slots[i];
         switch (e.behavior) {
         case BEHAVIOR_FLY:
-            CombatFly(map, store, hero, e, feel, spark, dt);
+            CombatFly(map, store, hero, e, feel, spark, dt, sound);
             continue; /* the flight moves itself, through the mover */
         case BEHAVIOR_SETTLE:
             FeelParticle(store, e, dt);
@@ -281,7 +281,7 @@ int GameWalk(EntityStore &store, const TileMap &map, const Entity &hero,
            its row's weapon at the hero at its rate. The hero is exempt
            (its trigger is the player's); an unarmed kind fires
            nothing. */
-        CombatAttack(store, shots, e, hero, dt);
+        CombatAttack(store, shots, e, hero, dt, sound);
         MoveEntity(map, e, e.move_x * e.speed * dt, e.move_y * e.speed * dt);
         if (e.move_x > 0.0)
             e.facing = 0;
diff --git a/src/game.h b/src/game.h
index 4a6642e..a95b466 100644
--- a/src/game.h
+++ b/src/game.h
@@ -27,6 +27,7 @@
 #include "framebuffer.h"
 #include "gametime.h"
 #include "platform.h"
+#include "sound.h"
 #include "tiles.h"
 
 namespace engine {
@@ -123,7 +124,7 @@ void GameDrawSprites(const Game &game, Framebuffer &fb,
    Returns the visit count. */
 int GameWalk(EntityStore &store, const TileMap &map, const Entity &hero,
              const EntityTable &shots, Feedback &feel, const EntityDef &spark,
-             double dt);
+             double dt, Sound &sound);
 
 /* Lesson 091: the waves, once per frame of play. A fresh fight clears
    the last one from the store; a wave spawns its composition from the
diff --git a/src/hero.cpp b/src/hero.cpp
index 8945263..a3118d4 100644
--- a/src/hero.cpp
+++ b/src/hero.cpp
@@ -67,7 +67,7 @@ void HeroMove(Entity &hero, platform::Window *window, double dt)
 
 void HeroFire(Entity &hero, platform::Window *window,
               const EntityTable &weapons, const EntityTable &shots,
-              EntityStore &store, double dt)
+              EntityStore &store, double dt, Sound &sound)
 {
     /* Lesson 087: the number keys arm the weapons table's rows. A weapon
        is a row — arming carries its values — so the weapons grow as
@@ -104,7 +104,7 @@ void HeroFire(Entity &hero, platform::Window *window,
         else
             dir_y = -1.0;
     }
-    if (CombatFire(store, shots, hero, dir_x, dir_y))
+    if (CombatFire(store, shots, hero, dir_x, dir_y, sound))
         hero.cooldown = 60.0 / (double)hero.rate;
 }
 
diff --git a/src/hero.h b/src/hero.h
index 6804096..6487d35 100644
--- a/src/hero.h
+++ b/src/hero.h
@@ -14,6 +14,7 @@
 
 #include "entity.h"
 #include "platform.h"
+#include "sound.h"
 
 namespace engine {
 
@@ -43,7 +44,7 @@ void HeroMove(Entity &hero, platform::Window *window, double dt);
    states is the ceiling on how often the trigger answers. */
 void HeroFire(Entity &hero, platform::Window *window,
               const EntityTable &weapons, const EntityTable &shots,
-              EntityStore &store, double dt);
+              EntityStore &store, double dt, Sound &sound);
 
 } /* namespace engine */
 
diff --git a/src/main.cpp b/src/main.cpp
index 72f0aec..faedbc3 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -380,41 +380,39 @@ int Run(void)
     std::printf("engine: table: \"dragon\" -> %s\n",
                 unknown.error == DEF_OK ? "found" : "unknown");
 
-    /* Lesson 066: the run's two sounds as files' bytes — the music that
-       loops and the effect that plays once. Lesson 061's tone leaves the
-       run here (it stays on disk: the file lessons 059-065 were built
-       on); the game's own sounds are these two. Each load either yields
-       the complete sample or names what went wrong, and a failure ends
-       the run by name — like every asset above. */
-    Sample music = {}, effect = {};
-    if (!LoadRunSample(arena, "assets/music.wav", music) ||
-        !LoadRunSample(arena, "assets/effect.wav", effect)) {
+    /* Lesson 095: the game's sound — its music and one effect per
+       event, as files' bytes. Lesson 061's tone and lesson 066's
+       demonstration effect leave the run here (both stay on disk: the
+       files lessons 059-068 were built on); the game's own sounds are
+       these four. Each load either yields the complete sample or names
+       what went wrong, and a failure ends the run by name — like every
+       asset above. */
+    Sound sound = {};
+    if (!LoadRunSample(arena, "assets/music.wav", sound.music) ||
+        !LoadRunSample(arena, "assets/shot.wav", sound.shot) ||
+        !LoadRunSample(arena, "assets/hit.wav", sound.hit) ||
+        !LoadRunSample(arena, "assets/death.wav", sound.death)) {
         platform::CloseWindow(opened.window);
         ArenaRelease(arena);
         return 1;
     }
 
-    /* The byte-level check, before anything is played: each sound's facts,
-       its peak, and its first frames — the same check lesson 061 made on
-       its one file, now on both. */
-    PrintSample("music", music);
-    PrintSample("effect", effect);
-
-    /* Lesson 068: the game's sound as the game has it — the music
-       looping on the music channel and effects firing over it on the
-       pool's channels, every one of them summed by the same MixBuffer
-       into the one stream. Nothing in the mix knows which is which. */
-    Mixer mixer;
-    MixerInit(mixer);
-    MixerPlayMusic(mixer, music, AUDIO_VOLUME_FULL);
-    std::printf("engine: mix: music   -> channel %2d (looping, volume %d of %d)\n",
-                AUDIO_MUSIC_CHANNEL, mixer.channels[AUDIO_MUSIC_CHANNEL].volume,
-                AUDIO_VOLUME_FULL);
-
-    /* The run's rhythm: a burst of effects at the start — up to three in
-       flight — then one a second, all at a quarter volume so the music
-       and the busiest moment still sum inside the format. */
-    int effect_count = 0;
+    /* The byte-level check, before anything is played: each sound's
+       facts, its peak, and its first frames — the same check lesson 066
+       made on its two files, now on the game's four. */
+    PrintSample("music", sound.music);
+    PrintSample("shot", sound.shot);
+    PrintSample("hit", sound.hit);
+    PrintSample("death", sound.death);
+
+    /* Lesson 068: the music loops on the music channel and the effects
+       fire over it on the pool's channels, every one of them summed by
+       the same MixBuffer into the one stream. Lesson 095: what fires
+       them is the game now — the events of lesson 092's toolkit, each
+       with its own sound — and the demonstration rhythm that used to
+       fire them on a clock is gone. Nothing in the mix knows which
+       sound is which. */
+    SoundStart(sound);
     int music_wraps = 0;
 
     double started = platform::Now();
@@ -434,9 +432,10 @@ int Run(void)
                 map.width, map.height, map.width * TILE_SIZE,
                 map.height * TILE_SIZE, map.kind_count, FONT_COUNT,
                 hero.sprite->width, hero.sprite->height);
-    std::printf("engine: sound %d-frame music looping on channel %d, %d-frame effect on the pool; one mixer of %d channels\n",
-                music.frame_count, AUDIO_MUSIC_CHANNEL, effect.frame_count,
-                AUDIO_MIXER_CHANNELS);
+    std::printf("engine: sound %d-frame music looping on channel %d; effects of %d/%d/%d frames on the pool; one mixer of %d channels\n",
+                sound.music.frame_count, AUDIO_MUSIC_CHANNEL,
+                sound.shot.frame_count, sound.hit.frame_count,
+                sound.death.frame_count, AUDIO_MIXER_CHANNELS);
     std::printf("engine: arrows move the hero, 1 and 2 arm the weapons, space fires; close the window to stop\n");
     std::printf("engine: hero at %.0f,%.0f\n", hero.x, hero.y);
 
@@ -460,14 +459,14 @@ int Run(void)
         /* The failure is a value, not an ending: a machine with no output
            still runs — this one continues without sound. */
         std::fprintf(stderr, "engine: continuing without sound\n");
-    } else {
-        /* What the loop does with the sample: one buffer of stream per
-           feed — the horizon the paced wait keeps queued. The buffer's
-           length in time is the sample's own rate answering. */
-        std::printf("engine: stream: %d-frame buffers, horizon %.1f ms; the loop feeds one when it is due\n",
-                    CHUNK_FRAMES, 1e3 * CHUNK_FRAMES / music.rate);
     }
 
+    /* What the loop does with the sample, device or not: one buffer of
+       stream per mix — the horizon the paced wait keeps queued. The
+       buffer's length in time is the sample's own rate answering. */
+    std::printf("engine: stream: %d-frame buffers, horizon %.1f ms; the loop mixes one when it is due\n",
+                CHUNK_FRAMES, 1e3 * CHUNK_FRAMES / sound.music.rate);
+
     /* The frame step: read news, update from polled state, feed the
        stream, draw, present — every phase measured, one record per
        frame. */
@@ -484,7 +483,8 @@ int Run(void)
        handed to the device, which drives the rhythm above. The mix's own
        account of what it carried is the frame record's audio phase now,
        measured like every other phase of the frame. */
-    int feeds = 0;         /* buffers handed to the device */
+    int feeds = 0;         /* buffers of stream mixed */
+    bool reported_effects = false; /* the stream's bytes with effects in */
     long walk_visits = 0;  /* lesson 075: entities visited by the walk */
     while (!platform::CloseRequested(opened.window)) {
         platform::PumpEvents(opened.window);
@@ -532,7 +532,8 @@ int Run(void)
            flight. */
         if (game.state == GAME_PLAY) {
             HeroMove(hero, opened.window, dt);
-            HeroFire(hero, opened.window, weapons, shots, store, dt);
+            HeroFire(hero, opened.window, weapons, shots, store, dt,
+                     sound);
 
         }
 
@@ -550,7 +551,8 @@ int Run(void)
             GameWaves(game, store, foes);
 
         double t_entities = platform::Now();
-        int visited = GameWalk(store, map, hero, shots, feel, spark, dt);
+        int visited = GameWalk(store, map, hero, shots, feel, spark, dt,
+                               sound);
         frame.entities = platform::Now() - t_entities;
         walk_visits += visited;
 
@@ -648,49 +650,59 @@ int Run(void)
            wraps there instead of ending, and the mix does not know the
            difference. */
         double t_audio = platform::Now();
-        if (audio.output && t_audio >= next_feed) {
-            /* The run's rhythm, in the game's own terms: an effect every
-               fifth buffer through the opening burst, then one every
-               second — each one a MixerPlayEffect on the pool's channels,
-               over the music that keeps looping. */
-            bool fire = (feeds < 30 && feeds % 5 == 0) ||
-                        (feeds >= 30 && feeds % 30 == 0);
-            if (fire) {
-                int ch = MixerPlayEffect(mixer, effect,
-                                         AUDIO_VOLUME_FULL / 4);
-                effect_count += 1;
-                std::printf("engine: mix: effect %2d -> channel %2d (volume %d of %d)\n",
-                            effect_count, ch, mixer.channels[ch].volume,
-                            AUDIO_VOLUME_FULL);
-            }
-
-            int music_before = mixer.channels[AUDIO_MUSIC_CHANNEL].cursor;
+        if (t_audio >= next_feed) {
+            /* Lesson 095: the mix is the game's work; the device is the
+               seam's. The stream is mixed on the engine's rate whether
+               or not a device exists — with no output this machine runs
+               the whole mix in silence, and the reports still say what
+               the channels and the stream carried. */
+            int music_before =
+                sound.mixer.channels[AUDIO_MUSIC_CHANNEL].cursor;
 
-            MixBuffer(mixer, stream, CHUNK_FRAMES);
+            MixBuffer(sound.mixer, stream, CHUNK_FRAMES);
 
             if (feeds == 0) {
-                /* The mix's own bytes with both kinds of sound in it:
-                   every frame is the music's and the effect's next frame
-                   added — the same sum either way. */
-                std::printf("engine: mix: first frames (music + effect 1, summed):");
+                /* The stream's own bytes — the first buffer, the music
+                   alone at this point. */
+                std::printf("engine: mix: first frames (music alone):");
                 for (int i = 0; i < 8 && i < CHUNK_FRAMES; ++i)
                     std::printf(" %d", (int)stream[i]);
                 std::printf("\n");
             }
 
-            if (mixer.channels[AUDIO_MUSIC_CHANNEL].active &&
-                mixer.channels[AUDIO_MUSIC_CHANNEL].cursor < music_before) {
+            if (!reported_effects) {
+                /* And the stream with the game's sounds in it: the first
+                   buffer any fired effect reaches — every frame the sum
+                   of the music's next frame and the effects'. */
+                bool any = false;
+                for (int c = AUDIO_MUSIC_CHANNEL + 1;
+                     c < AUDIO_MIXER_CHANNELS && !any; ++c)
+                    any = sound.mixer.channels[c].active;
+                if (any) {
+                    std::printf("engine: mix: first frames with the effects in:");
+                    for (int i = 0; i < 8 && i < CHUNK_FRAMES; ++i)
+                        std::printf(" %d", (int)stream[i]);
+                    std::printf("\n");
+                    reported_effects = true;
+                }
+            }
+
+            if (sound.mixer.channels[AUDIO_MUSIC_CHANNEL].active &&
+                sound.mixer.channels[AUDIO_MUSIC_CHANNEL].cursor <
+                    music_before) {
                 /* The wrap: the cursor went backwards — the loop's own
                    arithmetic, visible from outside the mixer. */
                 music_wraps += 1;
-                int cursor = mixer.channels[AUDIO_MUSIC_CHANNEL].cursor;
+                int cursor = sound.mixer.channels[AUDIO_MUSIC_CHANNEL].cursor;
                 std::printf("engine: loop: music wrapped on channel %d — wrap %d, %ld frames played, cursor %d of %d\n",
                             AUDIO_MUSIC_CHANNEL, music_wraps,
-                            (long)music_wraps * music.frame_count + cursor,
-                            cursor, music.frame_count);
+                            (long)music_wraps * sound.music.frame_count +
+                                cursor,
+                            cursor, sound.music.frame_count);
             }
 
-            if (!platform::SubmitSamples(audio.output, stream,
+            if (audio.output &&
+                !platform::SubmitSamples(audio.output, stream,
                                          CHUNK_FRAMES)) {
                 /* A device that will not take the samples is named once,
                    not once per frame: the run closes the output and carries
@@ -704,7 +716,7 @@ int Run(void)
             /* The schedule restarts from now, not from the missed slot: a
                long frame is caught up by one buffer, never by a backlog. */
             next_feed = platform::Now() +
-                        (double)CHUNK_FRAMES / (double)music.rate;
+                        (double)CHUNK_FRAMES / (double)sound.music.rate;
         }
         frame.audio = platform::Now() - t_audio;
 
@@ -780,8 +792,9 @@ int Run(void)
 
     /* The demo's account: what the run did — the world's frames and the
        sound's buffers, together — before the cost's table below. */
-    std::printf("engine: demo: %ld frames measured, %d buffers fed, %d effects fired, %d music wraps\n",
-                frame_number, feeds, effect_count, music_wraps);
+    std::printf("engine: sound: %ld frames measured, %d buffers of stream mixed (%d frames), %d effects fired, %d music wraps\n",
+                frame_number, feeds, feeds * CHUNK_FRAMES, sound.fired,
+                music_wraps);
 
     /* Lesson 089: where the behaviors left the world — every live
        entity's position and its distance to the hero, the number all
diff --git a/src/sound.cpp b/src/sound.cpp
new file mode 100644
index 0000000..2b1dff4
--- /dev/null
+++ b/src/sound.cpp
@@ -0,0 +1,55 @@
+// sound.cpp — the game's events, played through the mixer's channels.
+//
+// Lesson 095: every function here is one line of routing — the event's
+// sample, its volume, and the mixer's pool — plus the report that says
+// which channel it landed on. The mixer decides everything else (the
+// one-shot contract, the oldest-stealing pool, the music channel's
+// immunity); this file only knows which sound belongs to which moment.
+
+#include "sound.h"
+
+#include <cstdio>
+
+namespace engine {
+
+void SoundStart(Sound &sound)
+{
+    /* The music is the run's music: the music channel, looping, at full
+       volume — under the effects for the whole game. */
+    MixerPlayMusic(sound.mixer, sound.music, AUDIO_VOLUME_FULL);
+    std::printf("engine: sound: music -> channel %d (looping, volume %d of %d)\n",
+                AUDIO_MUSIC_CHANNEL,
+                sound.mixer.channels[AUDIO_MUSIC_CHANNEL].volume,
+                AUDIO_VOLUME_FULL);
+}
+
+/* One event's sound: its sample on the pool's channels, at its volume,
+   reported with the channel it landed on. The volumes are the sounds'
+   own — a quiet blip for every shot, a heavier thud for a hit, the
+   loudest for a death — so the mix inside the format stays honest while
+   the game gets loud. */
+static void Fire(Sound &sound, const Sample &sample, int volume,
+                 const char *what)
+{
+    int ch = MixerPlayEffect(sound.mixer, sample, volume);
+    sound.fired += 1;
+    std::printf("engine: sound: %s -> channel %d (volume %d of %d)\n", what,
+                ch, volume, AUDIO_VOLUME_FULL);
+}
+
+void SoundShot(Sound &sound)
+{
+    Fire(sound, sound.shot, AUDIO_VOLUME_FULL / 4, "shot");
+}
+
+void SoundHit(Sound &sound)
+{
+    Fire(sound, sound.hit, AUDIO_VOLUME_FULL / 2, "hit");
+}
+
+void SoundDeath(Sound &sound)
+{
+    Fire(sound, sound.death, AUDIO_VOLUME_FULL / 2, "death");
+}
+
+} /* namespace engine */
diff --git a/src/sound.h b/src/sound.h
new file mode 100644
index 0000000..457979f
--- /dev/null
+++ b/src/sound.h
@@ -0,0 +1,46 @@
+// sound.h — the game's sound: its music and its effects, through the mixer.
+//
+// Lesson 095: the mixer is the engine's (lessons 063-066) and this is
+// the game's use of it — which sound plays at which of the game's
+// events. The events fire these in their own frame, like the feel
+// toolkit fires its effects: the shot's blip with the shot, the hit's
+// thud with the hit, the death's boom with the death. The channels and
+// the stream are the mixer's own work, and the game never touches a
+// device: with no output the run mixes in silence and the reports still
+// say what the channels and the stream carried.
+#ifndef SOUND_H
+#define SOUND_H
+
+#include "audio.h"
+
+namespace engine {
+
+/* The game's sounds: its music — looping on the music channel, under
+   everything — and its effects, one per event, fired on the pool's
+   channels. Loaded at startup like every asset; nothing is created
+   while the game runs. */
+struct Sound {
+    Mixer mixer;
+    Sample music;
+    Sample shot;
+    Sample hit;
+    Sample death;
+    int fired; /* how many effects the game's events have fired */
+};
+
+/* The soundtrack starts when the run does: the music on the music
+   channel, looping, at full volume — the channel no effect ever takes
+   or steals. */
+void SoundStart(Sound &sound);
+
+/* The game's events — a shot left a barrel, a hit landed, a thing fell
+   — each in its own frame. Every one fires its effect on the pool over
+   the music; the mixer's oldest-stealing pool decides what happens when
+   the game gets loud. */
+void SoundShot(Sound &sound);
+void SoundHit(Sound &sound);
+void SoundDeath(Sound &sound);
+
+} /* namespace engine */
+
+#endif
```

## Exercises

Two larger challenges. Each ends with its solution — a diff against this
lesson's end state plus a walkthrough — after the prompt.

### Exercise 1 — the volume follows the distance *(extend-the-code)*

Every sound plays at its own fixed volume today, wherever the game
spoke it — a shot at the hero's ear is as loud as one across the map.
Make the loudness **follow the distance**: an event's sound fades with
how far it happened from the hero and is silent beyond the falloff's
edge. Keep it inside the mixer's own volume knob (the one the channels
already carry) and avoid a square root per sound. Then run a fight with
events at two known distances and quote the volumes the run reports.

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-095/ex1.md)

### Exercise 2 — the pool under fire *(predict-the-output)*

The pool has fifteen effect channels (the music's channel 0 is never
stolen) and the mixer steals the **oldest** effect channel when they
are all busy — lesson 065's allocator. Predict the run before you make
it: with twenty of the game's sounds fired in a single frame, which
channels do the first fifteen take, which channel does the sixteenth
take, and what does the run's report show for the twenty? Then run it
(a probe that fires twenty sounds at once is the loudest frame this
machine can stage) and compare against your prediction — and say what
the oldest-stealing policy is *for*.

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-095/ex2.md)

---

**Part:** [Part 5 — the game](../../index.md) ·
**Previous:** [Lesson 094 — the HUD](lesson-094-hud.md) ·
**Next:** [Lesson 096 — screen polish](lesson-096-screens.md) ·
**Code tag:** [`lesson-095`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-095)
