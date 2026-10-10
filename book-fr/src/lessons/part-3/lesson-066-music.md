# Leçon 066 — la musique comme une boucle

{{#include ../../stability-horizon.md}}

## Prose

La leçon 065 a réservé un canal et l'a laissé vide. Cette leçon dépense la
réservation : la musique du jeu passe sur `AUDIO_MUSIC_CHANNEL`, et le fait le
plus important de la musique, c'est qu'elle ne se termine pas. Un effet sonore
est une rafale qui joue jusqu'à sa fin puis disparaît ; la musique est le son
qui se répète — les mêmes mesures, encore et encore, aussi longtemps que
l'exécution dure. Le travail du mixeur est, pour les deux, la même somme. Ce qui
diffère, c'est un bit d'état par canal, et toute la leçon tient à ce que ce bit
fait à un curseur.

### La boucle est l'arithmétique du curseur

`Channel` gagne un champ : `bool loop`. Tout le reste de la lecture —
échantillon, curseur, volume, actif — est de la leçon 063, intact. Le
retournement (wrap) vit dans le tirage par trame, `ChannelFrame` dans
`src/audio.cpp`, et c'est une ligne d'arithmétique : quand le curseur a atteint
la fin de l'échantillon et que le canal boucle, le curseur revient à la
première trame de l'échantillon et le tirage prend cette trame.
L'échantillon rejoue depuis son début.

Dites-le sans détour, car c'est le point : **une boucle n'exige aucun mixeur
spécial**. Il n'y a pas de mode boucle dans `MixBuffer`, pas de second tampon,
pas de copie de l'échantillon. Le retournement est l'arithmétique du curseur
lui-même à l'intérieur du tirage — une branche qui dit d'où vient le
prochain numéro de trame — et le mixage au-dessus additionne la prochaine trame
du canal exactement comme il l'a toujours fait. Un canal qui boucle et un canal
one-shot diffèrent comme diffèrent deux séquences de curseur distinctes : par
là où vont les nombres.

C'est la même branche qui préserve à l'identique le comportement de la leçon
065. Quand `loop` n'est pas positionné, le tirage retombe sur la fin qu'il
a toujours eue : le canal devient inactif à `frame_count`, le fait qui dit où
un échantillon se termine. Un one-shot n'est pas un cas particulier d'une
boucle ; une boucle est un canal qui décline la fin. Et un échantillon sans
aucune trame n'a rien vers quoi se retourner — l'autre garde de la branche —
donc il se termine ici comme n'importe quel autre au lieu de se retourner sans
fin.

### Ce qui rend une boucle sans couture

Une boucle s'entend encore et encore, donc sa couture s'entend encore et
encore : un écart à la jointure est un clic à chaque répétition. La fluidité
n'est pas au mixeur de la donner — c'est une **propriété de l'asset lui-même**,
et `assets/music.wav` est construit pour ça. Sa mélodie est douze notes d'un
quart de seconde par-dessus trois notes de basse d'une seconde, et la fréquence
de chaque note accomplit des cycles entiers sur son propre créneau et s'éteint
à zéro sur sa propre dernière trame. Chaque frontière de note est donc du
silence — et celle de la boucle ne fait pas exception : la dernière trame du
fichier rejoint sa première comme zéro rejoint zéro. La couture n'est pas un
endroit particulier de la forme d'onde ; c'est un endroit de plus où une note
s'est achevée.

La vérification au niveau des octets que fait l'exécution sur les deux sons,
imprimée avant que quoi que ce soit ne joue :

```
engine: music: 132300 frames at 44100 Hz, 1 channel, peak 10442, first frames: 0 277 554 831 1107 1381 1655 1927, last frame 0
engine: effect: 8820 frames at 44100 Hz, 1 channel, peak 9770, first frames: 0 1229 2438 3607 4719 5757 6703 7544, last frame 0
```

Les deux fichiers sont au format que la leçon 061 a défini et que le chargeur
de la leçon 062 vérifie : mono, 16 bits signés, 44100 Hz, un seul chunk `data`
de trames entières. La musique fait 132300 trames — trois secondes, exactement
180 tampons des 735 de l'exécution. L'effet en fait 8820 — un cinquième de
seconde, exactement 12 tampons. Voilà les deux sons d'un jeu : l'un long et qui
se répète, l'autre court et joué une seule fois.

Les crêtes sont dans la même ligne, et elles sont le budget de mixage. La trame
la plus forte de la musique est 10442 et celle de l'effet est 9770 — toutes
deux bien en dessous des 32767 du format, et ensemble au plus 20212. C'est
pourquoi ces assets peuvent jouer à plein volume l'un par-dessus l'autre sans
jamais demander de l'aide à l'écrêtage : la somme a de la marge avant la
limite.

### Le canal de la musique, en train de jouer

`MixerPlayMusic` dans `src/audio.cpp` est la route que prend la musique. Il
joue l'échantillon sur `AUDIO_MUSIC_CHANNEL` au volume donné et positionne le
drapeau de boucle du canal — la seule chose que `MixerPlay` ne fait jamais.
Voilà à quoi servait la réservation : l'arrière-plan est un long son sous tout
le reste, il prend le canal qu'aucun effet ne se voit jamais proposer, et rien
ne le vole. L'exécution lance sa musique là et son effet à travers le pool, et
les deux routes répondent avec leurs canaux :

```
engine: mix: music  -> channel  0 (looping, volume 256 of 256)
engine: mix: effect -> channel  1 (one-shot, volume 256 of 256)
engine: mix: the effect plays to its end; the music plays until the run stops it
```

Notez ce que `ChannelPlay` garantit désormais sur chaque route : un canal
démarre **non bouclé**. Le drapeau de boucle est positionné là où un son est
routé, pas repris de ce qui a joué sur ce canal auparavant — un effet du pool
qui atterrit par hasard sur un canal ayant déjà joué de la musique est un
one-shot, parce que `MixerPlayEffect` ne demande pas de boucle.

### Joue jusqu'à ce qu'on le lui dise

Un canal qui boucle ne se termine pas tout seul, donc quelqu'un doit y mettre
fin, et ce quelqu'un est l'exécution : `MixerStop` arrête maintenant le son
d'un canal. Le script de l'exécution arrête la musique au 600e tampon et le
journal montre toute l'histoire dans les propres nombres du mixeur :

```
engine: loop: the effect ended on channel 1 — 8820 frames in 12 buffers, the one-shot played to its end
engine: loop: music wrapped on channel 0 — wrap 1, 133035 frames played, cursor 735 of 132300
engine: loop: music wrapped on channel 0 — wrap 2, 265335 frames played, cursor 735 of 132300
engine: loop: music wrapped on channel 0 — wrap 3, 397635 frames played, cursor 735 of 132300
engine: mix: MixerStop ended the music on channel 0 — 441000 frames in 600 buffers, 3 wraps, cursor 44100 of 132300
engine: mix: 600 buffers of sound, then silence
```

Les retournements sont observés, pas supposés : l'exécution surveille le
curseur du canal de la musique, et un curseur qui va **en arrière** est le
retournement. Chaque ligne de retournement est cette observation avec
l'arithmétique qui l'explique — `frames played` est le nombre de passes
complètes sur les 132300 trames de la boucle plus la place du curseur dans la
passe courante. Le retournement 1 affiche 133035 trames jouées et un curseur de
735 : `1 × 132300 + 735`, et 133035, c'est 181 tampons de 735 — le retournement
est paresseux, et survient au premier tirage au-delà de la fin de
l'échantillon, un tampon après que le curseur s'est posé exactement sur 132300.

La ligne d'arrêt est l'affirmation que cette exécution a été construite pour
établir. À l'arrêt, la musique avait joué **441000 trames en 600 tampons** —
trois tours et un tiers autour de sa propre longueur de 132300 trames — en
était à sa quatrième passe au curseur 44100, et était encore active quand
`MixerStop` y a mis fin. Bien au-delà de la longueur de l'échantillon, le canal
se trouvait exactement là où l'arithmétique de la boucle dit qu'il devrait
être, trois retournements comptés en chemin. Et la ligne de silence clôt le
compte de l'exécution dans les propres nombres du mixage : 600 tampons ont
porté du son, puis plus rien — non pas parce qu'un échantillon s'est terminé,
mais parce que l'exécution a arrêté le dernier son.

### Le one-shot à côté, inchangé

La première ligne de ce journal est l'autre moitié du contraste. L'effet —
mêmes canaux du même moteur, même `MixBuffer` — a joué ses 8820 trames en 12
tampons et s'est terminé. Aucune décision de l'exécution ne l'a arrêté ; c'est
son échantillon qui l'a terminé. C'est exactement le comportement de la leçon
065 avec le drapeau de boucle à zéro, et c'est exactement ce que doit être un effet
sonore : **joue jusqu'à sa fin** là où la musique **joue jusqu'à ce qu'on le
lui dise**. Musique et effets utilisent les mêmes canaux, les mêmes volumes et
le même mixage ; la seule différence entre eux est le canal, et si le curseur
se retourne.

### Ce que cette exécution a vérifié, et ce qu'elle n'a pas vérifié

Tous les nombres proviennent d'exécutions réelles de l'état final de cette
leçon sur cette machine, face au périphérique `null` d'ALSA :

- **La boucle continue et se retourne comme documenté.** Trois retournements
  sont observés depuis l'arithmétique du curseur elle-même — 133035, 265335,
  397635 trames jouées — et à l'arrêt scripté de l'exécution, la musique était
  encore active, 441000 trames dans un échantillon de 132300, curseur 44100.
  Le canal de la musique est actif bien au-delà de la longueur de l'échantillon
  parce que rien dans la longueur de l'échantillon n'y met fin : le curseur se
  retourne et le canal continue de jouer.
- **`MixerStop` y met fin.** Un appel au 600e tampon et le canal est inactif :
  le tampon suivant ne porte aucun son et l'exécution nomme le silence — 600
  tampons de son, puis le silence.
- **La couture est vérifiable en octets.** L'exécution imprime les premières
  trames de chaque son et sa dernière trame : la dernière trame de la musique
  est `0` et ses premières trames commencent à `0`. L'exercice 1 parcourt la
  couture tampon par tampon.
- **Le build porte exactement un avertissement :** `DrawScene` défini mais non
  utilisé dans `src/main.cpp` — la verrue de la partie 2 reportée à dessein —
  et cette leçon n'en ajoute aucun. `tools/check-boundary.sh` passe toujours :
  le retournement est de l'arithmétique sur un `int` et ne pose aucune question
  à l'OS — et `openspec validate --all` rapporte 11 passed, 0 failed.

Ce que cette exécution ne peut pas vérifier, c'est la part qui exige des
oreilles. Aucun haut-parleur n'a émis de son — `null` prend les échantillons et
les jette. Que la couture de la boucle soit inaudible sur du matériel réel, et
que ces notes valent la peine d'être entendues deux fois, sont des questions
pour une machine qui fait du son ; ici l'affirmation s'arrête aux octets, et
les octets disent que la couture vaut zéro.

## Étape de code

Une modification pour cette leçon, trois fichiers : `src/audio.h` et
`src/audio.cpp` font grandir la boucle — `Channel.loop`, le retournement dans
le tirage par trame, `MixerPlayMusic` sur le canal de la musique, et
`MixerStop` — et `src/main.cpp` joue les deux sons de l'exécution à travers
eux, en guettant les retournements du curseur qui boucle et en arrêtant la
musique devant le lecteur. Les deux nouveaux assets voyagent avec l'étape de
code et en sont l'autre moitié : `assets/music.wav`, la boucle construite pour
que sa couture soit du silence, et `assets/effect.wav`, le one-shot que la
leçon 067 déclenche. Leurs octets sont binaires et leurs diffs aussi — git
imprime `Binary files … differ` pour eux ; la ligne de faits de l'exécution
ci-dessus est leur contenu dans les propres nombres du moteur. Le mixage, le
pool et l'allocateur sont ceux de la leçon 065, intacts. Son état final est
étiqueté `lesson-066`.

```diff
diff --git a/assets/effect.wav b/assets/effect.wav
new file mode 100644
index 0000000..df7300c
Binary files /dev/null and b/assets/effect.wav differ
diff --git a/assets/music.wav b/assets/music.wav
new file mode 100644
index 0000000..bede2fe
Binary files /dev/null and b/assets/music.wav differ
diff --git a/src/audio.cpp b/src/audio.cpp
index abe239b..0849b9d 100644
--- a/src/audio.cpp
+++ b/src/audio.cpp
@@ -193,6 +193,7 @@ void ChannelPlay(Channel &channel, const Sample &sample, int volume)
     channel.cursor = 0;
     channel.volume = volume;
     channel.active = true;
+    channel.loop = false; /* lesson 066: unlooped unless a route says otherwise */
 }
 
 namespace {
@@ -202,8 +203,26 @@ namespace {
    silence is not a value here, it is the absence of a contribution. */
 int ChannelFrame(Channel &channel)
 {
-    if (channel.active && channel.sample &&
-        channel.cursor < channel.sample->frame_count) {
+    if (channel.active && channel.sample) {
+        /* Lesson 066: the cursor's arithmetic at the sample's end. A
+           looping channel wraps — the cursor returns to the sample's
+           first frame and the pull below takes it again from there — so
+           the sample plays again from its start instead of ending. The
+           wrap is this one line; the mix above never knows it happened.
+           A sample with no frames has nothing to wrap to, and ends here
+           like any other. */
+        if (channel.cursor >= channel.sample->frame_count) {
+            if (channel.loop && channel.sample->frame_count > 0)
+                channel.cursor = 0;
+            else {
+                /* The sample's end — the fact frame_count carries. A
+                   channel that does not loop ends here, exactly as
+                   lesson 065 had it. */
+                channel.active = false;
+                return 0;
+            }
+        }
+
         /* One sample frame, scaled to the channel's volume. A frame is
            sample.channels values wide; the engine's stream is one
            channel wide, so it takes the frame's first value. */
@@ -213,7 +232,7 @@ int ChannelFrame(Channel &channel)
         return (frame * channel.volume) / AUDIO_VOLUME_FULL;
     }
 
-    /* The sample's end — the fact frame_count carries. */
+    /* Silence: the absence of a contribution. */
     channel.active = false;
     return 0;
 }
@@ -231,6 +250,7 @@ void MixerInit(Mixer &mixer)
         mixer.channels[c].cursor = 0;
         mixer.channels[c].volume = 0;
         mixer.channels[c].active = false;
+        mixer.channels[c].loop = false;
         mixer.channels[c].started = 0;
     }
     mixer.order = 0;
@@ -262,6 +282,25 @@ int MixerPlay(Mixer &mixer, const Sample &sample, int volume)
     return oldest;
 }
 
+void MixerPlayMusic(Mixer &mixer, const Sample &sample, int volume)
+{
+    /* The music channel — the reservation of lesson 065, spent here —
+       and the loop flag set: the run stops this sound with MixerStop,
+       the sample never does. */
+    ChannelPlay(mixer.channels[AUDIO_MUSIC_CHANNEL], sample, volume);
+    mixer.channels[AUDIO_MUSIC_CHANNEL].loop = true;
+    mixer.channels[AUDIO_MUSIC_CHANNEL].started = ++mixer.order;
+}
+
+void MixerStop(Mixer &mixer, int channel)
+{
+    /* The channel goes inactive and the mix stops pulling from it. Its
+       cursor keeps the place it stopped at; whatever plays on the channel
+       next starts from the sample's first frame. */
+    if (channel >= 0 && channel < AUDIO_MIXER_CHANNELS)
+        mixer.channels[channel].active = false;
+}
+
 void MixBuffer(Mixer &mixer, short *out, int frame_count)
 {
     for (int i = 0; i < frame_count; ++i) {
diff --git a/src/audio.h b/src/audio.h
index af36e32..e20f6d6 100644
--- a/src/audio.h
+++ b/src/audio.h
@@ -74,17 +74,25 @@ constexpr int AUDIO_VOLUME_FULL = 256;
 
 /* Lesson 063: one channel — the unit of playback. What it is playing,
    where it is in the sample, and how loud: three plain values, not a
-   device. A channel plays its sample to its end and then is free again. */
+   device. A channel plays its sample to its end and then is free again.
+   Lesson 066: or it loops — its cursor returns to the sample's first
+   frame at the sample's end and the channel plays on until the run
+   stops it. */
 struct Channel {
     const Sample *sample; /* what it is playing, or 0 */
     int cursor;           /* the next sample frame to read */
     int volume;           /* 0..AUDIO_VOLUME_FULL, fixed point */
     bool active;          /* playing now */
+    bool loop;            /* lesson 066: wrap to the sample's first frame
+                             at its end instead of ending */
     long started;         /* when this channel began, in the mixer's order */
 };
 
 /* Starts `sample` playing on this channel at `volume`, from its first
-   frame. Playing on one channel leaves every other channel alone. */
+   frame. Playing on one channel leaves every other channel alone. The
+   channel starts unlooped: looping is a decision made where a sound is
+   routed — `MixerPlayMusic` makes it — never something carried over from
+   whatever played on the channel before. */
 void ChannelPlay(Channel &channel, const Sample &sample, int volume);
 
 /* The mixer's fixed set of channels. */
@@ -112,6 +120,19 @@ void MixerInit(Mixer &mixer);
    music channel is never stolen. */
 int MixerPlay(Mixer &mixer, const Sample &sample, int volume);
 
+/* Starts `sample` playing as the run's music: on the music channel,
+   looping, at `volume`. That is what the reserved channel was reserved
+   for — a looping sound under everything else, one that no effect ever
+   takes or steals. The music runs until the run stops it with
+   `MixerStop`; its sample never ends it. */
+void MixerPlayMusic(Mixer &mixer, const Sample &sample, int volume);
+
+/* Stops the sound on `channel` now — the run's decision, because a
+   looping channel does not end on its own. The channel keeps the place
+   it stopped at, goes inactive, and whatever plays on it next starts
+   from the sample's first frame. */
+void MixerStop(Mixer &mixer, int channel);
+
 /* The mix: `frame_count` frames of stream, each one the sum of every
    active channel's next frame at its volume, clamped to the format's
    range. Clamped, never wrapped — a sum past the range lands on the
diff --git a/src/main.cpp b/src/main.cpp
index 1e86986..10cdc13 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -57,6 +57,53 @@ static void DrawScene(Framebuffer &fb, const TileMap &map,
     BlitSprite(fb, sprite, sprite_x - x, sprite_y - y);
 }
 
+/* Lesson 066: a loaded sample's facts, printed — the run's byte-level
+   check on its two sounds. The peak is the largest frame the sample
+   holds, and it is what says how much room the format still has above
+   the sound. */
+static void PrintSample(const char *name, const Sample &sample)
+{
+    int peak = 0;
+    for (int i = 0; i < sample.frame_count; ++i) {
+        int v = sample.frames[i * sample.channels];
+        if (v < 0)
+            v = -v;
+        if (v > peak)
+            peak = v;
+    }
+    std::printf("engine: %s: %d frames at %d Hz, %d channel%s, peak %d, first frames:",
+                name, sample.frame_count, sample.rate, sample.channels,
+                sample.channels == 1 ? "" : "s", peak);
+    for (int i = 0; i < 8 && i < sample.frame_count; ++i)
+        std::printf(" %d", (int)sample.frames[i]);
+    std::printf(", last frame %d\n",
+                sample.frame_count ? (int)sample.frames[sample.frame_count - 1]
+                                   : 0);
+}
+
+/* Lesson 066: one asset load's whole failure path — a failed load is
+   named typed and ends the run by name, exactly like the loads above it. */
+static bool LoadRunSample(Arena &arena, const char *path, Sample &into)
+{
+    SampleResult loaded = LoadSample(arena, path);
+    if (loaded.error == SAMPLE_OK) {
+        into = loaded.sample;
+        return true;
+    }
+    switch (loaded.error) {
+    case SAMPLE_MISSING:
+        std::fprintf(stderr, "engine: %s: could not load (missing)\n", path);
+        break;
+    case SAMPLE_MALFORMED:
+        std::fprintf(stderr, "engine: %s: could not load (malformed)\n", path);
+        break;
+    default:
+        std::fprintf(stderr, "engine: %s: could not load (no room)\n", path);
+        break;
+    }
+    return false;
+}
+
 int Run(void)
 {
     platform::WindowResult opened =
@@ -126,58 +173,48 @@ int Run(void)
     }
     TileSheet &sheet = tiles_loaded.sheet;
 
-    /* Lesson 061: the run's sound as a file's bytes. A sample is frames
-       of amplitude in a container, and the load either yields the
-       complete sample or names what went wrong — like every asset above.
-       A failure ends the run by name, like every asset above. */
-    SampleResult sample_loaded = LoadSample(arena, "assets/tone.wav");
-    if (sample_loaded.error != SAMPLE_OK) {
-        switch (sample_loaded.error) {
-        case SAMPLE_MISSING:
-            std::fprintf(stderr,
-                         "engine: assets/tone.wav: could not load (missing)\n");
-            break;
-        case SAMPLE_MALFORMED:
-            std::fprintf(stderr,
-                         "engine: assets/tone.wav: could not load (malformed)\n");
-            break;
-        default:
-            std::fprintf(stderr,
-                         "engine: assets/tone.wav: could not load (no room)\n");
-            break;
-        }
+    /* Lesson 066: the run's two sounds as files' bytes — the music that
+       loops and the effect that plays once. Lesson 061's tone leaves the
+       run here (it stays on disk: the file lessons 059-065 were built
+       on); the game's own sounds are these two. Each load either yields
+       the complete sample or names what went wrong, and a failure ends
+       the run by name — like every asset above. */
+    Sample music = {}, effect = {};
+    if (!LoadRunSample(arena, "assets/music.wav", music) ||
+        !LoadRunSample(arena, "assets/effect.wav", effect)) {
         platform::CloseWindow(opened.window);
         ArenaRelease(arena);
         return 1;
     }
-    Sample &sample = sample_loaded.sample;
-
-    /* The byte-level check, before anything is played: the sample's facts
-       and its first frames — the same bytes lesson 059 computed, now read
-       from a file instead. */
-    std::printf("engine: sample: %d frames at %d Hz, %d channel%s, first frames:",
-                sample.frame_count, sample.rate, sample.channels,
-                sample.channels == 1 ? "" : "s");
-    for (int i = 0; i < 8 && i < sample.frame_count; ++i)
-        std::printf(" %d", (int)sample.frames[i]);
-    std::printf("\n");
 
-    /* Lesson 065: a scripted burst of effects — one more than the pool's
-       effect channels — so the allocation policy runs in front of the
-       reader: the first free channel for each sound, and then the oldest
-       effect channel stolen. Volume is low so sixteen of them still sum
-       inside the format. */
+    /* The byte-level check, before anything is played: each sound's facts,
+       its peak, and its first frames — the same check lesson 061 made on
+       its one file, now on both. */
+    PrintSample("music", music);
+    PrintSample("effect", effect);
+
+    /* Lesson 066: the music as a loop and one effect as a one-shot in the
+       same run — the difference this lesson is about. The music takes the
+       music channel and runs until the run stops it; the effect takes a
+       pool channel and runs to its end. Both are the engine's format and
+       sum through the same mix. */
     Mixer mixer;
     MixerInit(mixer);
-    const int EFFECT_CHANNELS =
-        AUDIO_MIXER_CHANNELS - AUDIO_MUSIC_CHANNEL - 1;
-    for (int i = 0; i <= EFFECT_CHANNELS; ++i) {
-        int ch = MixerPlay(mixer, sample, AUDIO_VOLUME_FULL / 16);
-        std::printf("engine: mix: effect %2d -> channel %2d%s\n", i + 1, ch,
-                    i < EFFECT_CHANNELS ? "" : " (the oldest was stolen)");
-    }
-    std::printf("engine: mix: music channel %d is reserved and was never stolen\n",
-                AUDIO_MUSIC_CHANNEL);
+    MixerPlayMusic(mixer, music, AUDIO_VOLUME_FULL);
+    std::printf("engine: mix: music  -> channel %2d (looping, volume %d of %d)\n",
+                AUDIO_MUSIC_CHANNEL, mixer.channels[AUDIO_MUSIC_CHANNEL].volume,
+                AUDIO_VOLUME_FULL);
+    int effect_channel = MixerPlay(mixer, effect, AUDIO_VOLUME_FULL);
+    std::printf("engine: mix: effect -> channel %2d (one-shot, volume %d of %d)\n",
+                effect_channel, mixer.channels[effect_channel].volume,
+                AUDIO_VOLUME_FULL);
+    std::printf("engine: mix: the effect plays to its end; the music plays until the run stops it\n");
+
+    /* The run's script: the music stops after this many buffers. 600
+       buffers of 735 frames are 441000 frames of music — three and a
+       third times around its 132300-frame loop. The stop is the run's
+       decision; the loop itself would go on. */
+    constexpr int MUSIC_STOP_FEEDS = 600;
 
     double sprite_x = 312.0, sprite_y = 232.0;
     double started = platform::Now();
@@ -219,7 +256,7 @@ int Run(void)
            feed — the horizon the paced wait keeps queued. The buffer's
            length in time is the sample's own rate answering. */
         std::printf("engine: stream: %d-frame buffers, horizon %.1f ms; the loop feeds one when it is due\n",
-                    CHUNK_FRAMES, 1e3 * CHUNK_FRAMES / sample.rate);
+                    CHUNK_FRAMES, 1e3 * CHUNK_FRAMES / music.rate);
     }
 
     /* The frame step: read news, update from polled state, feed the
@@ -235,12 +272,15 @@ int Run(void)
        last one — and the loop knows that without asking the platform. */
     double next_feed = platform::Now();
 
-    /* Lesson 062: the channel's place in the sample — how far playback
-       has reached, what has been fed, and whether the end has been
-       named. The sample's frame_count is the fact that says when the
-       sample ends; nothing here assumes how long it is. */
-    int sample_feeds = 0;       /* buffers that carried sound */
-    bool sample_end_named = false;
+    /* Lesson 066: the loop's own bookkeeping, in the run's numbers — how
+       many buffers have been fed and how many carried sound, how far the
+       music has played (its wraps and its cursor say), and whether the
+       silence after the stop has been named. Nothing here assumes how
+       long the loop is. */
+    int feeds = 0;         /* buffers handed to the device */
+    int sound_feeds = 0;   /* buffers that carried sound */
+    int music_wraps = 0;   /* times the looping cursor returned to frame 0 */
+    bool silence_named = false;
     while (!platform::CloseRequested(opened.window)) {
         platform::PumpEvents(opened.window);
         if (platform::CloseRequested(opened.window))
@@ -344,32 +384,68 @@ int Run(void)
 
            Lesson 064: the stream is the mix. One buffer is every active
            channel's next frames summed and clamped — silence where no
-           channel has anything to say. frame_count is the fact that says
-           when a sample ends; no channel ever runs past it. */
+           channel has anything to say. Lesson 066: frame_count is still
+           the fact that says where a sample ends; a channel that loops
+           wraps there instead of ending, and the mix does not know the
+           difference. */
         double t_audio = platform::Now();
         if (audio.output && t_audio >= next_feed) {
-            /* Did this buffer carry sound? Answered from the cursors: the
-               buffer carried sample frames exactly when some channel's
-               cursor moved during the mix. */
-            long before = 0;
+            /* The run's script, one decision in it: at the 600th buffer
+               the run stops the music. A looping channel does not end on
+               its own, so ending it is the run's call — and this is the
+               call, made in front of the reader. */
+            if (feeds == MUSIC_STOP_FEEDS) {
+                MixerStop(mixer, AUDIO_MUSIC_CHANNEL);
+                std::printf("engine: mix: MixerStop ended the music on channel %d — %d frames in %d buffers, %d wraps, cursor %d of %d\n",
+                            AUDIO_MUSIC_CHANNEL, feeds * CHUNK_FRAMES, feeds,
+                            music_wraps,
+                            mixer.channels[AUDIO_MUSIC_CHANNEL].cursor,
+                            music.frame_count);
+            }
+
+            /* Does this buffer carry sound, and did the music wrap? Both
+               answered from the channels' own state: a channel active when
+               the mix starts speaks in this buffer, and a looping cursor
+               going backwards is the wrap. */
+            bool any = false;
             for (int c = 0; c < AUDIO_MIXER_CHANNELS; ++c)
-                before += mixer.channels[c].cursor;
+                any = any || mixer.channels[c].active;
+            int music_before = mixer.channels[AUDIO_MUSIC_CHANNEL].cursor;
+            bool effect_before = mixer.channels[effect_channel].active;
+
             MixBuffer(mixer, stream, CHUNK_FRAMES);
-            long after = 0;
-            for (int c = 0; c < AUDIO_MIXER_CHANNELS; ++c)
-                after += mixer.channels[c].cursor;
-            bool any = after > before;
+
+            if (effect_before && !mixer.channels[effect_channel].active) {
+                /* The one-shot's end, named in the sample's own numbers:
+                   it played once, to its end, and its channel is free. */
+                std::printf("engine: loop: the effect ended on channel %d — %d frames in %d buffers, the one-shot played to its end\n",
+                            effect_channel, effect.frame_count,
+                            (effect.frame_count + CHUNK_FRAMES - 1) /
+                                CHUNK_FRAMES);
+            }
+            if (mixer.channels[AUDIO_MUSIC_CHANNEL].active &&
+                mixer.channels[AUDIO_MUSIC_CHANNEL].cursor < music_before) {
+                /* The wrap: the cursor went backwards — the loop's own
+                   arithmetic, visible from outside the mixer. */
+                music_wraps += 1;
+                int cursor = mixer.channels[AUDIO_MUSIC_CHANNEL].cursor;
+                std::printf("engine: loop: music wrapped on channel %d — wrap %d, %ld frames played, cursor %d of %d\n",
+                            AUDIO_MUSIC_CHANNEL, music_wraps,
+                            (long)music_wraps * music.frame_count + cursor,
+                            cursor, music.frame_count);
+            }
 
             if (platform::SubmitSamples(audio.output, stream,
                                         CHUNK_FRAMES)) {
                 if (any)
-                    sample_feeds += 1;
-                if (!sample_end_named && !any) {
-                    /* The end, named in the mix's own numbers: how many
-                       buffers carried sound before it ran out. */
-                    sample_end_named = true;
+                    sound_feeds += 1;
+                if (!silence_named && !any) {
+                    /* The silence after the stop, named in the mix's own
+                       numbers: how many buffers carried sound before the
+                       run stopped the last sound. */
+                    silence_named = true;
                     std::printf("engine: mix: %d buffers of sound, then silence\n",
-                                sample_feeds);
+                                sound_feeds);
                 }
             } else {
                 /* A device that will not take the samples is named once,
@@ -380,10 +456,11 @@ int Run(void)
                 platform::CloseAudioOutput(audio.output);
                 audio.output = 0;
             }
+            feeds += 1;
             /* The schedule restarts from now, not from the missed slot: a
                long frame is caught up by one buffer, never by a backlog. */
             next_feed = platform::Now() +
-                        (double)CHUNK_FRAMES / (double)sample.rate;
+                        (double)CHUNK_FRAMES / (double)music.rate;
         }
         frame.audio = platform::Now() - t_audio;
 
```

## Exercices

Deux exercices mixtes. Chacun se termine par sa solution — un diff contre
l'état final de cette leçon plus une visite guidée — après l'énoncé.

### Exercice 1 — La couture, prédite *(predict-the-output)*

La couture d'une boucle est l'endroit où sa dernière trame rejoint sa première,
et les tampons de cette leçon la font tomber exactement sur une frontière de
tampon. Avant de lancer quoi que ce soit, prédisez les nombres autour d'elle.
Les tampons de l'exécution font 735 trames et la boucle en fait 132300 : dites
après quel tampon le curseur se pose pour la première fois sur la fin de
l'échantillon, quels tirages de quel tampon opèrent le retournement et où
ce tampon laisse le curseur, et ce que contient le tampon mixé à travers la
couture — les
trames à la fin du tampon d'avant le retournement et au début du tampon
d'après, pour la musique jouant seule à plein volume. Terminez la prédiction
par l'arithmétique de la ligne de retournement elle-même : comment
`frames played` se construit à partir du nombre de retournements et du curseur.
Donnez ensuite à l'exécution une sonde — un mixeur d'essai alimenté tampon par
tampon un peu au-delà d'une boucle, imprimant le curseur, le compteur de
retournements et les trames autour de la couture — et réconciliez chaque
nombre avec l'arithmétique du curseur.

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-066/ex1.md)

### Exercice 2 — Pause et reprise *(extend-the-code)*

`MixerStop` termine le son d'un canal et laisse son curseur là où il s'est
arrêté. Le menu pause d'un jeu veut autre chose : `MixerPause` suspend le son
là où il est et `MixerResume` reprend exactement de là, sans que le son soit
terminé. Rendez les deux réels à côté de `MixerStop`, l'échantillon, le
curseur, le volume et le drapeau de boucle survivant tous à une pause sans être
touchés. Faites passer la musique de l'exécution par l'un des deux sur un
mixeur d'essai — mettez-la en pause à un curseur connu, laissez des tampons
défiler pendant la pause, reprenez-la — et rapportez le curseur à la pause,
pendant celle-ci et après la reprise, plus ce que le mixage contenait dans les
tampons en pause. Répondez ensuite à une question en prose : que fait le
parcours du premier canal libre de `MixerPlay` à un canal **d'effet** en pause,
et est-ce ce qu'un jeu veut de son bouton pause ?

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-066/ex2.md)

---

**Partie :** [Partie 3 — le son](../../index.md) ·
**Précédente :** [Leçon 065 — l'allocation des canaux](lesson-065-allocation.md) ·
**Suivante :** [Leçon 067 — les effets comme des one-shots](lesson-067-effects.md) ·
**Étiquette de code :** [`lesson-066`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-066)

*Page traduite de la version anglaise `book/lessons/part-3/lesson-066-music.md`,
révision `70138ea`.*

<!-- translation-source: book/lessons/part-3/lesson-066-music.md @ 70138ea -->
