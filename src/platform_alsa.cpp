// platform_alsa.cpp — the ALSA implementation of the audio output.
//
// Lesson 059: the sound device is OS business, like the window is. This
// file and platform_x11.cpp are the two a second OS replaces (the boundary
// check's implementation list names them), and this is the only place an
// ALSA header or an ALSA call appears. The engine sees platform.h and
// nothing else.
//
// Which device this run opens is this file's business too: a machine with
// no usable output reports AUDIO_NO_DEVICE, and a machine that has none
// still runs.
//
// Lesson 060: the deadline bookkeeping. A device consumes samples at its
// own rate, so this file also knows when the device will have consumed
// what it was given — the bound the run's paced wait obeys. That number is
// platform state, shared with the event pump through platform_internal.h
// and not part of the seam's contract.
#define _POSIX_C_SOURCE 200809L

#include "platform.h"
#include "platform_internal.h"

#include <alsa/asoundlib.h>

#include <stdlib.h> /* getenv */

namespace platform {

/* What an audio output is made of on this OS. The definition lives here,
   where ALSA is visible; the engine holds the pointer and never looks
   inside. */
struct AudioOutput {
    snd_pcm_t *device;
    int engine_channels; /* the seam's frame width, and the device's own */
    int device_channels; /* layout, which this file maps between */
    int rate;            /* the sample rate it was opened at */
    double deadline;     /* lesson 060: when the device will have consumed
                            everything submitted so far — the boundary
                            AudioWaitSeconds reports */
};

/* The one output, in static storage: no new, no delete — the language law
   of lesson 026 keeps allocation out of the engine and this layer alike. */
static AudioOutput audio_state;

/* The device to open. ALSA's `default` is what a machine with sound
   answers with; the software device that accepts and discards samples
   (the headless check's `null`) is named here instead. The engine never
   sees the name — choosing a device is OS business. */
static const char *DeviceName(void)
{
    const char *named = getenv("ALSA_DEVICE");
    return named && named[0] ? named : "default";
}

/* The device's own shape. The engine's samples are one channel wide; this
   machine's outputs take interleaved frames across two, so the samples are
   duplicated here — the mix across both channels, which is the device's
   business and not the engine's. */
constexpr int DEVICE_CHANNELS = 2;

/* The conversion buffer, in chunks: the engine's mono frames become the
   device's interleaved frames. Static, like the rest of this file. */
constexpr int STAGE_FRAMES = 1024;
static short stage[STAGE_FRAMES * DEVICE_CHANNELS];

AudioResult OpenAudioOutput(int rate, int channels)
{
    AudioResult result = { 0, AUDIO_NO_DEVICE };

    snd_pcm_t *device = 0;
    if (snd_pcm_open(&device, DeviceName(), SND_PCM_STREAM_PLAYBACK, 0) < 0)
        return result; /* no usable output: named, not hidden */

    /* The engine's format, field by field — every one is a promise the
       engine's samples rely on. A device that cannot keep the rate is not
       the engine's output: there is no resampling here, so the open fails
       typed rather than quietly playing at the wrong speed. */
    snd_pcm_hw_params_t *hw = 0;
    snd_pcm_hw_params_malloc(&hw);
    snd_pcm_hw_params_any(device, hw);
    unsigned device_rate = (unsigned)rate;
    bool formatted =
        snd_pcm_hw_params_set_access(device, hw,
                                     SND_PCM_ACCESS_RW_INTERLEAVED) >= 0 &&
        snd_pcm_hw_params_set_format(device, hw, SND_PCM_FORMAT_S16_LE) >= 0 &&
        snd_pcm_hw_params_set_channels(device, hw,
                                       (unsigned)DEVICE_CHANNELS) >= 0 &&
        snd_pcm_hw_params_set_rate_near(device, hw, &device_rate, 0) >= 0 &&
        (unsigned)rate == device_rate &&
        snd_pcm_hw_params(device, hw) >= 0;
    snd_pcm_hw_params_free(hw);

    if (!formatted) {
        snd_pcm_close(device);
        return result;
    }

    audio_state.engine_channels = channels;
    audio_state.device = device;
    audio_state.device_channels = DEVICE_CHANNELS;
    audio_state.rate = rate;
    /* Nothing is queued yet, so the first buffer is due at once. */
    audio_state.deadline = 0.0;
    result.output = &audio_state;
    result.error = AUDIO_OK;
    return result;
}

bool SubmitSamples(AudioOutput *output, const short *samples, int frames)
{
    if (!output || !output->device)
        return false;

    const short *at = samples;
    int left = frames;
    while (left > 0) {
        int chunk = left < STAGE_FRAMES ? left : STAGE_FRAMES;

        /* The engine's frames become the device's. The device has its own
           channel count and the engine has `engine_channels` values per
           frame; each device channel takes one of the engine's, wrapping
           back to the first. With the engine's one channel that is one
           sample in every channel of its frame — the mix duplicated
           across the device's channels. */
        for (int f = 0; f < chunk; ++f) {
            for (int c = 0; c < output->device_channels; ++c) {
                int from = f * output->engine_channels +
                           c % output->engine_channels;
                stage[f * output->device_channels + c] = at[from];
            }
        }

        /* The device takes what it has room for; the rest comes back
           around. When it takes nothing at all, the caller hears false. */
        snd_pcm_sframes_t took = snd_pcm_writei(
            output->device, stage, (snd_pcm_uframes_t)chunk);
        if (took < 0)
            return false;
        at += (int)took * output->engine_channels;
        left -= (int)took;
    }

    /* Lesson 060: when will the device have consumed what this call gave
       it? On the platform clock, the answer is the device's own deadline —
       and it accumulates: a submit made while an earlier one is still
       playing lands after that backlog, never before it, so overlapping
       submits can only move the deadline later. `deadline > now` keeps the
       backlog's end when there is one; `now` is where consumption starts
       when the device has already caught up (a call that waited for room
       finds its deadline in the past). */
    double now = Now();
    output->deadline = (output->deadline > now ? output->deadline : now) +
                       (double)frames / (double)output->rate;
    return true;
}

double AudioWaitSeconds(void)
{
    if (!audio_state.device)
        return -1.0; /* no output to feed: the wait is unbounded, as before
                        sound — and a negative answer means exactly that */

    /* The time left until the device needs what comes next — zero once the
       buffer is already due. Zero, and never a negative: zero bounds the
       wait at once, a negative removes the bound, and confusing the two
       would make a due buffer sleep instead of feed. */
    double left = audio_state.deadline - Now();
    return left > 0.0 ? left : 0.0;
}

void CloseAudioOutput(AudioOutput *output)
{
    if (!output || !output->device)
        return;

    /* Drain first: the device finishes what it already has before the
       handle goes away — the samples handed over are the samples played. */
    snd_pcm_drain(output->device);
    snd_pcm_close(output->device);
    output->device = 0;
    output->deadline = 0.0; /* a closed output stops bounding the wait */
}

} /* namespace platform */
