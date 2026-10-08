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

namespace engine {

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

} /* namespace engine */

#endif
