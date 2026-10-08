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

#include <cmath>
#include <cstdio>

#include "arena.h"
#include "audio.h"
#include "blit.h"
#include "combat.h"
#include "entity.h"
#include "feel.h"
#include "font.h"
#include "framebuffer.h"
#include "frame.h"
#include "game.h"
#include "gametime.h"
#include "hero.h"
#include "platform.h"
#include "sprite.h"
#include "table.h"
#include "text.h"
#include "tilemap.h"
#include "tiles.h"

namespace engine {

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

/* Lesson 087: one table load's whole failure path, the same shape — the
   load either hands over every definition or names what went wrong typed
   and the run ends by name. Used for every table file the game loads. */
static bool LoadRunTable(Arena &arena, const char *path, EntityTable &into)
{
    TableResult loaded = LoadTable(arena, path);
    if (loaded.error == TABLE_OK) {
        into = loaded.table;
        return true;
    }
    switch (loaded.error) {
    case TABLE_MISSING:
        std::fprintf(stderr, "engine: %s: could not load (missing)\n", path);
        break;
    case TABLE_MALFORMED:
        std::fprintf(stderr, "engine: %s: could not load (malformed)\n", path);
        break;
    default:
        std::fprintf(stderr, "engine: %s: could not load (no room)\n", path);
        break;
    }
    return false;
}

/* Lesson 073/087: the definitions' art, loaded at startup. The sprite
   column names the file; the run loads each one and hands the definition
   its image, so an entity created from the definition is answered from
   the definition alone. A row that names no sprite (a weapon row) has no
   art and needs none. */
static bool LoadRunArt(Arena &arena, EntityTable &table)
{
    Sprite *images = (Sprite *)ArenaAlloc(
        arena, (size_t)table.count * sizeof(Sprite), 4);
    if (!images) {
        std::fprintf(stderr, "engine: no room for the definitions' art\n");
        return false;
    }
    for (int i = 0; i < table.count; ++i) {
        EntityDef &def = table.rows[i];
        if (!def.sprite[0])
            continue;
        SpriteResult art = LoadSprite(arena, def.sprite);
        if (art.error != SPRITE_OK) {
            std::fprintf(stderr, "engine: %s: could not load\n", def.sprite);
            return false;
        }
        images[i] = art.sprite;
        def.image = &images[i];
    }
    return true;
}

/* Lesson 087: the byte-level check on a table, before anything uses it —
   every definition, carrying every field: the values its row states and
   the format's defaults for the columns its file did not name. */
static void PrintDefs(const char *path, const EntityTable &table)
{
    std::printf("engine: table %s: %d definition%s\n", path, table.count,
                table.count == 1 ? "" : "s");
    for (int i = 0; i < table.count; ++i) {
        const EntityDef &def = table.rows[i];
        std::printf("engine: def %s: x %d y %d facing %d speed %d health %d sprite %s accel %d damage %d rate %d fires %s range %d behavior %s wave %d count %d\n",
                    def.name, def.x, def.y, def.facing, def.speed, def.health,
                    def.sprite[0] ? def.sprite : "none", def.accel, def.damage,
                    def.rate, def.fires[0] ? def.fires : "none", def.range,
                    BehaviorName(def.behavior), def.wave, def.count);
    }
}

/* Lesson 088: one live entity, carrying its row's values — printed in
   the same words as the definition above, so the carrying is checkable
   by eye against the file's rows. The sprite prints as its dimensions
   because the entity carries the row's art *loaded* — the image, not
   the path that named it. */
static void PrintEntity(const char *kind, const Entity &e)
{
    std::printf("engine: %s %s: x %d y %d facing %d speed %d health %d sprite %dx%d accel %d damage %d rate %d fires %s range %d behavior %s wave %d count %d\n",
                kind, e.name, (int)e.x, (int)e.y, e.facing, e.speed, e.health,
                e.sprite->width, e.sprite->height, e.accel, e.damage, e.rate,
                e.fires[0] ? e.fires : "none", e.range, BehaviorName(e.behavior),
                e.wave, e.count);
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
       a font, a map, and the map's tile art — and, since lesson 073,
       the art each definition names. Every load is a typed failure or a
       complete asset — and a failure ends the run by name. */
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

    /* Lesson 071: the run's entities are data. A table file holds one
       row per definition — its columns named by its header — and the load
       either hands over every definition or names what went wrong, like
       every asset above. Lesson 072: the rows are the arena's, and a
       refused load keeps none of them. Lesson 087: the format grew by
       named columns — and this file keeps loading byte-for-byte, its
       seven columns exactly as lesson 071 wrote them, every field it
       never named at the format's default. */
    EntityTable table;
    if (!LoadRunTable(arena, "assets/entities.txt", table)) {
        platform::CloseWindow(opened.window);
        ArenaRelease(arena);
        return 1;
    }

    /* Lesson 087: the game's own data, in the grown format. The weapons
       are rows that name the projectile kind they fire and carry their
       rate and damage; the projectile kinds are rows a fired shot is an
       entity of. Each file's header names the columns it uses — and only
       those; what it leaves unnamed sits at the format's defaults.
       Lesson 088: and the enemy roster — the three types and the boss,
       every per-type fact its own row's value. */
    EntityTable weapons, shots, foes;
    if (!LoadRunTable(arena, "assets/weapons.txt", weapons) ||
        !LoadRunTable(arena, "assets/projectiles.txt", shots) ||
        !LoadRunTable(arena, "assets/enemies.txt", foes)) {
        platform::CloseWindow(opened.window);
        ArenaRelease(arena);
        return 1;
    }

    /* The byte-level check, before anything uses the tables: every
       definition of every table, carrying every field — the values its
       row states and the format's defaults for the columns its file did
       not name. */
    std::printf("engine: table: unnamed fields at their defaults — accel %d, damage 0, rate 0, fires none, range 0, behavior none, wave 0, count 1\n",
                TABLE_ACCEL_DEFAULT);
    PrintDefs("assets/entities.txt", table);
    PrintDefs("assets/weapons.txt", weapons);
    PrintDefs("assets/projectiles.txt", shots);
    PrintDefs("assets/enemies.txt", foes);

    /* Lesson 073: the definitions' art, loaded at startup. A row that
       names no sprite (a weapon row) has no art and needs none. */
    if (!LoadRunArt(arena, table) || !LoadRunArt(arena, shots) ||
        !LoadRunArt(arena, foes)) {
        platform::CloseWindow(opened.window);
        ArenaRelease(arena);
        return 1;
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
    std::printf("engine: entity %s: x %.0f y %.0f facing %d speed %d health %d sprite %dx%d\n",
                hero.name, hero.x, hero.y, hero.facing, hero.speed, hero.health,
                hero.sprite->width, hero.sprite->height);

    /* Lesson 082: the game-state machine. The game is a state now, not a
       loop with flags — it starts on the title screen, each state owns
       its screen and its input, and the transitions are named conditions
       (D7). The hero's starting health is the row's fact, handed to the
       machine so a fresh game can restore it. */
    Game game;
    GameInit(game, hero.health);

    /* Lesson 086: the feedback hooks — a screenshake and a hitstop, both
       at rest. The juice toolkit (lessons 092-093) will fire these from
       the game's events; here a demonstration fires both once so their
       fire-and-rest life is visible. */
    Feedback feel;
    FeelInit(feel);
    bool feel_demo = false;

    /* Lesson 080: the vertical slice — the game's shape, and nothing
       else. The hero is the row the game asks for by name (it is the
       one the player controls); the world's other kinds come from the
       same table, one entity per row. A new row is a new entity; the
       run has no per-kind code to grow. */
    int created = 1;
    for (int i = 0; i < table.count; ++i) {
        if (&table.rows[i] == hero_def.def)
            continue;
        EntityResult made = EntityCreate(store, table.rows[i]);
        if (made.error != ENTITY_OK) {
            std::fprintf(stderr, "engine: the store refused %s\n",
                         table.rows[i].name);
            platform::CloseWindow(opened.window);
            ArenaRelease(arena);
            return 1;
        }
        /* Lesson 084: a non-hero entity walked (down-right) here — a
           stand-in for the AI. Lesson 089 replaced it: the behaviors
           are real now, and the world's kinds move the ways their rows
           say (the slime's row says `none`, so it stands). */
        created += 1;
    }
    std::printf("engine: world: %d entities from the table's rows, live %d of %d\n",
                created, store.live, ENTITY_CAP);

    /* Lesson 088: the enemy roster is data. The three types and the
       boss are rows of the game's table; each row becomes one entity,
       carrying its row's values in named fields the game reads
       directly. A new row is a new enemy — the run has no per-kind code
       to grow, and no per-type copy of any attribute to keep honest. */
    for (int i = 0; i < foes.count; ++i) {
        EntityResult made = EntityCreate(store, foes.rows[i]);
        if (made.error != ENTITY_OK) {
            std::fprintf(stderr, "engine: the store refused %s\n",
                         foes.rows[i].name);
            platform::CloseWindow(opened.window);
            ArenaRelease(arena);
            return 1;
        }
        PrintEntity("entity", *made.entity);
    }
    std::printf("engine: roster: %d enemies from the table's rows, live %d of %d\n",
                foes.count, store.live, ENTITY_CAP);

    /* Lesson 087: weapons are rows. The hero starts armed with the
       weapons table's first row; the number keys arm the rest
       (HeroFire). Lesson 090: the enemy-fire stand-in and its key are
       gone — the enemy rows carry their own weapons and the walk's
       attack fires them. */
    if (weapons.count > 0)
        CombatArm(hero, weapons.rows[0]);

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

    double started = platform::Now();
    double last = started;
    double distance = 0.0; /* the score: the world the hero has walked */
    GameTime game_time = { GAMETIME_FULL }; /* lesson 078: the scale — set by the state now */
    bool was_blocked = false; /* lesson 077: the mover's state report */
    int was_vx = 0, was_vy = 0; /* lesson 085: the hero's velocity, as it eases */
    int was_frame = 0;          /* lesson 086: the hero's walk-cycle frame */
    double seen_x[ENTITY_CAP] = {}, seen_y[ENTITY_CAP] = {}; /* lesson 089:
                                  where each entity was last reported */

    /* The slice's identity: what the run is, named at once — L0*, the
       gate this part closes on. Every service it uses was finished
       before this lesson; the lesson is the fit. */
    std::printf("engine: part 4 done — the vertical slice: a hero walks the tilemap, the camera follows\n");
    std::printf("engine: world %dx%d cells (%dx%d px), %d kinds; %d glyphs; hero %dx%d\n",
                map.width, map.height, map.width * TILE_SIZE,
                map.height * TILE_SIZE, map.kind_count, FONT_COUNT,
                hero.sprite->width, hero.sprite->height);
    std::printf("engine: sound %d-frame music looping on channel %d, %d-frame effect on the pool; one mixer of %d channels\n",
                music.frame_count, AUDIO_MUSIC_CHANNEL, effect.frame_count,
                AUDIO_MIXER_CHANNELS);
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

        /* Lesson 086: the feedback hooks run on their own wall-time —
           each fires, decays, and rests. The demonstration fires both
           once, in play, so their fire-and-rest life is visible; the
           juice toolkit (lessons 092-093) will fire them from the game's
           events instead of this script. */
        if (!feel_demo && game.state == GAME_PLAY &&
            platform::Now() - started >= 3.0) {
            feel_demo = true;
            FeelShake(feel, 6.0, 0.5);
            FeelHitstop(feel, 0.25, 0.4);
            std::printf("engine: feel: shake fired (6 px, 0.5s), hitstop fired (0.25x, 0.4s)\n");
        }
        FeelUpdate(feel, wall_dt, game.camera);

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
            HeroFire(hero, opened.window, weapons, shots, store, dt);

        }

        /* Lesson 084: the game resolves its movement against its map —
           the walk is the game's now (GameWalk, in game.cpp), turning
           every live entity's request into motion through the mover (and
           every projectile into its flight). The loop times it as the
           frame record's entity sub-phase. */
        double t_entities = platform::Now();
        int visited = GameWalk(store, map, hero, shots, dt);
        frame.entities = platform::Now() - t_entities;
        walk_visits += visited;

        /* Lesson 089: the world's motion, as the behaviors produce it —
           every non-hero entity reported as it travels about a tile, its
           distance to the hero beside it (the number all three behaviors
           are about: chase shrinks it, flee grows it, keep holds it). */
        for (int i = 0; i < ENTITY_CAP; ++i) {
            Entity &e = store.slots[i];
            if (!e.live || &e == &hero)
                continue;
            double dx = e.x - seen_x[i], dy = e.y - seen_y[i];
            if (dx * dx + dy * dy < 24.0 * 24.0)
                continue;
            seen_x[i] = e.x;
            seen_y[i] = e.y;
            double to_x = hero.x - e.x, to_y = hero.y - e.y;
            std::printf("engine: %s at %d,%d — %d px of the hero (t=%.3f)\n",
                        e.name, (int)e.x, (int)e.y,
                        (int)std::sqrt(to_x * to_x + to_y * to_y),
                        platform::Now() - started);
        }

        /* The score, and the hero's own report: where the entity the
           game moves has got to. */
        distance += (hero.x > was_x ? hero.x - was_x : was_x - hero.x) +
                    (hero.y > was_y ? hero.y - was_y : was_y - hero.y);

        /* Lesson 077: the mover's state report, on transitions — the
           hero moving, or pushed against something that will not move. */
        bool blocked = (hero.move_x != 0.0 || hero.move_y != 0.0) &&
                       hero.x == was_x && hero.y == was_y;
        if (blocked != was_blocked) {
            std::printf("engine: hero %s at %d,%d (t=%.3f)\n",
                        blocked ? "blocked" : "unblocked", (int)hero.x,
                        (int)hero.y, platform::Now() - started);
            was_blocked = blocked;
        }
        if ((int)hero.x != (int)was_x || (int)hero.y != (int)was_y)
            std::printf("engine: hero at %d,%d (t=%.3f)\n", (int)hero.x,
                        (int)hero.y, platform::Now() - started);

        /* Lesson 085: the hero's velocity, as it eases — the accel (the
           speed rising over frames) and the decel (falling to rest) are
           what the player feels, and this is the measurement of it. */
        {
            int vx = (int)(hero.move_x * hero.speed);
            int vy = (int)(hero.move_y * hero.speed);
            if (vx != was_vx || vy != was_vy) {
                std::printf("engine: hero velocity %d,%d (t=%.3f)\n", vx, vy,
                            platform::Now() - started);
                was_vx = vx;
                was_vy = vy;
            }
        }

        /* Lesson 086: the walk cycle — the frame advances while the hero
           steps, and this is the measurement of it advancing. */
        if (hero.frame != was_frame) {
            std::printf("engine: hero frame %d (t=%.3f)\n", hero.frame,
                        platform::Now() - started);
            was_frame = hero.frame;
        }

        /* Lesson 083: the game's world-view — the camera's base follows
           the hero, clamped to the map's bounds, and its additive offset
           rests at exactly zero. The game owns the camera now (GameFollow,
           in game.cpp); the loop keeps no camera of its own. */
        GameFollow(game, hero, map);

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

        /* Render: the current state's screen, and only that one (lesson
           082). The backdrop is the state's own — the world's blue in
           play, the panel's darker blue on the panel screens — cleared
           once here, in the render phase, before the named sub-phases. */
        if (game.state == GAME_PLAY)
            ClearBuffer(*fb, 32, 32, 64);
        else
            ClearBuffer(*fb, 24, 24, 40);
        if (game.state == GAME_PLAY) {
            /* Lesson 083: the game draws its own world — the scrolling
               map and the live entities, through the game's camera. The
               loop times the two the way it always has, as the frame
               record's named sub-phases. */
            double t_tilemap = platform::Now();
            GameDrawMap(game, *fb, map, sheet);
            frame.tilemap = platform::Now() - t_tilemap;
            double t_sprites = platform::Now();
            GameDrawSprites(game, *fb, store);
            frame.sprites = platform::Now() - t_sprites;
            double t_text = platform::Now();
            char score_line[32];
            std::snprintf(score_line, sizeof score_line, "SCORE %06d",
                          (int)distance);
            DrawText(*fb, font, score_line, 8, 8);
            char pos_line[32];
            std::snprintf(pos_line, sizeof pos_line, "X %3d Y %3d",
                          (int)hero.x, (int)hero.y);
            DrawText(*fb, font, pos_line, 8, 8 + FONT_CELL + 4);
            frame.text = platform::Now() - t_text;
        } else {
            /* The state's own screen. The world is frozen outside play —
               the simulation stands still — and the panel is what the
               window shows. */
            double t_text = platform::Now();
            GameDrawPanel(game, *fb, font);
            frame.text = platform::Now() - t_text;
        }

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
           phase (lesson 060) joins in the record's own order, and
           lesson 079's step leads it: the game's advance beside the
           machine's durations. */
        std::printf("frame %ld: step %.3f ms, update %.3f ms (entities %.3f), audio %.3f ms, render %.3f ms (sprites %.3f, text %.3f, tilemap %.3f), present %.3f ms, total %.3f ms\n",
                    frame.number, frame.step * 1e3, frame.update * 1e3,
                    frame.entities * 1e3,
                    frame.audio * 1e3,
                    frame.render * 1e3,
                    frame.sprites * 1e3, frame.text * 1e3,
                    frame.tilemap * 1e3, frame.present * 1e3,
                    frame.total * 1e3);
    }

    /* The demo's account: what the run did — the world's frames and the
       sound's buffers, together — before the cost's table below. */
    std::printf("engine: demo: %ld frames measured, %d buffers fed, %d effects fired, %d music wraps\n",
                frame_number, feeds, effect_count, music_wraps);

    /* Lesson 089: where the behaviors left the world — every live
       entity's position and its distance to the hero, the number all
       three behaviors are about (chase shrinks it, flee grows it, keep
       holds it). The report above samples a moving world; this one
       states where it ended. */
    for (int i = 0; i < ENTITY_CAP; ++i) {
        Entity &e = store.slots[i];
        if (!e.live || &e == &hero)
            continue;
        double to_x = hero.x - e.x, to_y = hero.y - e.y;
        std::printf("engine: world: %s ends at %d,%d — %d px of the hero\n",
                    e.name, (int)e.x, (int)e.y,
                    (int)std::sqrt(to_x * to_x + to_y * to_y));
    }

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
