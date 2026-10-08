// main.cpp — the engine: the run.
//
// The run is four things and nothing else: open the window, start the
// world, run the measured frame loop, hand over the account. Lesson 097
// paid the debt fifteen lessons of assembly accumulated here — the
// asset wiring moved to load.*, the world's construction to world.*,
// the probes and the closing account to report.*, the stream's feed to
// the sound it belongs to — so what stays in this file is the loop and
// its phases, and a profiler reading this run sees the frame's work in
// the functions that do it. The language law of lesson 026 still holds
// over all of it.

#include <cstdio>

#include "audio.h"
#include "entity.h"
#include "font.h"
#include "framebuffer.h"
#include "frame.h"
#include "game.h"
#include "gametime.h"
#include "hero.h"
#include "hud.h"
#include "platform.h"
#include "report.h"
#include "sound.h"
#include "tiles.h"
#include "world.h"

namespace engine {

/* Lesson 101: the machine these measurements belong to (D12 — a
   performance claim carries its machine). The report prints it with
   its numbers; a run on different hardware names different hardware.
   What this name means for the numbers: a paced headless run (the
   loop is event-driven; the pacing is window-move jiggles at ~25 fps),
   a `-O0` build, the audio mixed in silence (no sound device), and the
   display's copy through the X server. */
constexpr const char *RUN_MACHINE =
    "WSL2, Xvfb :99, no sound hardware (the course's authoring machine)";

int Run(void)
{
    platform::WindowResult opened =
        platform::OpenWindow(FRAME_WIDTH, FRAME_HEIGHT);
    if (!opened.window) {
        /* The error path: nothing was taken that the platform layer did
           not put back, and the failure is reported by name. */
        switch (opened.error) {
        case platform::OPEN_NO_DISPLAY:
            std::fprintf(stderr, "engine: no display to open a window on\n");
            break;
        case platform::OPEN_NO_WINDOW:
            std::fprintf(stderr, "engine: the OS refused the window\n");
            break;
        default:
            std::fprintf(stderr, "engine: platform error %d\n",
                         opened.error);
            break;
        }
        return 1;
    }

    /* The world, started whole (lesson 097): every asset loaded or a
       named typed failure, every byte-level check printed, the hero
       created, the game's machine started, the world's rows filled, the
       sound started. A failure here has already said its name — the run
       closes what it opened and ends. */
    World world = {};
    Game game;
    Feedback feel;
    if (!WorldStart(world, game, feel)) {
        platform::CloseWindow(opened.window);
        ArenaRelease(world.arena);
        return 1;
    }
    Entity &hero = *world.hero;

    double started = platform::Now();
    double last = started;
    GameTime game_time = { GAMETIME_FULL }; /* lesson 078: the scale — set by the state now */
    RunReport report = {};  /* lesson 097: the probes' bookkeeping, named */
    long walk_visits = 0;   /* lesson 075: entities visited by the walk */

    /* The slice's identity: what the run is, named at once — L0*, the
       gate this part closes on. Every service it uses was finished
       before this lesson; the lesson is the fit. */
    std::printf("engine: part 4 done — the vertical slice: a hero walks the tilemap, the camera follows\n");
    std::printf("engine: world %dx%d cells (%dx%d px), %d kinds; %d glyphs; hero %dx%d\n",
                world.map.width, world.map.height, world.map.width * TILE_SIZE,
                world.map.height * TILE_SIZE, world.map.kind_count, FONT_COUNT,
                hero.sprite->width, hero.sprite->height);
    std::printf("engine: sound %d-frame music looping on channel %d; effects of %d/%d/%d frames on the pool; one mixer of %d channels\n",
                world.sound.music.frame_count, AUDIO_MUSIC_CHANNEL,
                world.sound.shot.frame_count, world.sound.hit.frame_count,
                world.sound.death.frame_count, AUDIO_MIXER_CHANNELS);
    std::printf("engine: arrows move the hero, 1 and 2 arm the weapons, space fires; close the window to stop\n");
    std::printf("engine: hero at %.0f,%.0f\n", hero.x, hero.y);

    /* Lesson 059: the run's sound is a run of amplitude at the engine's
       rate, and the seam's audio output is what puts those frames in
       front of a device. Lesson 062: the frames are the sample loaded at
       startup — a file's bytes, played to their end — and the loop feeds
       them as the stream lesson 060 shaped: buffer by buffer, at the
       horizon's pace, silence once the sample is done. */
    platform::AudioResult audio =
        platform::OpenAudioOutput(AUDIO_RATE, AUDIO_OUTPUT_CHANNELS);
    if (!audio.output) {
        switch (audio.error) {
        case platform::AUDIO_NO_DEVICE:
            std::fprintf(stderr, "engine: no audio output on this machine\n");
            break;
        default:
            std::fprintf(stderr, "engine: audio error %d\n", audio.error);
            break;
        }
        /* The failure is a value, not an ending: a machine with no output
           still runs — this one continues without sound. */
        std::fprintf(stderr, "engine: continuing without sound\n");
    }

    /* What the loop does with the sample, device or not: one buffer of
       stream per mix — the horizon the paced wait keeps queued. The
       buffer's length in time is the sample's own rate answering. */
    std::printf("engine: stream: %d-frame buffers, horizon %.1f ms; the loop mixes one when it is due\n",
                CHUNK_FRAMES, 1e3 * CHUNK_FRAMES / world.sound.music.rate);

    /* The frame step: read news, update from polled state, feed the
       stream, draw, present — every phase measured, one record per
       frame. */
    int exit_code = 0;
    long frame_number = 0;
    FrameStats stats = {};
    Feed feed = { platform::Now(), 0, 0, false }; /* lesson 060: the
                       feeding schedule; lesson 097: the sound's own */

    while (!platform::CloseRequested(opened.window)) {
        platform::PumpEvents(opened.window);
        if (platform::CloseRequested(opened.window))
            break;

        FrameRecord frame = {};
        frame.number = ++frame_number;
        double t0 = platform::Now();

        /* Update: a frame reads state — it never handles events. */
        double now = platform::Now();
        double wall_dt = now - last;
        last = now;

        /* Lesson 082: the state machine reads this frame's input and the
           named transitions, and sets the game-time scale the current
           state calls for. Play advances the world at full speed; every
           other state holds it still — so the simulation stands still
           outside play while the presentation keeps drawing the state's
           screen. The hero's movement request is written here (play's
           arrows) and left at rest in every other state. */
        GameInput(game, opened.window, hero, wall_dt);

        /* The game-time scale is the state's (play runs, the rest hold)
           times the hitstop's factor (a fraction during a hitstop, full
           at rest) — one knob, two drivers, multiplied. */
        GameTimeSetScale(game_time, GameScale(game) * FeelTimeScale(feel));

        /* Lesson 078: the update advances by game time — the wall
           clock's step, scaled. Everything the simulation does with dt
           is scaled; nothing else is. Lesson 079: the step is recorded
           beside the phases — the one field in the record that is game
           time, and the rest are wall clock at any scale. */
        double dt = GameTimeStep(game_time, wall_dt);
        frame.step = dt;

        double was_x = hero.x, was_y = hero.y;

        /* Lesson 085: the hero's movement — the held direction eased into
           motion (accel/decel, the diagonal at the straight-line speed).
           Lesson 087: and its weapon — the number keys arm the weapons
           table's rows, the fire key sends a shot. Only in play; the
           walk turns the eased velocity into steps and the shot into its
           flight. */
        if (game.state == GAME_PLAY) {
            HeroMove(hero, opened.window, dt);
            HeroFire(hero, opened.window, world.weapons, world.shots,
                     world.store, dt, world.sound);
        }

        /* Lesson 091: the waves — the fight's shape. A fresh game
           clears the last fight; a wave spawns its composition from the
           table's rows; the next begins when the last enemy of the
           current one is retired; and the last wave's clear is the
           game's completion. */
        if (game.state == GAME_PLAY)
            GameWaves(game, world.store, world.foes);

        /* Lesson 084: the game resolves its movement against its map —
           the walk is the game's now (GameWalk, in game.cpp), turning
           every live entity's request into motion through the mover (and
           every projectile into its flight). The loop times it as the
           frame record's entity sub-phase. */
        double t_entities = platform::Now();
        int visited = GameWalk(world.store, world.map, hero, world.shots,
                               feel, world.particles.rows[0], dt,
                               world.sound);
        frame.entities = platform::Now() - t_entities;
        walk_visits += visited;

        /* The score, and the hero's own reports: where the entity the
           game moves has got to. Lesson 094: the score is the game's
           own state now — the HUD reads it where the states can. */
        game.score += (hero.x > was_x ? hero.x - was_x : was_x - hero.x) +
                      (hero.y > was_y ? hero.y - was_y : was_y - hero.y);

        /* Lesson 089/085/086/077: the run's probes — the world's travel,
           the mover's state, the eased velocity, the walk cycle — each
           reporting on change, each measured against the frame's start.
           Lesson 097: their home is report.*, their bookkeeping one
           named state; the loop calls them where it always printed
           them. */
        ReportFrame(report, world.store, hero, was_x, was_y, started);

        /* Lesson 083: the game's world-view — the camera's base follows
           the hero, clamped to the map's bounds, and its additive offset
           rests at exactly zero. The game owns the camera now (GameFollow,
           in game.cpp); the loop keeps no camera of its own. */
        GameFollow(game, hero, world.map);

        /* Lesson 086: the feedback hooks run on their own wall-time —
           each fires, decays, and rests. Lesson 092 moved the run to
           here, after the frame's events and before the frame is drawn:
           the toolkit settles whatever the walk fired this frame, so a
           hit's shake is already in the hit's own frame's picture. Each
           hook returns to rest on its own — the hitstop to full speed,
           the shake's additive offset to exactly zero. */
        FeelUpdate(feel, wall_dt, game.camera);

        frame.update = platform::Now() - t0;

        /* Lesson 060: the audio step — the loop feeds the device the next
           buffer of the stream, and only when the buffer is due. The step
           is measured on every frame — it is ~0 where no buffer was due —
           so the phase accounts for all of the frame's audio work.
           Lesson 097: the feed itself is the sound's own work now
           (SoundFeed, in sound.cpp); the loop times the phase. */
        double t_audio = platform::Now();
        SoundFeed(world.sound, feed, audio.output);
        frame.audio = platform::Now() - t_audio;

        double t1 = platform::Now();

        /* Render: the current state's screen, and only that one (lesson
           082). The backdrop is the state's own — the world's blue in
           play, the panel's darker blue on the panel screens — cleared
           once here, in the render phase, before the named sub-phases.
           Lesson 098: the clear is a named sub-phase now — the measure
           pass's instrument, and the account's row. */
        double t_clear = platform::Now();
        if (game.state == GAME_PLAY) {
            ClearBuffer(*world.fb, 32, 32, 64);
        } else {
            /* Lesson 096: the screen's own backdrop, faded in from
               black by its ease — the clear stays the render phase's
               work, its color the screen's (GameScreenColor). */
            int screen_r, screen_g, screen_b;
            GameScreenColor(game, screen_r, screen_g, screen_b);
            ClearBuffer(*world.fb, screen_r, screen_g, screen_b);
        }
        frame.clear = platform::Now() - t_clear;
        if (game.state == GAME_PLAY) {
            /* Lesson 083: the game draws its own world — the scrolling
               map and the live entities, through the game's camera. The
               loop times the two the way it always has, as the frame
               record's named sub-phases. */
            double t_tilemap = platform::Now();
            GameDrawMap(game, *world.fb, world.map, world.sheet);
            frame.tilemap = platform::Now() - t_tilemap;
            double t_sprites = platform::Now();
            GameDrawSprites(game, *world.fb, world.store);
            frame.sprites = platform::Now() - t_sprites;
            /* Lesson 094: the play screen's readouts are the HUD's
               (HudDraw, in hud.cpp) — the game's own state in text,
               drawn over the world and never with the camera. */
            double t_text = platform::Now();
            HudDraw(game, hero, *world.fb, world.font);
            frame.text = platform::Now() - t_text;
        } else {
            /* The state's own screen. The world is frozen outside play —
               the simulation stands still — and the panel is what the
               window shows. */
            double t_text = platform::Now();
            GameDrawPanel(game, *world.fb, world.font);
            frame.text = platform::Now() - t_text;
        }

        frame.render = platform::Now() - t1;
        double t2 = platform::Now();

        if (!platform::Present(opened.window, world.fb->pixels,
                               world.fb->width, world.fb->height)) {
            /* A present can fail because the window died mid-copy — that
               is close news and the fold already said so. Anything else is
               a real failure and is reported as one. */
            if (platform::CloseRequested(opened.window))
                break;
            std::fprintf(stderr, "engine: presentation failed\n");
            exit_code = 1;
            break;
        }

        frame.present = platform::Now() - t2;
        frame.total = platform::Now() - t0;
        AccountFrame(stats, frame);

        /* The frame log: one line per record — the format grows its named
           fields, one per subsystem, as the parts name them. The audio
           phase (lesson 060) joins in the record's own order, and
           lesson 079's step leads it: the game's advance beside the
           machine's durations. Lesson 098: the clear's field joins the
           render's list, first — where it happens in the frame. */
        std::printf("frame %ld: step %.3f ms, update %.3f ms (entities %.3f), audio %.3f ms, render %.3f ms (clear %.3f, sprites %.3f, text %.3f, tilemap %.3f), present %.3f ms, total %.3f ms\n",
                    frame.number, frame.step * 1e3, frame.update * 1e3,
                    frame.entities * 1e3,
                    frame.audio * 1e3,
                    frame.render * 1e3,
                    frame.clear * 1e3,
                    frame.sprites * 1e3, frame.text * 1e3,
                    frame.tilemap * 1e3, frame.present * 1e3,
                    frame.total * 1e3);
    }

    /* The run's account (lesson 097: report.*), then the frame's cost
       (the frame account's own table) and the arena's. */
    ReportEnd(world.sound, feed, world.store, hero, walk_visits,
              frame_number);
    PrintFrameBudget(stats, RUN_MACHINE);
    std::printf("engine: arena: %zu of %zu bytes used\n", world.arena.used,
                world.arena.memory.size);

    if (platform::CloseRequested(opened.window))
        std::printf("engine: close reported\n");
    platform::CloseAudioOutput(audio.output);
    platform::CloseWindow(opened.window);
    ArenaRelease(world.arena);
    std::printf("engine: closed\n");
    return exit_code;
}

} /* namespace engine */

int main(void)
{
    return engine::Run();
}
