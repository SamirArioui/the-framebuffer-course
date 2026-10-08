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

/* Lesson 062: the buffer of stream one feed hands the device, filled
   from the sample (or with silence) as the feed is due. Static, like the
   platform layer's own staging buffers — the language law of lesson 026
   keeps allocation out of the run. Lesson 097: it lives beside the feed
   that fills it. */
static short stream[CHUNK_FRAMES];

void SoundFeed(Sound &sound, Feed &feed, platform::AudioOutput *&output)
{
    /* The loop feeds the device the next buffer of the stream, and only
       when the buffer is due. Input news can wake a frame early; a frame
       woken early must not queue extra audio, or the run would bury the
       device in buffers instead of pacing them.

       Lesson 064: the stream is the mix. One buffer is every active
       channel's next frames summed and clamped — silence where no
       channel has anything to say. Lesson 066: frame_count is still
       the fact that says where a sample ends; a channel that loops
       wraps there instead of ending, and the mix does not know the
       difference. */
    if (platform::Now() < feed.next)
        return;

    /* Lesson 095: the mix is the game's work; the device is the
       seam's. The stream is mixed on the engine's rate whether or not
       a device exists — with no output this machine runs the whole mix
       in silence, and the reports still say what the channels and the
       stream carried. */
    int music_before = sound.mixer.channels[AUDIO_MUSIC_CHANNEL].cursor;

    MixBuffer(sound.mixer, stream, CHUNK_FRAMES);

    if (feed.feeds == 0) {
        /* The stream's own bytes — the first buffer, the music
           alone at this point. */
        std::printf("engine: mix: first frames (music alone):");
        for (int i = 0; i < 8 && i < CHUNK_FRAMES; ++i)
            std::printf(" %d", (int)stream[i]);
        std::printf("\n");
    }

    if (!feed.reported) {
        /* And the stream with the game's sounds in it: the first
           buffer any fired effect reaches — every frame the sum
           of the music's next frame and the effects'. */
        bool any = false;
        for (int c = AUDIO_MUSIC_CHANNEL + 1;
             c < AUDIO_MIXER_CHANNELS && !any; ++c)
            any = sound.mixer.channels[c].active;
        if (any) {
            std::printf("engine: mix: first frames with the effects in:");
            for (int i = 0; i < 8 && i < CHUNK_FRAMES; ++i)
                std::printf(" %d", (int)stream[i]);
            std::printf("\n");
            feed.reported = true;
        }
    }

    if (sound.mixer.channels[AUDIO_MUSIC_CHANNEL].active &&
        sound.mixer.channels[AUDIO_MUSIC_CHANNEL].cursor < music_before) {
        /* The wrap: the cursor went backwards — the loop's own
           arithmetic, visible from outside the mixer. */
        feed.wraps += 1;
        int cursor = sound.mixer.channels[AUDIO_MUSIC_CHANNEL].cursor;
        std::printf("engine: loop: music wrapped on channel %d — wrap %d, %ld frames played, cursor %d of %d\n",
                    AUDIO_MUSIC_CHANNEL, feed.wraps,
                    (long)feed.wraps * sound.music.frame_count + cursor,
                    cursor, sound.music.frame_count);
    }

    if (output && !platform::SubmitSamples(output, stream, CHUNK_FRAMES)) {
        /* A device that will not take the samples is named once,
           not once per frame: the run closes the output and carries
           on in silence — its wait unbounded again. */
        std::fprintf(stderr,
                     "engine: the output would not take the samples\n");
        platform::CloseAudioOutput(output);
        output = 0;
    }
    feed.feeds += 1;
    /* The schedule restarts from now, not from the missed slot: a
       long frame is caught up by one buffer, never by a backlog. */
    feed.next = platform::Now() +
                (double)CHUNK_FRAMES / (double)sound.music.rate;
}

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
