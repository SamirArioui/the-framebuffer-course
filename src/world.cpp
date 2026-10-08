// world.cpp — the world the run plays in: the startup, whole.
//
// Lesson 097: moved whole from Run() — the same sequence, the same
// typed failures, the same byte-level checks, in the order the run has
// always reported them. A move, nothing more.

#include "world.h"

#include <cstdio>

#include "combat.h"
#include "load.h"

namespace engine {

bool WorldStart(World &world, Game &game, Feedback &feel)
{
    /* The engine's memory: one arena over one reservation. Everything the
       engine allocates lives in here and is released together. */
    ArenaInit(world.arena, 32 * 1024 * 1024);
    world.fb = GetFramebuffer(world.arena);

    /* The world's assets, loaded whole at startup (lessons 044-053):
       a font, a map, and the map's tile art — and, since lesson 073,
       the art each definition names. Every load is a typed failure or a
       complete asset — and a failure ends the run by name. */
    FontResult font_loaded = LoadFont(world.arena, "assets/font.ppm");
    if (font_loaded.error != FONT_OK) {
        std::fprintf(stderr, "engine: assets/font.ppm: could not load\n");
        return false;
    }
    world.font = font_loaded.font;

    TileResult map_loaded = LoadTileMap(world.arena, "assets/map.txt");
    if (map_loaded.error != TILE_OK) {
        std::fprintf(stderr, "engine: assets/map.txt: could not load\n");
        return false;
    }
    world.map = map_loaded.map;

    TileSheetResult tiles_loaded = LoadTileSheet(world.arena, "assets/tiles.ppm",
                                                world.map.kind_count);
    if (tiles_loaded.error != TILES_OK) {
        std::fprintf(stderr, "engine: assets/tiles.ppm: could not load\n");
        return false;
    }
    world.sheet = tiles_loaded.sheet;

    /* Lesson 071: the run's entities are data. A table file holds one
       row per definition — its columns named by its header — and the load
       either hands over every definition or names what went wrong, like
       every asset above. Lesson 072: the rows are the arena's, and a
       refused load keeps none of them. Lesson 087: the format grew by
       named columns — and this file keeps loading byte-for-byte, its
       seven columns exactly as lesson 071 wrote them, every field it
       never named at the format's default. */
    if (!LoadRunTable(world.arena, "assets/entities.txt", world.table))
        return false;

    /* Lesson 087: the game's own data, in the grown format. The weapons
       are rows that name the projectile kind they fire and carry their
       rate and damage; the projectile kinds are rows a fired shot is an
       entity of. Each file's header names the columns it uses — and only
       those; what it leaves unnamed sits at the format's defaults.
       Lesson 088: and the enemy roster — the three types and the boss,
       every per-type fact its own row's value. Lesson 093: and the
       toolkit's particle kinds — cosmetic entities from rows like every
       other kind, the burst's art and settle in the table's columns. */
    if (!LoadRunTable(world.arena, "assets/weapons.txt", world.weapons) ||
        !LoadRunTable(world.arena, "assets/projectiles.txt", world.shots) ||
        !LoadRunTable(world.arena, "assets/enemies.txt", world.foes) ||
        !LoadRunTable(world.arena, "assets/particles.txt", world.particles))
        return false;

    /* The byte-level check, before anything uses the tables: every
       definition of every table, carrying every field — the values its
       row states and the format's defaults for the columns its file did
       not name. */
    std::printf("engine: table: unnamed fields at their defaults — accel %d, damage 0, rate 0, fires none, range 0, behavior none, wave 0, count 1\n",
                TABLE_ACCEL_DEFAULT);
    PrintDefs("assets/entities.txt", world.table);
    PrintDefs("assets/weapons.txt", world.weapons);
    PrintDefs("assets/projectiles.txt", world.shots);
    PrintDefs("assets/enemies.txt", world.foes);
    PrintDefs("assets/particles.txt", world.particles);

    /* Lesson 073: the definitions' art, loaded at startup. A row that
       names no sprite (a weapon row) has no art and needs none. */
    if (!LoadRunArt(world.arena, world.table) ||
        !LoadRunArt(world.arena, world.shots) ||
        !LoadRunArt(world.arena, world.foes) ||
        !LoadRunArt(world.arena, world.particles))
        return false;

    /* Lesson 073: the game's first entity — created from the hero's
       definition, carrying the values its row states in named fields the
       game reads directly. Lesson 074: it lives in the store now, in a
       slot of the capacity decided up front. */
    DefResult hero_def = TableFind(world.table, "hero");
    if (hero_def.error != DEF_OK) {
        std::fprintf(stderr,
                     "engine: assets/entities.txt: no definition named \"hero\"\n");
        return false;
    }
    EntityResult hero_made = EntityCreate(world.store, *hero_def.def);
    if (hero_made.error != ENTITY_OK) {
        std::fprintf(stderr, "engine: the store refused the hero\n");
        return false;
    }
    world.hero = hero_made.entity;
    std::printf("engine: entity %s: x %.0f y %.0f facing %d speed %d health %d sprite %dx%d\n",
                world.hero->name, world.hero->x, world.hero->y,
                world.hero->facing, world.hero->speed, world.hero->health,
                world.hero->sprite->width, world.hero->sprite->height);

    /* Lesson 082: the game-state machine. The game is a state now, not a
       loop with flags — it starts on the title screen, each state owns
       its screen and its input, and the transitions are named conditions
       (D7). The hero's starting health is the row's fact, handed to the
       machine so a fresh game can restore it. */
    GameInit(game, world.hero->health);

    /* Lesson 086: the feedback hooks — a screenshake and a hitstop, both
       at rest. Lesson 092: the juice toolkit fires them from the game's
       own events now — a hit lands, a death falls — in the event's own
       frame (the walk's flight, in game.cpp/combat.cpp); the wall-time
       demonstration that used to fire them here is gone. */
    FeelInit(feel);

    /* Lesson 080: the vertical slice — the game's shape, and nothing
       else. The hero is the row the game asks for by name (it is the
       one the player controls); the world's other kinds come from the
       same table, one entity per row. A new row is a new entity; the
       run has no per-kind code to grow. */
    int created = 1;
    for (int i = 0; i < world.table.count; ++i) {
        if (&world.table.rows[i] == hero_def.def)
            continue;
        EntityResult made = EntityCreate(world.store, world.table.rows[i]);
        if (made.error != ENTITY_OK) {
            std::fprintf(stderr, "engine: the store refused %s\n",
                         world.table.rows[i].name);
            return false;
        }
        /* Lesson 084: a non-hero entity walked (down-right) here — a
           stand-in for the AI. Lesson 089 replaced it: the behaviors
           are real now, and the world's kinds move the ways their rows
           say (the slime's row says `none`, so it stands). */
        created += 1;
    }
    std::printf("engine: world: %d entities from the table's rows, live %d of %d\n",
                created, world.store.live, ENTITY_CAP);

    /* Lesson 091: the enemy roster is the waves' now — lesson 088's
       standing spawn gave way to the wave fight (GameWaves), which
       spawns the same rows wave by wave. A new row is still a new
       enemy: no per-kind code has appeared since. */

    /* Lesson 087: weapons are rows. The hero starts armed with the
       weapons table's first row; the number keys arm the rest
       (HeroFire). Lesson 090: the enemy-fire stand-in and its key are
       gone — the enemy rows carry their own weapons and the walk's
       attack fires them. */
    if (world.weapons.count > 0)
        CombatArm(*world.hero, world.weapons.rows[0]);

    /* Lesson 093: the burst kind — the particles table's first row. The
       game bursts what the table puts first, the way the hero arms with
       the weapons table's first row; a table with no particle kind is a
       named failure, never a burst of assumed attributes. */
    if (world.particles.count == 0) {
        std::fprintf(stderr,
                     "engine: assets/particles.txt: no particle kind\n");
        return false;
    }

    /* The lookup's typed failure, checked on purpose: a definition the
       table does not hold is a value — never an entity with assumed
       attributes. */
    DefResult unknown = TableFind(world.table, "dragon");
    std::printf("engine: table: \"dragon\" -> %s\n",
                unknown.error == DEF_OK ? "found" : "unknown");

    /* Lesson 095: the game's sound — its music and one effect per
       event, as files' bytes. Lesson 061's tone and lesson 066's
       demonstration effect leave the run here (both stay on disk: the
       files lessons 059-068 were built on); the game's own sounds are
       these four. Each load either yields the complete sample or names
       what went wrong, and a failure ends the run by name — like every
       asset above. */
    world.sound = {};
    if (!LoadRunSample(world.arena, "assets/music.wav", world.sound.music) ||
        !LoadRunSample(world.arena, "assets/shot.wav", world.sound.shot) ||
        !LoadRunSample(world.arena, "assets/hit.wav", world.sound.hit) ||
        !LoadRunSample(world.arena, "assets/death.wav", world.sound.death))
        return false;

    /* The byte-level check, before anything is played: each sound's
       facts, its peak, and its first frames — the same check lesson 066
       made on its two files, now on the game's four. */
    PrintSample("music", world.sound.music);
    PrintSample("shot", world.sound.shot);
    PrintSample("hit", world.sound.hit);
    PrintSample("death", world.sound.death);

    /* Lesson 068: the music loops on the music channel and the effects
       fire over it on the pool's channels, every one of them summed by
       the same MixBuffer into the one stream. Lesson 095: what fires
       them is the game now — the events of lesson 092's toolkit, each
       with its own sound — and the demonstration rhythm that used to
       fire them on a clock is gone. Nothing in the mix knows which
       sound is which. */
    SoundStart(world.sound);
    return true;
}

} /* namespace engine */
