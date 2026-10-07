// main.cpp — the engine: one measured frame loop, the world and its sound.
//
// Lesson 069: the Part 3 closing demo. Every capability of the engine at
// once — the Part 2 world drawn through the renderer (the map through the
// camera, the sprite moved by polled input and stopped by the map, text
// laid out over it all) beside the Part 3 sound through the mixer (music
// looping on its channel, effects over it on the pool's, one MixBuffer
// into one stream) — every phase measured, one record per frame. Nothing
// is invented here; today the parts fit, and the fit is what the demo
// shows. The language law of lesson 026 still holds over all of it.

#include <cstdio>

#include "arena.h"
#include "audio.h"
#include "blit.h"
#include "camera.h"
#include "entity.h"
#include "font.h"
#include "framebuffer.h"
#include "frame.h"
#include "platform.h"
#include "sprite.h"
#include "table.h"
#include "text.h"
#include "tilemap.h"
#include "tiles.h"

namespace engine {

/* The scene's one object: the sprite the arrow keys move. Its speed is
   the engine's — pixels per second — and the clock's dt turns it into a
   per-frame step. */
constexpr double SPRITE_SPEED = 240.0; /* pixels per second */

/* Lesson 060: one buffer of stream per feed — one sixtieth of a second,
   the horizon the loop keeps queued. Lesson 062: a feed is always
   exactly this much stream — the sample's frames where the sample has
   them, silence beyond its end — so the horizon arithmetic is untouched
   whatever the sample's length is. The sample's own length is the file's
   fact: playback stops where its frame_count says it stops, not where a
   constant here would. */
constexpr int CHUNK_FRAMES = AUDIO_RATE / 60;   /* 735 */

/* Lesson 062: the buffer of stream one feed hands the device, filled
   from the sample (or with silence) as the feed is due. Static, like the
   platform layer's own staging buffers — the language law of lesson 026
   keeps allocation out of the run. */
static short stream[CHUNK_FRAMES];

/* Lesson 066: a loaded sample's facts, printed — the run's byte-level
   check on its two sounds. The peak is the largest frame the sample
   holds, and it is what says how much room the format still has above
   the sound. */
static void PrintSample(const char *name, const Sample &sample)
{
    int peak = 0;
    for (int i = 0; i < sample.frame_count; ++i) {
        int v = sample.frames[i * sample.channels];
        if (v < 0)
            v = -v;
        if (v > peak)
            peak = v;
    }
    std::printf("engine: %s: %d frames at %d Hz, %d channel%s, peak %d, first frames:",
                name, sample.frame_count, sample.rate, sample.channels,
                sample.channels == 1 ? "" : "s", peak);
    for (int i = 0; i < 8 && i < sample.frame_count; ++i)
        std::printf(" %d", (int)sample.frames[i]);
    std::printf(", last frame %d\n",
                sample.frame_count ? (int)sample.frames[sample.frame_count - 1]
                                   : 0);
}

/* Lesson 066: one asset load's whole failure path — a failed load is
   named typed and ends the run by name, exactly like the loads above it. */
static bool LoadRunSample(Arena &arena, const char *path, Sample &into)
{
    SampleResult loaded = LoadSample(arena, path);
    if (loaded.error == SAMPLE_OK) {
        into = loaded.sample;
        return true;
    }
    switch (loaded.error) {
    case SAMPLE_MISSING:
        std::fprintf(stderr, "engine: %s: could not load (missing)\n", path);
        break;
    case SAMPLE_MALFORMED:
        std::fprintf(stderr, "engine: %s: could not load (malformed)\n", path);
        break;
    default:
        std::fprintf(stderr, "engine: %s: could not load (no room)\n", path);
        break;
    }
    return false;
}

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

    /* The engine's memory: one arena over one reservation. Everything the
       engine allocates lives in here and is released together. */
    Arena arena;
    ArenaInit(arena, 32 * 1024 * 1024);
    Framebuffer *fb = GetFramebuffer(arena);

    /* The world's assets, loaded whole at startup (lessons 044-053):
       a sprite, a font, a map, and the map's tile art. Every load is a
       typed failure or a complete asset — and a failure ends the run by
       name. */
    SpriteResult loaded = LoadSprite(arena, "assets/sprite.ppm");
    if (loaded.error != SPRITE_OK) {
        std::fprintf(stderr, "engine: assets/sprite.ppm: could not load\n");
        platform::CloseWindow(opened.window);
        ArenaRelease(arena);
        return 1;
    }
    Sprite &sprite = loaded.sprite;

    FontResult font_loaded = LoadFont(arena, "assets/font.ppm");
    if (font_loaded.error != FONT_OK) {
        std::fprintf(stderr, "engine: assets/font.ppm: could not load\n");
        platform::CloseWindow(opened.window);
        ArenaRelease(arena);
        return 1;
    }
    Font &font = font_loaded.font;

    TileResult map_loaded = LoadTileMap(arena, "assets/map.txt");
    if (map_loaded.error != TILE_OK) {
        std::fprintf(stderr, "engine: assets/map.txt: could not load\n");
        platform::CloseWindow(opened.window);
        ArenaRelease(arena);
        return 1;
    }
    TileMap &map = map_loaded.map;

    TileSheetResult tiles_loaded = LoadTileSheet(arena, "assets/tiles.ppm",
                                                 map.kind_count);
    if (tiles_loaded.error != TILES_OK) {
        std::fprintf(stderr, "engine: assets/tiles.ppm: could not load\n");
        platform::CloseWindow(opened.window);
        ArenaRelease(arena);
        return 1;
    }
    TileSheet &sheet = tiles_loaded.sheet;

    /* Lesson 071: the run's entities are data. The table file holds one
       row per definition — its columns named by its header — and the load
       either hands over every definition or names what went wrong, like
       every asset above. Lesson 072: the rows are the arena's, and a
       refused load keeps none of them. */
    TableResult table_loaded = LoadTable(arena, "assets/entities.txt");
    if (table_loaded.error != TABLE_OK) {
        switch (table_loaded.error) {
        case TABLE_MISSING:
            std::fprintf(stderr,
                         "engine: assets/entities.txt: could not load (missing)\n");
            break;
        case TABLE_MALFORMED:
            std::fprintf(stderr,
                         "engine: assets/entities.txt: could not load (malformed)\n");
            break;
        default:
            std::fprintf(stderr,
                         "engine: assets/entities.txt: could not load (no room)\n");
            break;
        }
        platform::CloseWindow(opened.window);
        ArenaRelease(arena);
        return 1;
    }
    EntityTable &table = table_loaded.table;

    /* The byte-level check, before anything uses the table: every
       definition, carrying the values its row states. */
    std::printf("engine: table: %d definition%s\n", table.count,
                table.count == 1 ? "" : "s");
    for (int i = 0; i < table.count; ++i) {
        const EntityDef &def = table.rows[i];
        std::printf("engine: def %s: x %d y %d facing %d speed %d health %d sprite %s\n",
                    def.name, def.x, def.y, def.facing, def.speed, def.health,
                    def.sprite);
    }

    /* Lesson 073: the definitions' art, loaded at startup. The table's
       sprite column names the file; the run loads each one and hands the
       definition its image, so an entity created from a definition is
       answered from the definition alone. */
    Sprite *images = (Sprite *)ArenaAlloc(
        arena, (size_t)table.count * sizeof(Sprite), 4);
    if (!images) {
        std::fprintf(stderr, "engine: no room for the definitions' art\n");
        platform::CloseWindow(opened.window);
        ArenaRelease(arena);
        return 1;
    }
    for (int i = 0; i < table.count; ++i) {
        EntityDef &def = table.rows[i];
        SpriteResult art = LoadSprite(arena, def.sprite);
        if (art.error != SPRITE_OK) {
            std::fprintf(stderr, "engine: %s: could not load\n", def.sprite);
            platform::CloseWindow(opened.window);
            ArenaRelease(arena);
            return 1;
        }
        images[i] = art.sprite;
        def.image = &images[i];
    }

    /* Lesson 073: the game's first entity — created from the hero's
       definition, carrying the values its row states in named fields the
       game reads directly. Lesson 074: it lives in the store now, in a
       slot of the capacity decided up front. */
    DefResult hero_def = TableFind(table, "hero");
    if (hero_def.error != DEF_OK) {
        std::fprintf(stderr,
                     "engine: assets/entities.txt: no definition named \"hero\"\n");
        platform::CloseWindow(opened.window);
        ArenaRelease(arena);
        return 1;
    }
    EntityStore store = {};
    EntityResult hero_made = EntityCreate(store, *hero_def.def);
    if (hero_made.error != ENTITY_OK) {
        std::fprintf(stderr, "engine: the store refused the hero\n");
        platform::CloseWindow(opened.window);
        ArenaRelease(arena);
        return 1;
    }
    Entity &hero = *hero_made.entity;
    std::printf("engine: entity %s: x %d y %d facing %d speed %d health %d sprite %dx%d\n",
                hero.name, hero.x, hero.y, hero.facing, hero.speed, hero.health,
                hero.sprite->width, hero.sprite->height);

    /* Lesson 074: the creation policy — the first free slot, and never
       a live entity's. Lesson 075: the demo keeps a small world — eight
       entities, three of them killed before the first walk — so the
       store has free slots and the report can tell a freed slot from one
       that has never been used. */
    int created = 0;
    while (store.live < 8) {
        EntityResult made =
            EntityCreate(store, table.rows[store.live % table.count]);
        if (made.error != ENTITY_OK)
            break;
        created += 1;
    }
    store.slots[2].health = 0;
    store.slots[4].health = 0;
    store.slots[6].health = 0;
    std::printf("engine: store: live %d of %d — the hero and %d from the script; slots 2, 4, 6 killed\n",
                store.live, ENTITY_CAP, created);

    /* The lookup's typed failure, checked on purpose: a definition the
       table does not hold is a value — never an entity with assumed
       attributes. */
    DefResult unknown = TableFind(table, "dragon");
    std::printf("engine: table: \"dragon\" -> %s\n",
                unknown.error == DEF_OK ? "found" : "unknown");

    /* Lesson 066: the run's two sounds as files' bytes — the music that
       loops and the effect that plays once. Lesson 061's tone leaves the
       run here (it stays on disk: the file lessons 059-065 were built
       on); the game's own sounds are these two. Each load either yields
       the complete sample or names what went wrong, and a failure ends
       the run by name — like every asset above. */
    Sample music = {}, effect = {};
    if (!LoadRunSample(arena, "assets/music.wav", music) ||
        !LoadRunSample(arena, "assets/effect.wav", effect)) {
        platform::CloseWindow(opened.window);
        ArenaRelease(arena);
        return 1;
    }

    /* The byte-level check, before anything is played: each sound's facts,
       its peak, and its first frames — the same check lesson 061 made on
       its one file, now on both. */
    PrintSample("music", music);
    PrintSample("effect", effect);

    /* Lesson 068: the game's sound as the game has it — the music
       looping on the music channel and effects firing over it on the
       pool's channels, every one of them summed by the same MixBuffer
       into the one stream. Nothing in the mix knows which is which. */
    Mixer mixer;
    MixerInit(mixer);
    MixerPlayMusic(mixer, music, AUDIO_VOLUME_FULL);
    std::printf("engine: mix: music   -> channel %2d (looping, volume %d of %d)\n",
                AUDIO_MUSIC_CHANNEL, mixer.channels[AUDIO_MUSIC_CHANNEL].volume,
                AUDIO_VOLUME_FULL);

    /* The run's rhythm: a burst of effects at the start — up to three in
       flight — then one a second, all at a quarter volume so the music
       and the busiest moment still sum inside the format. */
    int effect_count = 0;
    int music_wraps = 0;

    double sprite_x = 312.0, sprite_y = 232.0;
    double started = platform::Now();
    double last = started;
    double distance = 0.0; /* the score: the world the sprite has walked */
    int shake_frames = 0; /* lesson 054: the additive hook's demo */
    bool was_blocked = false; /* lesson 056: the mover's state report */

    /* The demo's identity: what the run is, named at once — the world
       and its sound, one measured frame loop. */
    std::printf("engine: part 3 done — the world draws and the sound plays\n");
    std::printf("engine: world %dx%d cells (%dx%d px), %d kinds; %d glyphs; sprite %dx%d\n",
                map.width, map.height, map.width * TILE_SIZE,
                map.height * TILE_SIZE, map.kind_count, FONT_COUNT,
                sprite.width, sprite.height);
    std::printf("engine: sound %d-frame music looping on channel %d, %d-frame effect on the pool; one mixer of %d channels\n",
                music.frame_count, AUDIO_MUSIC_CHANNEL, effect.frame_count,
                AUDIO_MIXER_CHANNELS);
    std::printf("engine: arrow keys move the sprite, space shakes the camera; close the window to stop\n");
    std::printf("engine: sprite at %d,%d\n", (int)sprite_x, (int)sprite_y);

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
    } else {
        /* What the loop does with the sample: one buffer of stream per
           feed — the horizon the paced wait keeps queued. The buffer's
           length in time is the sample's own rate answering. */
        std::printf("engine: stream: %d-frame buffers, horizon %.1f ms; the loop feeds one when it is due\n",
                    CHUNK_FRAMES, 1e3 * CHUNK_FRAMES / music.rate);
    }

    /* The frame step: read news, update from polled state, feed the
       stream, draw, present — every phase measured, one record per
       frame. */
    int exit_code = 0;
    long frame_number = 0;
    FrameStats stats = {};
    Camera camera = { 0, 0, 0, 0 };

    /* Lesson 060: the run's own feeding schedule. The device consumes at
       the engine's rate, so the next buffer is due one horizon from the
       last one — and the loop knows that without asking the platform. */
    double next_feed = platform::Now();

    /* Lesson 068: the run's bookkeeping — how many buffers have been
       handed to the device, which drives the rhythm above. The mix's own
       account of what it carried is the frame record's audio phase now,
       measured like every other phase of the frame. */
    int feeds = 0;         /* buffers handed to the device */
    long walk_visits = 0;  /* lesson 075: entities visited by the walk */
    while (!platform::CloseRequested(opened.window)) {
        platform::PumpEvents(opened.window);
        if (platform::CloseRequested(opened.window))
            break;

        FrameRecord frame;
        frame.number = ++frame_number;
        double t0 = platform::Now();

        /* Update: a frame reads state — it never handles events. */
        double now = platform::Now();
        double dt = now - last;
        last = now;

        /* Lesson 075: the walk — every live entity, once per frame, in
           slot order. The per-entity work is expressed here, once, and
           not per type; today it is the demo's kill check. An entity
           retired in passing is not visited again and no other is
           skipped — the slots do not move under the walk. */
        int visited = 0;
        for (int i = 0; i < ENTITY_CAP; ++i) {
            if (!store.slots[i].live)
                continue;
            visited += 1;
            if (store.slots[i].health <= 0) {
                std::printf("engine: walk (frame %ld): retiring slot %d in passing\n",
                            frame.number, i);
                EntityRetire(store, store.slots[i]);
            }
        }
        walk_visits += visited;
        if (frame_number == 1)
            std::printf("engine: walk (frame 1): visited %d live entities, once each, in slot order; live %d of %d\n",
                        visited, store.live, ENTITY_CAP);

        /* Lesson 075: the reuse — three requests once the walk has
           freed three slots. Each lands in a freed slot, before any
           slot that has never been used. */
        if (frame_number == 2) {
            for (int k = 0; k < 3; ++k) {
                EntityResult made =
                    EntityCreate(store, table.rows[k % table.count]);
                if (made.error != ENTITY_OK)
                    break;
                std::printf("engine: store: created in slot %d (freed before never-used)\n",
                            (int)(made.entity - store.slots));
            }
            std::printf("engine: store: live %d of %d\n", store.live, ENTITY_CAP);
        }

        double was_x = sprite_x, was_y = sprite_y;
        double move_x = 0.0, move_y = 0.0;
        if (platform::KeyDown(opened.window, platform::KEY_LEFT))
            move_x -= SPRITE_SPEED * dt;
        if (platform::KeyDown(opened.window, platform::KEY_RIGHT))
            move_x += SPRITE_SPEED * dt;
        if (platform::KeyDown(opened.window, platform::KEY_UP))
            move_y -= SPRITE_SPEED * dt;
        if (platform::KeyDown(opened.window, platform::KEY_DOWN))
            move_y += SPRITE_SPEED * dt;

        /* Lesson 056: the mover — intent becomes motion only where the
           map allows it. One axis at a time, so a wall blocks the
           movement into it and the movement along it still works. */
        double next_x = sprite_x + move_x;
        if (!TileRectSolid(map, (int)next_x, (int)sprite_y, sprite.width,
                           sprite.height))
            sprite_x = next_x;
        double next_y = sprite_y + move_y;
        if (!TileRectSolid(map, (int)sprite_x, (int)next_y, sprite.width,
                           sprite.height))
            sprite_y = next_y;
        distance += (sprite_x > was_x ? sprite_x - was_x : was_x - sprite_x) +
                    (sprite_y > was_y ? sprite_y - was_y : was_y - sprite_y);

        /* The mover reports its state on transitions: moving, or pushed
           against something that will not move. */
        bool blocked = (move_x != 0.0 || move_y != 0.0) &&
                       sprite_x == was_x && sprite_y == was_y;
        if (blocked != was_blocked) {
            std::printf("engine: sprite %s at %d,%d (t=%.3f)\n",
                        blocked ? "blocked" : "unblocked", (int)sprite_x,
                        (int)sprite_y, platform::Now() - started);
            was_blocked = blocked;
        }
        if ((int)sprite_x != (int)was_x || (int)sprite_y != (int)was_y)
            std::printf("engine: sprite at %d,%d (t=%.3f)\n", (int)sprite_x,
                        (int)sprite_y, platform::Now() - started);

        /* Lesson 054: the camera's base follows the sprite — the world
           scrolls under the movement — clamped to the map's bounds. */
        int base_x = (int)sprite_x + sprite.width / 2 - FRAME_WIDTH / 2;
        int base_y = (int)sprite_y + sprite.height / 2 - FRAME_HEIGHT / 2;
        if (base_x < 0)
            base_x = 0;
        if (base_y < 0)
            base_y = 0;
        if (base_x > map.width * TILE_SIZE - FRAME_WIDTH)
            base_x = map.width * TILE_SIZE - FRAME_WIDTH;
        if (base_y > map.height * TILE_SIZE - FRAME_HEIGHT)
            base_y = map.height * TILE_SIZE - FRAME_HEIGHT;
        if (base_x != camera.base_x || base_y != camera.base_y) {
            camera.base_x = base_x;
            camera.base_y = base_y;
            std::printf("engine: camera base %d,%d (t=%.3f)\n", base_x,
                        base_y, platform::Now() - started);
        }

        /* The additive offset: the hook the juice toolkit will drive.
           Here SPACE demonstrates it — a shake that ends at zero, which
           is where it lives at rest. */
        if (platform::KeyPressed(opened.window, platform::KEY_SPACE) &&
            shake_frames <= 0) {
            shake_frames = 30;
            std::printf("engine: camera additive 6,0 (shake starts)\n");
        }
        if (shake_frames > 0) {
            --shake_frames;
            camera.add_x = (shake_frames % 2) ? 6 : -6;
            camera.add_y = 0;
            if (shake_frames == 0) {
                camera.add_x = 0;
                camera.add_y = 0;
                std::printf("engine: camera additive 0,0 (at rest)\n");
            }
        }

        frame.update = platform::Now() - t0;

        /* Lesson 060: the audio step — the loop feeds the device the next
           buffer of the stream, and only when the buffer is due. Input
           news can wake a frame early; a frame woken early must not queue
           extra audio, or the run would bury the device in buffers instead
           of pacing them. The step is measured on every frame — it is ~0
           where no buffer was due — so the phase accounts for all of the
           frame's audio work.

           Lesson 064: the stream is the mix. One buffer is every active
           channel's next frames summed and clamped — silence where no
           channel has anything to say. Lesson 066: frame_count is still
           the fact that says where a sample ends; a channel that loops
           wraps there instead of ending, and the mix does not know the
           difference. */
        double t_audio = platform::Now();
        if (audio.output && t_audio >= next_feed) {
            /* The run's rhythm, in the game's own terms: an effect every
               fifth buffer through the opening burst, then one every
               second — each one a MixerPlayEffect on the pool's channels,
               over the music that keeps looping. */
            bool fire = (feeds < 30 && feeds % 5 == 0) ||
                        (feeds >= 30 && feeds % 30 == 0);
            if (fire) {
                int ch = MixerPlayEffect(mixer, effect,
                                         AUDIO_VOLUME_FULL / 4);
                effect_count += 1;
                std::printf("engine: mix: effect %2d -> channel %2d (volume %d of %d)\n",
                            effect_count, ch, mixer.channels[ch].volume,
                            AUDIO_VOLUME_FULL);
            }

            int music_before = mixer.channels[AUDIO_MUSIC_CHANNEL].cursor;

            MixBuffer(mixer, stream, CHUNK_FRAMES);

            if (feeds == 0) {
                /* The mix's own bytes with both kinds of sound in it:
                   every frame is the music's and the effect's next frame
                   added — the same sum either way. */
                std::printf("engine: mix: first frames (music + effect 1, summed):");
                for (int i = 0; i < 8 && i < CHUNK_FRAMES; ++i)
                    std::printf(" %d", (int)stream[i]);
                std::printf("\n");
            }

            if (mixer.channels[AUDIO_MUSIC_CHANNEL].active &&
                mixer.channels[AUDIO_MUSIC_CHANNEL].cursor < music_before) {
                /* The wrap: the cursor went backwards — the loop's own
                   arithmetic, visible from outside the mixer. */
                music_wraps += 1;
                int cursor = mixer.channels[AUDIO_MUSIC_CHANNEL].cursor;
                std::printf("engine: loop: music wrapped on channel %d — wrap %d, %ld frames played, cursor %d of %d\n",
                            AUDIO_MUSIC_CHANNEL, music_wraps,
                            (long)music_wraps * music.frame_count + cursor,
                            cursor, music.frame_count);
            }

            if (!platform::SubmitSamples(audio.output, stream,
                                         CHUNK_FRAMES)) {
                /* A device that will not take the samples is named once,
                   not once per frame: the run closes the output and carries
                   on in silence — its wait unbounded again. */
                std::fprintf(stderr,
                             "engine: the output would not take the samples\n");
                platform::CloseAudioOutput(audio.output);
                audio.output = 0;
            }
            feeds += 1;
            /* The schedule restarts from now, not from the missed slot: a
               long frame is caught up by one buffer, never by a backlog. */
            next_feed = platform::Now() +
                        (double)CHUNK_FRAMES / (double)music.rate;
        }
        frame.audio = platform::Now() - t_audio;

        double t1 = platform::Now();

        /* Render: every frame draws the whole scene — clear, the world
           through the camera, and the HUD over it — each timed as its own
           named phase: the subsystems the frame record can name. */
        ClearBuffer(*fb, 32, 32, 64);
        double t_tilemap = platform::Now();
        DrawTileMap(*fb, map, sheet, -CameraX(camera), -CameraY(camera));
        frame.tilemap = platform::Now() - t_tilemap;
        double t_sprites = platform::Now();
        BlitSprite(*fb, sprite, (int)sprite_x - CameraX(camera),
                   (int)sprite_y - CameraY(camera));
        frame.sprites = platform::Now() - t_sprites;
        double t_text = platform::Now();
        char score_line[32];
        std::snprintf(score_line, sizeof score_line, "SCORE %06d",
                      (int)distance);
        DrawText(*fb, font, score_line, 8, 8);
        char pos_line[32];
        std::snprintf(pos_line, sizeof pos_line, "X %3d Y %3d",
                      (int)sprite_x, (int)sprite_y);
        DrawText(*fb, font, pos_line, 8, 8 + FONT_CELL + 4);
        frame.text = platform::Now() - t_text;

        frame.render = platform::Now() - t1;
        double t2 = platform::Now();

        if (!platform::Present(opened.window, fb->pixels, fb->width,
                               fb->height)) {
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
           phase (lesson 060) joins in the record's own order. */
        std::printf("frame %ld: update %.3f ms, audio %.3f ms, render %.3f ms (sprites %.3f, text %.3f, tilemap %.3f), present %.3f ms, total %.3f ms\n",
                    frame.number, frame.update * 1e3, frame.audio * 1e3,
                    frame.render * 1e3,
                    frame.sprites * 1e3, frame.text * 1e3,
                    frame.tilemap * 1e3, frame.present * 1e3,
                    frame.total * 1e3);
    }

    /* The demo's account: what the run did — the world's frames and the
       sound's buffers, together — before the cost's table below. */
    std::printf("engine: demo: %ld frames measured, %d buffers fed, %d effects fired, %d music wraps\n",
                frame_number, feeds, effect_count, music_wraps);

    /* Lesson 075: the walk's account — one visit per live entity per
       frame, and nothing else. */
    std::printf("engine: walk: %ld visits over %ld frames — one per live entity per frame\n",
                walk_visits, frame_number);

    /* The account as the frame-budget table (lesson 058): the frame
       count, the average, the worst frame — and the render attributed to
       its subsystems, the report Part 5's finale grows. */
    PrintFrameBudget(stats);
    std::printf("engine: arena: %zu of %zu bytes used\n", arena.used,
                arena.memory.size);

    if (platform::CloseRequested(opened.window))
        std::printf("engine: close reported\n");
    platform::CloseAudioOutput(audio.output);
    platform::CloseWindow(opened.window);
    ArenaRelease(arena);
    std::printf("engine: closed\n");
    return exit_code;
}

} /* namespace engine */

int main(void)
{
    return engine::Run();
}
