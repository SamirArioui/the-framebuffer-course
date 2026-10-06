// frame.cpp — the frame account: sums, worst case, nothing else.
//
// Lesson 036: measured data is just data — this file does arithmetic on it.

#include "frame.h"

namespace engine {

void AccountFrame(FrameStats &stats, const FrameRecord &frame)
{
    stats.frames += 1;
    stats.update_sum += frame.update;
    stats.render_sum += frame.render;
    stats.present_sum += frame.present;
    stats.total_sum += frame.total;
    stats.sprites_sum += frame.sprites;
    if (frame.total > stats.worst) {
        stats.worst = frame.total;
        stats.worst_number = frame.number;
    }
}

} /* namespace engine */
