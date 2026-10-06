// audio.h — sound as samples: frames of amplitude, the engine's format.
//
// Lesson 059: sound is data before it is sound. A sample is a frame of
// amplitude — one number saying where the speaker sits at that instant —
// and a sound is a run of those frames at a fixed rate. Nothing here knows
// about devices or mixing: this is the format the engine's output speaks
// and the format the mixer of lessons 063-065 sums.
//
// Lesson 061: the format is now loadable. A sample is authored as a file
// — the same bytes GenerateTone computes, in a RIFF/WAVE container — and
// the loader below either hands over the complete sample or names what
// went wrong. From there on the sample carries its own playback facts, so
// lesson 062 plays it from the sample alone.
#ifndef AUDIO_H
#define AUDIO_H

#include "arena.h"

namespace engine {

/* The engine's sample format: 16-bit signed frames at this rate. One
   sample frame is one `short`, from -32768 to 32767. The rate is the
   format's other half: AUDIO_RATE frames make one second of sound, so a
   frame's number in the run says exactly when it is heard. */
constexpr int AUDIO_RATE = 44100; /* sample frames per second */

/* The engine's mix is one channel wide. What the device itself wants is
   the platform layer's business — it maps these samples into the device's
   own layout, exactly as Present maps the framebuffer's pixels into the
   window's. */
constexpr int AUDIO_OUTPUT_CHANNELS = 1;

/* A tone computed by code: `frame_count` sample frames of a sine wave at
   `frequency` hertz, at `amplitude` (0.0 to 1.0), in the engine's format.
   This is what a sample looks like before any file holds one — the bytes
   a sound is made of, produced by arithmetic instead of read from disk.
   It is a worked example of what a sample *is*, not a synthesis feature:
   the engine plays samples, it does not design sounds. */
void GenerateTone(short *frames, int frame_count, double frequency,
                  double amplitude);

/* A loaded sample: the frames, and the facts playback needs carried with
   them — its length in sample frames and its format. */
struct Sample {
    short *frames;
    int frame_count;
    int rate;
    int channels;
};

enum SampleError {
    SAMPLE_OK = 0,
    SAMPLE_MISSING,   /* the file is not there or cannot be read */
    SAMPLE_MALFORMED, /* the bytes are not a complete sample in the engine's format */
    SAMPLE_NO_ROOM,   /* the arena had no room for the frames */
};

struct SampleResult {
    Sample sample;
    SampleError error; /* SAMPLE_OK exactly when sample.frames is non-0 */
};

/* Loads a sample from a RIFF/WAVE file. The container is walked chunk by
   chunk and byte by byte — no library reads it — and anything that is not
   a complete sample in the engine's format is refused typed. The frames
   are copied into the arena and the file's own bytes go back to the OS:
   what the engine keeps is its copy, and a refused load keeps nothing. */
SampleResult LoadSample(Arena &arena, const char *path);

} /* namespace engine */

#endif
