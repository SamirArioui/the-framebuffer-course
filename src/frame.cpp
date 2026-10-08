// frame.cpp — the frame account: sums, worst case, nothing else.
//
// Lesson 036: measured data is just data — this file does arithmetic on it.

#include "frame.h"

#include <cstdio>

namespace engine {

void AccountFrame(FrameStats &stats, const FrameRecord &frame)
{
    stats.frames += 1;
    stats.update_sum += frame.update;
    stats.audio_sum += frame.audio;
    stats.render_sum += frame.render;
    stats.present_sum += frame.present;
    stats.total_sum += frame.total;
    stats.sprites_sum += frame.sprites;
    stats.text_sum += frame.text;
    stats.tilemap_sum += frame.tilemap;
    stats.clear_sum += frame.clear;
    stats.entities_sum += frame.entities;
    if (frame.total > stats.worst) {
        stats.worst = frame.total;
        stats.worst_number = frame.number;
    }
}

void PrintFrameBudget(const FrameStats &stats)
{
    if (!stats.frames)
        return;
    double n = (double)stats.frames;
    double avg = stats.total_sum / n;

    /* The account the spec requires: how many frames, what they cost on
       average, and the worst one by number. */
    std::printf("engine: frame budget — %ld frames, avg %.3f ms, worst %.3f ms (frame %ld)\n",
                stats.frames, avg * 1e3, stats.worst * 1e3,
                stats.worst_number);

    /* The attribution: every row a measured sum, every share of the
       average frame. The phases take the record's own order — update,
       audio, render, present — and the named phases live inside render:
       they say where it went, they do not replace it. Lesson 070: the
       audio phase, measured since lesson 060, gets its row at last. */
    double update = stats.update_sum / n * 1e3;
    double audio = stats.audio_sum / n * 1e3;
    double render = stats.render_sum / n * 1e3;
    double clear = stats.clear_sum / n * 1e3;
    double sprites = stats.sprites_sum / n * 1e3;
    double text = stats.text_sum / n * 1e3;
    double tilemap = stats.tilemap_sum / n * 1e3;
    double entities = stats.entities_sum / n * 1e3;
    double present = stats.present_sum / n * 1e3;
    std::printf("engine:   subsystem   avg ms    share\n");
    std::printf("engine:   update      %6.3f      %2.0f%%\n", update,
                100.0 * update / (avg * 1e3));
    /* Lesson 081: the update's entity work, named inside the phase it
       lives in — measured from the frames that ran, like every row. */
    std::printf("engine:     entities  %6.3f      %2.0f%%\n", entities,
                100.0 * entities / (avg * 1e3));
    std::printf("engine:   audio       %6.3f      %2.0f%%\n", audio,
                100.0 * audio / (avg * 1e3));
    std::printf("engine:   render      %6.3f      %2.0f%%\n", render,
                100.0 * render / (avg * 1e3));
    /* Lesson 098: the measure pass's instrument — the clear, named at
       last. It was always inside render; until now it was the unnamed
       remainder between render's row and its named sub-phases' sum. */
    std::printf("engine:     clear     %6.3f      %2.0f%%\n", clear,
                100.0 * clear / (avg * 1e3));
    std::printf("engine:     sprites   %6.3f      %2.0f%%\n", sprites,
                100.0 * sprites / (avg * 1e3));
    std::printf("engine:     text      %6.3f      %2.0f%%\n", text,
                100.0 * text / (avg * 1e3));
    std::printf("engine:     tilemap   %6.3f      %2.0f%%\n", tilemap,
                100.0 * tilemap / (avg * 1e3));
    std::printf("engine:   present     %6.3f      %2.0f%%\n", present,
                100.0 * present / (avg * 1e3));
    std::printf("engine:   total       %6.3f     100%%\n", avg * 1e3);
}

} /* namespace engine */
