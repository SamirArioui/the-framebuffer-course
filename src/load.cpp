// load.cpp — the run's asset wiring: the loads, and the checks.
//
// Lesson 097: moved whole from main.cpp — the failure paths and the
// byte-level prints are the same code this run has always run, in the
// same order. A move, nothing more.

#include "load.h"

#include <cstdio>

#include "sprite.h"

namespace engine {

void PrintSample(const char *name, const Sample &sample)
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

bool LoadRunSample(Arena &arena, const char *path, Sample &into)
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

bool LoadRunTable(Arena &arena, const char *path, EntityTable &into)
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

bool LoadRunArt(Arena &arena, EntityTable &table)
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

void PrintDefs(const char *path, const EntityTable &table)
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

} /* namespace engine */
