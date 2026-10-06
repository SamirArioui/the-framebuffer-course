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
    double render; /* drawing the scene into the framebuffer */
    double present;/* the copy to the window, sync included */
    double total;  /* the whole frame step */

    /* Lesson 046: the render phase starts naming what is inside it — one
       field per subsystem, the attribution the frame-budget table
       (lesson 058) grows from. The named times are inside render, never
       instead of it: render stays the phase, these say where it went. */
    double sprites; /* sprite draws through the blit */
    double text;    /* lesson 051: text drawing — glyphs through the blit */
    double tilemap; /* lesson 053: the map's walk — tiles through the blit */
};

/* The running account: every frame measured so far. */
struct FrameStats {
    long frames;
    double update_sum;
    double render_sum;
    double present_sum;
    double total_sum;
    double sprites_sum; /* lesson 046's named sub-phase, summed like the rest */
    double text_sum;
    double tilemap_sum;
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
