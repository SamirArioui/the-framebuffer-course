// sound.cpp — the game's events, played through the mixer's channels.
//
// Lesson 095: every function here is one line of routing — the event's
// sample, its volume, and the mixer's pool — plus the report that says
// which channel it landed on. The mixer decides everything else (the
// one-shot contract, the oldest-stealing pool, the music channel's
// immunity); this file only knows which sound belongs to which moment.

#include "sound.h"

#include <cstdio>

namespace engine {

void SoundStart(Sound &sound)
{
    /* The music is the run's music: the music channel, looping, at full
       volume — under the effects for the whole game. */
    MixerPlayMusic(sound.mixer, sound.music, AUDIO_VOLUME_FULL);
    std::printf("engine: sound: music -> channel %d (looping, volume %d of %d)\n",
                AUDIO_MUSIC_CHANNEL,
                sound.mixer.channels[AUDIO_MUSIC_CHANNEL].volume,
                AUDIO_VOLUME_FULL);
}

/* One event's sound: its sample on the pool's channels, at its volume,
   reported with the channel it landed on. The volumes are the sounds'
   own — a quiet blip for every shot, a heavier thud for a hit, the
   loudest for a death — so the mix inside the format stays honest while
   the game gets loud. */
static void Fire(Sound &sound, const Sample &sample, int volume,
                 const char *what)
{
    int ch = MixerPlayEffect(sound.mixer, sample, volume);
    sound.fired += 1;
    std::printf("engine: sound: %s -> channel %d (volume %d of %d)\n", what,
                ch, volume, AUDIO_VOLUME_FULL);
}

void SoundShot(Sound &sound)
{
    Fire(sound, sound.shot, AUDIO_VOLUME_FULL / 4, "shot");
}

void SoundHit(Sound &sound)
{
    Fire(sound, sound.hit, AUDIO_VOLUME_FULL / 2, "hit");
}

void SoundDeath(Sound &sound)
{
    Fire(sound, sound.death, AUDIO_VOLUME_FULL / 2, "death");
}

} /* namespace engine */
