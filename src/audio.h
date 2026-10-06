// audio.h — sound as samples: frames of amplitude, the engine's format.
//
// Lesson 059: sound is data before it is sound. A sample is a frame of
// amplitude — one number saying where the speaker sits at that instant —
// and a sound is a run of those frames at a fixed rate. Nothing here knows
// about devices, files, or mixing: this is the format the engine's output
// speaks, the format lesson 061's loader accepts and refuses everything
// else, and the format the mixer of lessons 063-065 sums.
#ifndef AUDIO_H
#define AUDIO_H

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

} /* namespace engine */

#endif
