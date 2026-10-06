// audio.cpp — the tone computed by code: arithmetic that becomes sound.
//
// Lesson 059: every sample in the engine is a sequence of numbers, and
// this file makes one out of a sine wave so the numbers can be read,
// checked, and played before any file format is involved. The same bytes
// come back later as an asset (lesson 061) and go into the mixer (lesson
// 063); here they are simply written.

#include "audio.h"

#include <cmath>

namespace engine {
namespace {

/* One turn of the sine, in radians: two pi. */
constexpr double TURN = 6.283185307179586;

/* The format's most positive sample. The sine runs -1.0 to 1.0; scaling
   by amplitude * 32767 lands inside the format at any amplitude <= 1.0. */
constexpr double SAMPLE_PEAK = 32767.0;

} /* namespace */

void GenerateTone(short *frames, int frame_count, double frequency,
                  double amplitude)
{
    for (int i = 0; i < frame_count; ++i) {
        /* Frame i is heard i / AUDIO_RATE seconds in: the rate turns a
           frame number into a time, and the sine turns a time into an
           amplitude. */
        double time = (double)i / (double)AUDIO_RATE;
        double wave = std::sin(TURN * frequency * time);
        frames[i] = (short)(wave * amplitude * SAMPLE_PEAK);
    }
}

} /* namespace engine */
