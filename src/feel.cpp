// feel.cpp — the juice toolkit: the feedback hooks, the bursts, and the
// easing that shapes them.
//
// Lesson 086: each hook fires, runs down its own wall-time, and returns
// exactly to rest. Lesson 092: the game's own events fire them — a hit
// lands, a death falls — in the event's own frame, and each hook says
// when it fires beside the event's own line. The weights are the
// event's, passed in from where the event happens.
//
// Lesson 093: the toolkit's other two effects live here too — particle
// bursts (cosmetic entities, bounded by the store's policy) and the
// small set of ease functions that make animated values arrive at their
// targets instead of stepping to them.

#include "feel.h"

#include <cstdio>

namespace engine {

void FeelInit(Feedback &feel)
{
    feel.shake = 0.0;
    feel.shake_mag = 0.0;
    feel.hitstop = 0.0;
    feel.hitstop_k = 0.0;
}

void FeelShake(Feedback &feel, double magnitude, double seconds)
{
    feel.shake = seconds;
    feel.shake_mag = magnitude;
    std::printf("engine: feel: shake fired (%.0f px, %.2fs)\n", magnitude,
                seconds);
}

void FeelHitstop(Feedback &feel, double fraction, double seconds)
{
    feel.hitstop = seconds;
    feel.hitstop_k = fraction;
    std::printf("engine: feel: hitstop fired (%.2fx, %.2fs)\n", fraction,
                seconds);
}

double FeelTimeScale(const Feedback &feel)
{
    /* At rest the factor is full speed; during a hitstop it is the
       fraction the hitstop was fired at. */
    return feel.hitstop > 0.0 ? feel.hitstop_k : GAMETIME_FULL;
}

/* Lesson 093: easing. Each shape takes t in [0, 1] and answers the
   fraction travelled — exactly 0 at 0, exactly 1 at 1. The clamps at
   both ends are the "arrives exactly" contract: past its target the
   value sits at its target, and an eased value never overshoots. */
double EaseInQuad(double t)
{
    if (t <= 0.0)
        return 0.0;
    if (t >= 1.0)
        return 1.0;
    return t * t;
}

double EaseOutQuad(double t)
{
    if (t <= 0.0)
        return 0.0;
    if (t >= 1.0)
        return 1.0;
    double u = 1.0 - t;
    return 1.0 - u * u;
}

double EaseInOutQuad(double t)
{
    if (t <= 0.0)
        return 0.0;
    if (t >= 1.0)
        return 1.0;
    return t < 0.5 ? 2.0 * t * t : 1.0 - 2.0 * (1.0 - t) * (1.0 - t);
}

/* Lesson 093: the burst's eight lanes — the world's compass points, a
   diagonal at 1/sqrt(2), the same lanes the aim and the movement use.
   A burst is reproducible: lane i of a count is the same direction on
   every machine and every run. */
constexpr double BURST_DIAG = 0.70710678;
const double LANE_X[8] = { 1.0, BURST_DIAG, 0.0, -BURST_DIAG, -1.0,
                           -BURST_DIAG, 0.0, BURST_DIAG };
const double LANE_Y[8] = { 0.0, BURST_DIAG, 1.0, BURST_DIAG, 0.0,
                           -BURST_DIAG, -1.0, -BURST_DIAG };

int FeelBurst(EntityStore &store, const EntityDef &kind, double x, double y,
              int count)
{
    /* The cosmetic share, counted before the burst asks: the toolkit's
       particles may hold FEEL_COSMETIC_SLOTS of the store's slots and
       no more. */
    int cosmetic = 0;
    for (int i = 0; i < ENTITY_CAP; ++i)
        if (store.slots[i].live && store.slots[i].behavior == BEHAVIOR_SETTLE)
            cosmetic += 1;

    int made = 0, dropped = 0;
    for (int i = 0; i < count; ++i) {
        /* A burst that finds no slot drops its particle — cosmetic work
           may be dropped (counted, and invisible); gameplay work may
           not (lesson 074's contrast). Nothing is ever stolen. */
        if (cosmetic >= FEEL_COSMETIC_SLOTS) {
            dropped += 1;
            continue;
        }
        EntityResult result = EntityCreate(store, kind);
        if (result.error != ENTITY_OK) {
            dropped += 1;
            continue;
        }
        Entity &e = *result.entity;
        int lane = count < 8 ? (i * 8) / count : i % 8;
        e.x = x;
        e.y = y;
        e.from_x = x;
        e.from_y = y;
        e.move_x = LANE_X[lane];
        e.move_y = LANE_Y[lane];
        e.traveled = 0.0;
        e.life_t = 0.0;
        cosmetic += 1;
        made += 1;
    }
    std::printf("engine: burst: %s x%d at %d,%d — %d made, %d dropped\n",
                kind.name, count, (int)x, (int)y, made, dropped);
    return made;
}

void FeelParticle(EntityStore &store, Entity &e, double dt)
{
    /* The settle's clock runs on game time: a pause freezes a spark
       mid-air, a hitstop slows it — the world's clock, like everything
       the simulation does. */
    e.life_t += dt;
    double t = e.accel > 0 ? e.life_t / (e.accel / 1000.0) : 1.0;
    if (t > 1.0)
        t = 1.0;

    /* The eased value is the distance out: it follows the ease's curve
       frame by frame and arrives exactly at the row's range. The
       position is that distance along the spark's lane — measured from
       the burst point every frame, never accumulated step by step, so
       the arrival is the target and not a rounding of it. */
    e.traveled = (double)e.range * EaseOutQuad(t);
    e.x = e.from_x + e.move_x * e.traveled;
    e.y = e.from_y + e.move_y * e.traveled;

    if (t >= 1.0) {
        /* The life ends where it settles — and the arrival is exact:
           the eased value is the target, not a neighbour of it. */
        bool exact = e.traveled == (double)e.range;
        std::printf("engine: %s settled at %d,%d — %g px out, its row's range %d (%s)\n",
                    e.name, (int)e.x, (int)e.y, e.traveled, e.range,
                    exact ? "exact" : "drifted");
        EntityRetire(store, e);
    }
}

void FeelUpdate(Feedback &feel, double wall_dt, Camera &camera)
{
    /* The hitstop runs on its own wall-time and returns to full speed
       when its deadline passes — the game need not remember to undo it. */
    if (feel.hitstop > 0.0) {
        feel.hitstop -= wall_dt;
        if (feel.hitstop < 0.0) {
            feel.hitstop = 0.0;
            std::printf("engine: feel: hitstop rested — full speed again\n");
        }
    }

    /* The screenshake drives the camera's additive offset — the juice
       hook lesson 054 defined. While it lasts the offset alternates;
       when it ends the offset rests at exactly zero. */
    if (feel.shake > 0.0) {
        feel.shake -= wall_dt;
        if (feel.shake <= 0.0) {
            feel.shake = 0.0;
            camera.add_x = 0;
            camera.add_y = 0;
            std::printf("engine: feel: shake rested at %d,%d\n", camera.add_x,
                        camera.add_y);
        } else {
            camera.add_x = ((int)(feel.shake * 40.0) & 1) ? (int)feel.shake_mag
                                                          : -(int)feel.shake_mag;
            camera.add_y = 0;
        }
    }
}

} /* namespace engine */
