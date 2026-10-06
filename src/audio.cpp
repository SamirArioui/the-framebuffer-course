// audio.cpp — the tone computed by code, and the sample read from a file.
//
// Lesson 059: every sample in the engine is a sequence of numbers, and
// this file makes one out of a sine wave so the numbers can be read,
// checked, and played before any file format is involved.
//
// Lesson 061: the same bytes now come back out of a file. LoadSample
// walks a RIFF/WAVE container chunk by chunk — every size checked against
// the bytes around it, every format fact checked against the engine's —
// and copies the frames into the arena. The numbers are the point on both
// sides: what GenerateTone writes, the loader reads back.

#include "audio.h"

#include <cmath>

#include "platform.h"

namespace engine {
namespace {

/* One turn of the sine, in radians: two pi. */
constexpr double TURN = 6.283185307179586;

/* The format's most positive sample. The sine runs -1.0 to 1.0; scaling
   by amplitude * 32767 lands inside the format at any amplitude <= 1.0. */
constexpr double SAMPLE_PEAK = 32767.0;

/* The container's numbers are little-endian — the file's bytes are not
   the machine's bytes, and this is where the difference is resolved
   (lesson 014): low byte first, assembled by hand. */
unsigned ReadU32(const unsigned char *p)
{
    return (unsigned)p[0] | ((unsigned)p[1] << 8) | ((unsigned)p[2] << 16) |
           ((unsigned)p[3] << 24);
}

/* One sample frame: two little-endian bytes whose count carries its sign
   in bit 15 — 0x8000 and up are the format's negative frames. */
short ReadFrame(const unsigned char *p)
{
    int count = p[0] | (p[1] << 8);
    return (short)(count < 0x8000 ? count : count - 0x10000);
}

/* A four-byte chunk id, exactly — `fmt ` keeps its space. */
bool IdIs(const unsigned char *p, const char *id)
{
    return p[0] == (unsigned char)id[0] && p[1] == (unsigned char)id[1] &&
           p[2] == (unsigned char)id[2] && p[3] == (unsigned char)id[3];
}

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

SampleResult LoadSample(Arena &arena, const char *path)
{
    SampleResult result = { { 0, 0, 0, 0 }, SAMPLE_OK };

    platform::FileData file = platform::ReadFile(path);
    if (file.error != platform::FILE_OK) {
        result.error = SAMPLE_MISSING;
        return result;
    }

    const unsigned char *data = file.data;
    size_t size = file.size;

    /* The container's account of itself: RIFF names the form, claims a
       length, and says the form inside is WAVE. A claim is only a claim —
       this one is checked against the bytes the file actually has, and a
       file whose RIFF size disagrees with its own length is malformed
       before its chunks are even walked. */
    bool ok = size >= 12 && IdIs(data, "RIFF") && IdIs(data + 8, "WAVE");
    ok = ok && ReadU32(data + 4) == size - 8;

    /* The walk: chunk after chunk — an id, a size, then exactly that many
       bytes (padded to an even length). Chunks are never assumed to be
       where they "should" be: the walk finds `fmt ` and `data` wherever
       they are, steps over what it does not know, and refuses any chunk
       whose own size disagrees with the bytes around it. */
    size_t at = 12;
    bool have_format = false, have_frames = false;
    unsigned rate = 0, channels = 0, bits = 0;
    size_t frames_at = 0, frames_bytes = 0;

    while (ok && at + 8 <= size) {
        const unsigned char *id = data + at;
        size_t body = at + 8;
        size_t chunk = ReadU32(data + at + 4);
        ok = ok && chunk <= size - body;
        if (!ok)
            break;

        if (IdIs(id, "fmt ")) {
            /* The format chunk: at least the sixteen bytes of PCM facts.
               Every fact is checked typed — not mono, not 16-bit, not
               AUDIO_RATE is not the engine's format, and nothing here
               will resample or reinterpret a frame to make it fit. The
               chunk's own derived numbers are checked against the facts
               they are derived from: the claims must agree with each
               other as well as with the bytes. */
            ok = ok && !have_format && chunk >= 16;
            if (ok) {
                unsigned format =
                    (unsigned)data[body] | ((unsigned)data[body + 1] << 8);
                channels =
                    (unsigned)data[body + 2] | ((unsigned)data[body + 3] << 8);
                rate = ReadU32(data + body + 4);
                unsigned byte_rate = ReadU32(data + body + 8);
                unsigned block_align = (unsigned)data[body + 12] |
                                       ((unsigned)data[body + 13] << 8);
                bits = (unsigned)data[body + 14] |
                       ((unsigned)data[body + 15] << 8);
                bool claims_agree =
                    byte_rate == rate * channels * (bits / 8) &&
                    block_align == channels * (bits / 8);
                ok = ok && format == 1 /* linear PCM */ && channels == 1 &&
                     bits == 16 && rate == (unsigned)AUDIO_RATE &&
                     claims_agree;
            }
            have_format = true;
        } else if (IdIs(id, "data")) {
            ok = ok && !have_frames;
            frames_at = body;
            frames_bytes = chunk;
            have_frames = true;
        }
        at = body + chunk + (chunk & 1); /* chunks pad to an even length */
    }

    /* The walk's verdict: both halves found, and the frames a whole
       number of them — one frame is one short, and a data chunk whose
       size is not a whole number of frames claims something the format
       cannot hold. */
    ok = ok && have_format && have_frames;
    ok = ok && frames_bytes % 2 == 0;

    if (!ok) {
        result.error = SAMPLE_MALFORMED;
        platform::ReleaseFile(file);
        return result;
    }
    int frame_count = (int)(frames_bytes / 2);

    /* The frames, into the arena — the engine keeps its own copy, and the
       file's bytes go back to the OS. The mark is the load's transaction:
       from here on a refusal rolls the arena back, and a failed load
       leaves no partial frames behind. */
    size_t mark = ArenaMark(arena);
    short *frames = (short *)ArenaAlloc(arena, frames_bytes, sizeof(short));
    if (!frames) {
        result.error = SAMPLE_NO_ROOM;
        platform::ReleaseFile(file);
        return result;
    }
    for (int i = 0; i < frame_count; ++i)
        frames[i] = ReadFrame(data + frames_at + (size_t)i * 2);
    platform::ReleaseFile(file);

    /* The last agreement: the frames are the file's last bytes. The file
       holds exactly the sample — no short read presented as a sample, and
       nothing after the frames the loader would half-understand. */
    if (frames_at + frames_bytes != size) {
        ArenaRollback(arena, mark);
        result.error = SAMPLE_MALFORMED;
        return result;
    }

    result.sample.frames = frames;
    result.sample.frame_count = frame_count;
    result.sample.rate = (int)rate;
    result.sample.channels = (int)channels;
    result.error = SAMPLE_OK;
    return result;
}

} /* namespace engine */
