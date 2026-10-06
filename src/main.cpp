// main.cpp — the engine: one measured frame loop, drawing the world.
//
// Lesson 057: the Part 2 closing demo. Every capability of the software
// renderer at once — the map drawn through the camera, the sprite moved
// by polled input and stopped by the map, text laid out over it all —
// and every phase measured, one record per frame. Nothing is invented
// here; today the parts fit, and the fit is what the demo shows. The
// language law of lesson 026 still holds over all of it.

#include <cstdio>

#include "arena.h"
#include "audio.h"
#include "blit.h"
#include "camera.h"
#include "font.h"
#include "framebuffer.h"
#include "frame.h"
#include "platform.h"
#include "sprite.h"
#include "text.h"
#include "tilemap.h"
#include "tiles.h"

namespace engine {

/* The scene's one object: the sprite the arrow keys move. Its speed is
   the engine's — pixels per second — and the clock's dt turns it into a
   per-frame step. */
constexpr double SPRITE_SPEED = 240.0; /* pixels per second */

/* Lesson 059: the run's sound is half a second of tone — 22050 frames.
   One second of a 440 Hz tone is exactly 440 cycles at AUDIO_RATE, so
   this buffer holds a whole 220 cycles and its end meets its beginning
   with no click: the wrap lesson 059 found, which lesson 060's feeding
   cursor leans on when it hands the same run of frames to the device
   again and again. */
constexpr int TONE_FRAMES = AUDIO_RATE / 2; /* 22050 */

/* Lesson 060: one buffer of stream per feed — one sixtieth of a second,
   the horizon the loop keeps queued. The cursor relies on the tone's
   length dividing evenly into buffers: 22050 / 735 = 30 exactly, so it
   wraps at a buffer boundary and no feed ever has to copy across the
   tone's end. */
constexpr int CHUNK_FRAMES = AUDIO_RATE / 60;   /* 735 */

/* Lesson 054: the scene, drawn through the camera. The camera's summed
   offset is applied once, at each draw's origin — the map's and the
   sprite's. The HUD is not scene and does not pass through here. */
static void DrawScene(Framebuffer &fb, const TileMap &map,
                      const TileSheet &sheet, const Sprite &sprite,
                      int sprite_x, int sprite_y, const Camera &camera)
{
    int x = CameraX(camera);
    int y = CameraY(camera);
    DrawTileMap(fb, map, sheet, -x, -y);
    BlitSprite(fb, sprite, sprite_x - x, sprite_y - y);
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

    /* Lesson 061: the run's sound as a file's bytes. A sample is frames
       of amplitude in a container, and the load either yields the
       complete sample or names what went wrong — like every asset above.
       A failure ends the run by name, like every asset above. */
    SampleResult sample_loaded = LoadSample(arena, "assets/tone.wav");
    if (sample_loaded.error != SAMPLE_OK) {
        switch (sample_loaded.error) {
        case SAMPLE_MISSING:
            std::fprintf(stderr,
                         "engine: assets/tone.wav: could not load (missing)\n");
            break;
        case SAMPLE_MALFORMED:
            std::fprintf(stderr,
                         "engine: assets/tone.wav: could not load (malformed)\n");
            break;
        default:
            std::fprintf(stderr,
                         "engine: assets/tone.wav: could not load (no room)\n");
            break;
        }
        platform::CloseWindow(opened.window);
        ArenaRelease(arena);
        return 1;
    }
    Sample &sample = sample_loaded.sample;

    /* The byte-level check, before anything is played: the sample's facts
       and its first frames — the same bytes lesson 059 computed, now read
       from a file instead. */
    std::printf("engine: sample: %d frames at %d Hz, %d channel%s, first frames:",
                sample.frame_count, sample.rate, sample.channels,
                sample.channels == 1 ? "" : "s");
    for (int i = 0; i < 8 && i < sample.frame_count; ++i)
        std::printf(" %d", (int)sample.frames[i]);
    std::printf("\n");

    double sprite_x = 312.0, sprite_y = 232.0;
    double started = platform::Now();
    double last = started;
    double distance = 0.0; /* the score: the world the sprite has walked */
    int shake_frames = 0; /* lesson 054: the additive hook's demo */
    bool was_blocked = false; /* lesson 056: the mover's state report */

    std::printf("engine: part 2 done — the software renderer draws the world\n");
    std::printf("engine: world %dx%d cells (%dx%d px), %d kinds; %d glyphs; sprite %dx%d\n",
                map.width, map.height, map.width * TILE_SIZE,
                map.height * TILE_SIZE, map.kind_count, FONT_COUNT,
                sprite.width, sprite.height);
    std::printf("engine: arrow keys move the sprite, space shakes the camera; close the window to stop\n");
    std::printf("engine: sprite at %d,%d\n", (int)sprite_x, (int)sprite_y);

    /* Lesson 059: the run's sound. A sample is a frame of amplitude at
       the engine's rate — here a tone computed by code instead of read
       from a file — and the seam's audio output is what puts those frames
       in front of a device. Lesson 060: those frames are a *stream*, not
       one submission done at startup — the frame loop feeds the device
       buffer by buffer, for as long as the run lasts. */
    platform::AudioResult audio =
        platform::OpenAudioOutput(AUDIO_RATE, AUDIO_OUTPUT_CHANNELS);
    short *tone = 0;
    int tone_cursor = 0; /* where the stream's next buffer starts */
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
        tone = (short *)ArenaAlloc(arena, TONE_FRAMES * sizeof(short),
                                   sizeof(short));
        if (!tone) {
            std::fprintf(stderr, "engine: no room for the tone\n");
        } else {
            GenerateTone(tone, TONE_FRAMES, 440.0, 0.25);

            /* The bytes are checkable before they are audible: the first
               frames of the tone, in the engine's own format. */
            std::printf("engine: tone: %d frames at %d Hz, first frames:",
                        TONE_FRAMES, AUDIO_RATE);
            for (int i = 0; i < 4; ++i)
                std::printf(" %d", (int)tone[i]);
            std::printf("\n");

            /* And what the loop does with them: one buffer of stream per
               feed — the horizon the paced wait keeps queued. */
            std::printf("engine: stream: %d-frame buffers, horizon %.1f ms; the loop feeds one when it is due\n",
                        CHUNK_FRAMES, 1e3 * CHUNK_FRAMES / AUDIO_RATE);
        }
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
           frame's audio work. */
        double t_audio = platform::Now();
        if (audio.output && tone && t_audio >= next_feed) {
            if (platform::SubmitSamples(audio.output, tone + tone_cursor,
                                        CHUNK_FRAMES)) {
                tone_cursor = (tone_cursor + CHUNK_FRAMES) % TONE_FRAMES;
            } else {
                /* A device that will not take the samples is named once,
                   not once per frame: the run closes the output and carries
                   on in silence — its wait unbounded again. */
                std::fprintf(stderr,
                             "engine: the output would not take the samples\n");
                platform::CloseAudioOutput(audio.output);
                audio.output = 0;
            }
            /* The schedule restarts from now, not from the missed slot: a
               long frame is caught up by one buffer, never by a backlog. */
            next_feed = platform::Now() +
                        (double)CHUNK_FRAMES / (double)AUDIO_RATE;
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
