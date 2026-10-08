// report.cpp — the run's reports: the probes, and the closing account.
//
// Lesson 097: moved whole from main.cpp. The printfs are the same, in
// the same order; only their home and their bookkeeping's names are new.

#include "report.h"

#include <cmath>
#include <cstdio>

#include "platform.h"

namespace engine {

void ReportFrame(RunReport &report, const EntityStore &store,
                 const Entity &hero, double was_x, double was_y,
                 double started)
{
    /* Lesson 089: the world's motion, as the behaviors produce it —
       every non-hero entity reported as it travels about a tile, its
       distance to the hero beside it (the number all three behaviors
       are about: chase shrinks it, flee grows it, keep holds it). */
    for (int i = 0; i < ENTITY_CAP; ++i) {
        const Entity &e = store.slots[i];
        if (!e.live || &e == &hero)
            continue;
        double dx = e.x - report.seen_x[i], dy = e.y - report.seen_y[i];
        if (dx * dx + dy * dy < 24.0 * 24.0)
            continue;
        report.seen_x[i] = e.x;
        report.seen_y[i] = e.y;
        double to_x = hero.x - e.x, to_y = hero.y - e.y;
        std::printf("engine: %s at %d,%d — %d px of the hero (t=%.3f)\n",
                    e.name, (int)e.x, (int)e.y,
                    (int)std::sqrt(to_x * to_x + to_y * to_y),
                    platform::Now() - started);
    }

    /* Lesson 077: the mover's state report, on transitions — the
       hero moving, or pushed against something that will not move. */
    bool blocked = (hero.move_x != 0.0 || hero.move_y != 0.0) &&
                   hero.x == was_x && hero.y == was_y;
    if (blocked != report.blocked) {
        std::printf("engine: hero %s at %d,%d (t=%.3f)\n",
                    blocked ? "blocked" : "unblocked", (int)hero.x,
                    (int)hero.y, platform::Now() - started);
        report.blocked = blocked;
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
        if (vx != report.vx || vy != report.vy) {
            std::printf("engine: hero velocity %d,%d (t=%.3f)\n", vx, vy,
                        platform::Now() - started);
            report.vx = vx;
            report.vy = vy;
        }
    }

    /* Lesson 086: the walk cycle — the frame advances while the hero
       steps, and this is the measurement of it advancing. */
    if (hero.frame != report.walk_frame) {
        std::printf("engine: hero frame %d (t=%.3f)\n", hero.frame,
                    platform::Now() - started);
        report.walk_frame = hero.frame;
    }
}

void ReportEnd(const Sound &sound, const Feed &feed,
               const EntityStore &store, const Entity &hero,
               long walk_visits, long frames)
{
    /* The demo's account: what the run did — the world's frames and the
       sound's buffers, together — before the cost's table below. */
    std::printf("engine: sound: %ld frames measured, %d buffers of stream mixed (%d frames), %d effects fired, %d music wraps\n",
                frames, feed.feeds, feed.feeds * CHUNK_FRAMES, sound.fired,
                feed.wraps);

    /* Lesson 089: where the behaviors left the world — every live
       entity's position and its distance to the hero, the number all
       three behaviors are about (chase shrinks it, flee grows it, keep
       holds it). The report above samples a moving world; this one
       states where it ended. */
    for (int i = 0; i < ENTITY_CAP; ++i) {
        const Entity &e = store.slots[i];
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
                walk_visits, frames);
}

} /* namespace engine */
