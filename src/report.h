// report.h — the run's reports: the probes that make a run measurable,
// and the account that closes it.
//
// Lesson 097: the probes were born inline in the loop, one per lesson
// that taught a measurement (077's mover state, 085's eased velocity,
// 086's walk cycle, 089's traveling world), each with its own private
// `was_*` bookkeeping in the loop's body. They are the run's eyes — a
// headless run can only verify what it prints — so they live here now,
// with one named state (`RunReport`) instead of anonymous locals, and
// the loop reads as the frame's phases again.
//
// Every report line is the same line it has always been: the refactor
// moves the printfs, it does not rewrite them. The transcript a
// checklist's demonstration produces is unchanged.
#ifndef REPORT_H
#define REPORT_H

#include "entity.h"
#include "sound.h"

namespace engine {

/* What the probes remember between frames: where each entity was last
   reported (the world's motion reports only on travel), and what the
   hero's last report said (the mover's state, the eased velocity, the
   walk cycle's frame — each prints on change, and the change is only
   visible against the previous frame). */
struct RunReport {
    bool blocked;        /* lesson 077: the mover's state, last reported */
    int vx, vy;          /* lesson 085: the hero's velocity, last reported */
    int walk_frame;      /* lesson 086: the walk cycle's frame, last reported */
    double seen_x[ENTITY_CAP], seen_y[ENTITY_CAP]; /* lesson 089: where
                       each entity was last reported */
};

/* The frame's probes, in the order the run has always printed them: the
   world's motion (every entity that traveled about a tile, its distance
   to the hero beside it), then the hero's own reports — the mover's
   state on transition, the position, the eased velocity, the walk
   cycle's frame. `was_x`/`was_y` are the hero's position at the frame's
   start — the probes read the walk's result against it. */
void ReportFrame(RunReport &report, const EntityStore &store,
                 const Entity &hero, double was_x, double was_y,
                 double started);

/* The account that closes the run: what the sound carried (frames
   measured, buffers of stream, effects fired, wraps), where the world's
   live entities ended beside the hero, and the walk's visit count — one
   per live entity per frame. The frame-budget table follows this in the
   loop's own close; it is the frame account's, not the report's. */
void ReportEnd(const Sound &sound, const Feed &feed,
               const EntityStore &store, const Entity &hero,
               long walk_visits, long frames);

} /* namespace engine */

#endif
