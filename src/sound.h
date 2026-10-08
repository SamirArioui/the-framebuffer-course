// sound.h — the game's sound: its music and its effects, through the mixer.
//
// Lesson 095: the mixer is the engine's (lessons 063-066) and this is
// the game's use of it — which sound plays at which of the game's
// events. The events fire these in their own frame, like the feel
// toolkit fires its effects: the shot's blip with the shot, the hit's
// thud with the hit, the death's boom with the death. The channels and
// the stream are the mixer's own work, and the game never touches a
// device: with no output the run mixes in silence and the reports still
// say what the channels and the stream carried.
#ifndef SOUND_H
#define SOUND_H

#include "audio.h"
#include "platform.h"

namespace engine {

/* Lesson 060: one buffer of stream per feed — one sixtieth of a second,
   the horizon the run keeps queued. Lesson 062: a feed is always
   exactly this much stream — the sample's frames where the sample has
   them, silence beyond its end — so the horizon arithmetic is untouched
   whatever the sample's length is. The sample's own length is the file's
   fact: playback stops where its frame_count says it stops, not where a
   constant here would. Lesson 097: the constant is the sound's own now —
   the stream's buffers are this module's business. */
constexpr int CHUNK_FRAMES = AUDIO_RATE / 60;   /* 735 */

/* The game's sounds: its music — looping on the music channel, under
   everything — and its effects, one per event, fired on the pool's
   channels. Loaded at startup like every asset; nothing is created
   while the game runs. */
struct Sound {
    Mixer mixer;
    Sample music;
    Sample shot;
    Sample hit;
    Sample death;
    int fired; /* how many effects the game's events have fired */
};

/* The soundtrack starts when the run does: the music on the music
   channel, looping, at full volume — the channel no effect ever takes
   or steals. */
void SoundStart(Sound &sound);

/* The game's events — a shot left a barrel, a hit landed, a thing fell
   — each in its own frame. Every one fires its effect on the pool over
   the music; the mixer's oldest-stealing pool decides what happens when
   the game gets loud. */
void SoundShot(Sound &sound);
void SoundHit(Sound &sound);
void SoundDeath(Sound &sound);

/* Lesson 097: the run's feed — the schedule the loop used to keep in
   its own locals (when the next buffer is due, how many have been
   mixed, how often the music has wrapped, whether the stream's bytes
   with effects in have been reported). The feed is the sound's book-
   keeping now; the loop keeps the clock that paces it and nothing more. */
struct Feed {
    double next;   /* when the next buffer of stream is due */
    int feeds;     /* buffers of stream mixed */
    int wraps;     /* the music's wraps */
    bool reported; /* the stream's bytes with effects in, reported */
};

/* One buffer of stream, when it is due — the loop's audio step. The
   mix runs on the engine's rate whether or not a device exists (lesson
   095): with no output the run mixes in silence and the reports still
   say what the channels and the stream carried. The seam takes the
   buffer — or, refusing it once, is closed and the run carries on in
   silence. `output` is the seam's handle the run opened; the feed sets
   it to null when the device is gone. */
void SoundFeed(Sound &sound, Feed &feed, platform::AudioOutput *&output);

} /* namespace engine */

#endif
