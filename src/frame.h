// frame.h — the per-frame record: what a frame cost, measured on the
// platform clock.
//
// Lesson 036: frame time as measured data. The engine does not guess what
// its frames cost — it measures each phase and keeps the numbers. Part 2
// grows this record with its own phases; Part 5's frame-budget report
// reads it. The format of one line of the log is the format of one record.
#ifndef FRAME_H
#define FRAME_H

namespace engine {

/* Seconds, each field: how long one frame's phase took. */
struct FrameRecord {
    long number;   /* the frame's count since the run started */
    double update; /* reading state, moving the world */
    double audio;  /* lesson 060: the run's audio step — mixing and
                      submitting the stream */
    double render; /* drawing the scene into the framebuffer */
    double present;/* the copy to the window, sync included */
    double total;  /* the whole frame step */

    /* Lesson 046: the render phase starts naming what is inside it — one
       field per subsystem, the attribution the frame-budget table
       (lesson 058) grows from. The named times are inside render, never
       instead of it: render stays the phase, these say where it went.
       Lesson 081: update grows the same kind of name — the entity work
       the walk does, attributed inside the phase it lives in.
       Lesson 098: the measure pass names the one piece of the frame no
       row carried — the clear, the render's first work, until now the
       unnamed remainder of the render row. */
    double clear;    /* the framebuffer's clear — one color, every pixel */
    double sprites; /* sprite draws through the blit */
    double text;    /* lesson 051: text drawing — glyphs through the blit */
    double tilemap; /* lesson 053: the map's walk — tiles through the blit */
    double entities; /* lesson 081: the walk's per-entity step — the
                        store's entities, moved through the mover */

    /* Lesson 079: the game-time step this frame advanced the simulation
       by — not a duration. Every field above is wall-clock, at any
       scale: the measurement is the machine's, not the game's. This one
       is where game time is visible, so a paused frame reads `step
       0.000 ms` beside wall-clock phases that took what they took. */
    double step;
};

/* The running account: every frame measured so far. */
struct FrameStats {
    long frames;
    double update_sum;
    double audio_sum; /* lesson 060's phase, summed like the rest */
    double render_sum;
    double present_sum;
    double total_sum;
    double sprites_sum; /* lesson 046's named sub-phase, summed like the rest */
    double text_sum;
    double tilemap_sum;
    double clear_sum; /* lesson 098: the clear's row, summed like the rest */
    double entities_sum; /* lesson 081: the update's entity work, summed */
    double worst;      /* the longest frame so far */
    long worst_number; /* and which one it was */
};

void AccountFrame(FrameStats &stats, const FrameRecord &frame);

/* Lesson 058: the frame-budget table — the account, attributed per
   subsystem, as the report Part 5's finale grows. Every number in it is
   a measured sum from the frames that actually ran; the shares are of
   the average frame. */
void PrintFrameBudget(const FrameStats &stats);

} /* namespace engine */

#endif
