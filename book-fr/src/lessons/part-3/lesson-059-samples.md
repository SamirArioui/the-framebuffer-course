# Leçon 059 — le son comme échantillons

{{#include ../../stability-horizon.md}}

## Prose

Le sujet de la partie 3, c'est le son, et cette leçon porte exactement une idée :
**le son, ce sont des échantillons — des trames d'amplitude à une fréquence
fixe**. C'est le geste que la partie 0 a fait pour les images (la leçon 013 a
transformé un pixel en nombre), refait un sens plus tard : avant d'être une
hauteur, un volume ou un fichier, le son est une suite d'entiers ordinaires.
Tout ce que la partie 3 fait grandir n'est que de la comptabilité sur ces
entiers — le chargeur de la leçon 061 les lit, le mixeur des leçons 063-065 les
additionne — et leur forme ne change jamais.

### La trame, et la fréquence

Le format d'échantillons du moteur a exactement deux moitiés, et `audio.h`
déclare les deux.

**Une trame d'échantillons, c'est un `short`** — un nombre signé de 16 bits, de
−32768 à 32767 — qui dit où se situe le haut-parleur à cet instant. Zéro, c'est
le repos ; 32767, c'est aussi loin vers l'extérieur que le format va ; −32768,
aussi loin vers l'intérieur. Une trame ne porte rien d'autre : ni horodatage, ni
canal, ni compression, ni interprétation. C'est du **PCM 16 bits** — de simples
valeurs d'amplitude, l'équivalent audio du pixel compacté de la leçon 030 : un
format si simple que les nombres se vérifient avant que quoi que ce soit ne soit
joué.

**La fréquence est l'autre moitié du format.** `AUDIO_RATE` vaut 44100, et
44100 trames font une seconde de son. La fréquence est ce qui transforme
l'*indice* d'une trame en *temps* : la trame `i` s'entend à `i / 44100`
secondes. La trame 0, c'est `0 / 44100` = 0 s ; la trame 4410, 0,1 s ; la trame
44100, une seconde. Comme la largeur du framebuffer — le nombre qui fait qu'un
décalage de pixel veut dire une position — la fréquence n'est pas une politique
de lecture ; c'est le *sens* des nombres.

Le mixage du moteur tient sur un canal (`AUDIO_OUTPUT_CHANNELS`) : une trame, un
nombre. Ce que veut à la place le périphérique d'une machine donnée est
l'affaire de la couture, et la seconde moitié de cette leçon, c'est cette
couture.

### Une tonalité calculée par le code

D'où viennent les trames avant que la leçon 061 ne les lise dans des fichiers ?
Cette leçon les calcule. `GenerateTone` écrit `frame_count` trames d'une
sinusoïde à une fréquence et une amplitude données — un exemple travaillé de ce
qu'un échantillon *est*, pas une fonctionnalité de synthèse. Le corps de sa
boucle tient en trois lignes d'arithmétique, une idée par ligne :

- `time = i / AUDIO_RATE` — l'indice de trame en temps, la fréquence faisant son
  unique travail ;
- `wave = sin(TURN * frequency * time)` — le temps en position sur un tour de
  sinusoïde (`TURN` vaut deux pi). `wave` parcourt −1.0 à 1.0 et fait un tour
  complet toutes les `1 / frequency` secondes — 440 tours par seconde à 440 Hz ;
- `frames[i] = (short)(wave * amplitude * SAMPLE_PEAK)` — l'onde devenue
  échantillon : la mise à l'échelle projette le ±1,0 de la sinusoïde sur le
  format à l'amplitude donnée, et la conversion en `short` stocke la valeur
  entière (en tronquant vers zéro).

`SAMPLE_PEAK` vaut 32767.0, l'échantillon le plus positif du format — 32767 et
non 32768, parce que +32767 est la plus grande valeur que le format puisse
contenir ; une mise à l'échelle par 32768 déborderait à la crête positive de
l'onde, tandis qu'une mise à l'échelle par 32767 reste à l'intérieur du format à
toute amplitude jusqu'à 1,0. À l'amplitude 0,25, la crête vaut
`0.25 * 32767` = 8191,75 unités — et voilà pourquoi le pic est 8191 à
l'amplitude 0,25 : la vraie crête tombe entre deux échantillons, donc la trame
la plus forte que la tonalité stocke jamais est 8191.

Les nombres se vérifient sans le moindre périphérique son, et l'exécution les
vérifie. La démo imprime les premières trames de la tonalité au moment où elle
les fabrique :

```
engine: tone: 22050 frames at 44100 Hz, first frames: 0 513 1024 1531
engine: tone played
```

Le tampon fait `TONE_FRAMES` — `AUDIO_RATE / 2` = 22050 trames, une demi-seconde
de son. La trame 0 vaut exactement 0 parce que la sinusoïde part du repos, et
chaque trame suivante est un pas de phase de plus : 440/44100 de tour (environ
3,6°) par trame. Les pas sont visibles dans les quatre trames imprimées — 513,
puis 511, puis 507 — et ils ne cessent de rétrécir à mesure que la sinusoïde se
courbe vers sa crête. Là où cette crête tombe, et ce que sont les trames autour,
se prédit avant toute exécution ; l'exercice 1 vous demande de le prédire.

### Le retournement sans couture

Un fait à emporter dans la boucle d'alimentation de la leçon 060 : une seconde
d'une tonalité à 440 Hz fait **exactement 440 cycles** à 44 100 échantillons par
seconde — environ 100,227 trames par cycle, mais bien 440 cycles entiers par
seconde. Le tampon d'une demi-seconde de l'exécution contient 220 cycles entiers
pour la même raison. La fin du tampon rejoint donc son début sans couture : la
trame juste après sa fin serait de nouveau exactement la trame 0. Jouez le
tampon bout à bout et rien ne claque à la jointure — les nombres se retournent.
C'est une propriété de la fréquence de *cette* tonalité face à *cette* fréquence
d'échantillonnage, pas des tampons en général, et la boucle de la leçon 060
s'appuie dessus quand elle remet et remet à nouveau la même suite de trames au
périphérique.

### Le son à la couture

Atteindre un périphérique est l'affaire de la couture plateforme, exactement
comme atteindre une fenêtre. `platform.h` fait grandir la sortie audio à côté de
la fenêtre : `OpenAudioOutput`, `SubmitSamples`, `CloseAudioOutput`. La forme est
celle que la couture a depuis la leçon 027 :

- `OpenAudioOutput` rend soit un `AudioOutput`, soit le nom de l'étape qui a
  échoué — l'échec typé `AUDIO_NO_DEVICE` siège à côté d'`OPEN_NO_DISPLAY`,
  `AudioResult` à côté de `WindowResult`. Une machine sans sortie utilisable est
  une condition *nommée* : ni un plantage, ni un mystère.
- `SubmitSamples` remet au périphérique les `frames` trames d'échantillons
  suivantes dans le format du moteur. Le contrat est simple : les échantillons
  sont consommés avant que l'appel ne rende la main, le tampon redevient donc
  celui de l'appelant, et un périphérique qui refuse de les prendre répond
  `false`.
- `CloseAudioOutput` libère ce que l'ouverture a pris, et vide d'abord — les
  échantillons remis sont les échantillons joués.

Ce qu'un périphérique veut *à la place* du format du moteur est l'affaire de
l'implémentation, et `src/platform_alsa.cpp` est le fichier où vit cette affaire —
avec `src/platform_x11.cpp`, l'un des deux qu'un second OS remplace. L'unique
canal du moteur devient les deux canaux entrelacés du périphérique dans le
tampon de préparation de ce fichier, chaque échantillon atterrissant dans chaque
canal de sa trame — le mappage que `Present` opère pour les pixels, opéré pour
les échantillons. Quel périphérique est ouvert est aussi l'affaire de ce fichier ;
le moteur ne voit jamais le nom.

Le contrôle de frontière de la leçon 042 garde tout ceci honnête, et ses motifs
grandissent avec la couture : `<alsa/` rejoint les en-têtes d'OS, `snd_` rejoint
les appels d'OS. Le contrôle imprime toujours son compte rendu inchangé :

```
boundary: OK — OS headers and OS calls appear only in src/platform_x11.cpp src/platform_alsa.cpp
boundary: 17 single-line declarations there (multi-line ones are in the header)
```

### L'option de la chaîne d'outils

La règle depuis la partie 0 est que **la chaîne d'outils fait partie du
programme** : une option de compilation s'enseigne au moment où elle arrive.
L'option de cette leçon est `-lasound`, et elle arrive sur la ligne d'édition de
liens de `build.sh`, à côté de `-lX11` — la bibliothèque de la fenêtre et celle
du périphérique son, une de chaque côté de la couture.

Les lignes de compilation sont inchangées, et c'est en soi la seule trouvaille de
chaîne d'outils de la leçon, à consigner parce qu'elle a coûté un détour :
l'en-tête d'ALSA ne compile **pas** en `-std=c11` strict — il redéfinit
`struct timespec`, que le C strict réserve à l'implémentation — mais le moteur
est en C++ (`-std=c++17`) et prend l'en-tête tel quel. Le seul changement de
build dont cette leçon a besoin est l'option d'édition de liens. Les prérequis
accueillent `libasound2-dev` en conséquence (README), parce que l'en-tête et la
bibliothèque arrivent dans leur propre paquet.

### Ce que cette exécution a vérifié, et ce qu'elle n'a pas vérifié

La machine de l'auteur n'a **aucun matériel son**. Le dire sans détour coûte
moins cher qu'une fiction :

- **L'exécution a été vérifiée contre le périphérique logiciel `null` d'ALSA** —
  le périphérique qui accepte les échantillons et les jette, à travers les mêmes
  appels `OpenAudioOutput` / `SubmitSamples` / `CloseAudioOutput` qu'un vrai
  périphérique. `engine: tone played` veut dire que le périphérique a pris les
  trames ; sur `null`, c'est *tout* ce que ça veut dire.
- **Le chemin du périphérique manquant a été vérifié comme un échec typé** —
  pointez l'exécution vers un nom de périphérique qui n'existe pas et stderr dit,
  au-dessus de notre compte rendu :

  ```
  ALSA lib pcm.c:2721:(snd_pcm_open_noupdate) Unknown PCM no-such-device-xyz
  engine: no audio output on this machine
  engine: continuing without sound
  ```

  La première ligne est la bibliothèque d'ALSA elle-même qui parle — l'OS qui
  fait son propre rapport. La nôtre est l'échec typé : `AUDIO_NO_DEVICE`, nommé
  et imprimé, et l'exécution se poursuit jusqu'à une fermeture propre
  (`engine: closed`). L'échec est une valeur, pas un point final.
- **Aucun haut-parleur n'a émis de son.** Que la tonalité soit audible, à quel
  volume, à quelle hauteur sur du vrai matériel — rien de tout cela n'a été
  vérifié ici, et rien dans ce cours ne prétend le contraire. « Est-ce que ça
  sonne vraiment juste ? » est une question qui porte sur *votre* machine, et
  c'est l'exercice 2 qui y répond.

## Étape de code

Un seul changement pour cette leçon, du format au périphérique : `src/audio.h` /
`src/audio.cpp` accueillent le format d'échantillons et `GenerateTone` — la
tonalité calculée par le code, avant qu'aucun fichier n'en contienne une ;
`src/platform.h` accueille la sortie audio avec son échec typé `AUDIO_NO_DEVICE`,
et `src/platform_alsa.cpp` est le nouveau fichier d'OS qu'un second OS remplace ;
`src/main.cpp` ouvre la sortie, génère le premier son de l'exécution et le
soumet ; `build.sh` gagne `-lasound` sur sa ligne d'édition de liens ; et
`tools/check-boundary.sh` accueille ses motifs d'OS pour que la frontière
continue d'être contrôlée. La boucle de démo, le monde et le chemin de dessin ne
sont pas touchés. Son état final est étiqueté `lesson-059`.

```diff
diff --git a/build.sh b/build.sh
index f852f8c..0b62656 100755
--- a/build.sh
+++ b/build.sh
@@ -13,8 +13,9 @@
 # Environment overrides:
 #   CC, CFLAGS       compiler and flags for C sources
 #   CXX, CXXFLAGS    compiler and flags for C++ sources
-#   LDFLAGS          extra link flags (default: the OS library the platform
-#                    layer wraps — -lX11 on the Linux/X11 main line)
+#   LDFLAGS          extra link flags (default: the OS libraries the platform
+#                    layer wraps — -lX11 for the window on the Linux/X11 main
+#                    line, -lasound for the sound device)
 #   BUILD_DIR        output directory (default: build)
 
 set -euo pipefail
@@ -25,7 +26,7 @@ CC="${CC:-gcc}"
 CXX="${CXX:-g++}"
 CFLAGS="${CFLAGS:--std=c11 -O0 -g -Wall -Wextra}"
 CXXFLAGS="${CXXFLAGS:--std=c++17 -O0 -g -Wall -Wextra}"
-LDFLAGS="${LDFLAGS:--lX11}"
+LDFLAGS="${LDFLAGS:--lX11 -lasound}"
 BUILD_DIR="${BUILD_DIR:-build}"
 OBJ_DIR="$BUILD_DIR/obj"
 BIN="$BUILD_DIR/game"
diff --git a/src/audio.cpp b/src/audio.cpp
new file mode 100644
index 0000000..e942411
--- /dev/null
+++ b/src/audio.cpp
@@ -0,0 +1,38 @@
+// audio.cpp — the tone computed by code: arithmetic that becomes sound.
+//
+// Lesson 059: every sample in the engine is a sequence of numbers, and
+// this file makes one out of a sine wave so the numbers can be read,
+// checked, and played before any file format is involved. The same bytes
+// come back later as an asset (lesson 061) and go into the mixer (lesson
+// 063); here they are simply written.
+
+#include "audio.h"
+
+#include <cmath>
+
+namespace engine {
+namespace {
+
+/* One turn of the sine, in radians: two pi. */
+constexpr double TURN = 6.283185307179586;
+
+/* The format's most positive sample. The sine runs -1.0 to 1.0; scaling
+   by amplitude * 32767 lands inside the format at any amplitude <= 1.0. */
+constexpr double SAMPLE_PEAK = 32767.0;
+
+} /* namespace */
+
+void GenerateTone(short *frames, int frame_count, double frequency,
+                  double amplitude)
+{
+    for (int i = 0; i < frame_count; ++i) {
+        /* Frame i is heard i / AUDIO_RATE seconds in: the rate turns a
+           frame number into a time, and the sine turns a time into an
+           amplitude. */
+        double time = (double)i / (double)AUDIO_RATE;
+        double wave = std::sin(TURN * frequency * time);
+        frames[i] = (short)(wave * amplitude * SAMPLE_PEAK);
+    }
+}
+
+} /* namespace engine */
diff --git a/src/audio.h b/src/audio.h
new file mode 100644
index 0000000..11264b0
--- /dev/null
+++ b/src/audio.h
@@ -0,0 +1,37 @@
+// audio.h — sound as samples: frames of amplitude, the engine's format.
+//
+// Lesson 059: sound is data before it is sound. A sample is a frame of
+// amplitude — one number saying where the speaker sits at that instant —
+// and a sound is a run of those frames at a fixed rate. Nothing here knows
+// about devices, files, or mixing: this is the format the engine's output
+// speaks, the format lesson 061's loader accepts and refuses everything
+// else, and the format the mixer of lessons 063-065 sums.
+#ifndef AUDIO_H
+#define AUDIO_H
+
+namespace engine {
+
+/* The engine's sample format: 16-bit signed frames at this rate. One
+   sample frame is one `short`, from -32768 to 32767. The rate is the
+   format's other half: AUDIO_RATE frames make one second of sound, so a
+   frame's number in the run says exactly when it is heard. */
+constexpr int AUDIO_RATE = 44100; /* sample frames per second */
+
+/* The engine's mix is one channel wide. What the device itself wants is
+   the platform layer's business — it maps these samples into the device's
+   own layout, exactly as Present maps the framebuffer's pixels into the
+   window's. */
+constexpr int AUDIO_OUTPUT_CHANNELS = 1;
+
+/* A tone computed by code: `frame_count` sample frames of a sine wave at
+   `frequency` hertz, at `amplitude` (0.0 to 1.0), in the engine's format.
+   This is what a sample looks like before any file holds one — the bytes
+   a sound is made of, produced by arithmetic instead of read from disk.
+   It is a worked example of what a sample *is*, not a synthesis feature:
+   the engine plays samples, it does not design sounds. */
+void GenerateTone(short *frames, int frame_count, double frequency,
+                  double amplitude);
+
+} /* namespace engine */
+
+#endif
diff --git a/src/main.cpp b/src/main.cpp
index 95f09af..a848cad 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -10,6 +10,7 @@
 #include <cstdio>
 
 #include "arena.h"
+#include "audio.h"
 #include "blit.h"
 #include "camera.h"
 #include "font.h"
@@ -28,6 +29,9 @@ namespace engine {
    per-frame step. */
 constexpr double SPRITE_SPEED = 240.0; /* pixels per second */
 
+/* Lesson 059: the run's first sound is half a second of tone. */
+constexpr int TONE_FRAMES = AUDIO_RATE / 2;
+
 /* Lesson 054: the scene, drawn through the camera. The camera's summed
    offset is applied once, at each draw's origin — the map's and the
    sprite's. The HUD is not scene and does not pass through here. */
@@ -125,6 +129,48 @@ int Run(void)
     std::printf("engine: arrow keys move the sprite, space shakes the camera; close the window to stop\n");
     std::printf("engine: sprite at %d,%d\n", (int)sprite_x, (int)sprite_y);
 
+    /* Lesson 059: the run's first sound. A sample is a frame of amplitude
+       at the engine's rate — here a tone computed by code instead of read
+       from a file — and the seam's audio output is what puts those frames
+       in front of a device. */
+    platform::AudioResult audio =
+        platform::OpenAudioOutput(AUDIO_RATE, AUDIO_OUTPUT_CHANNELS);
+    if (!audio.output) {
+        switch (audio.error) {
+        case platform::AUDIO_NO_DEVICE:
+            std::fprintf(stderr, "engine: no audio output on this machine\n");
+            break;
+        default:
+            std::fprintf(stderr, "engine: audio error %d\n", audio.error);
+            break;
+        }
+        /* The failure is a value, not an ending: a machine with no output
+           still runs — this one continues without sound. */
+        std::fprintf(stderr, "engine: continuing without sound\n");
+    } else {
+        short *tone = (short *)ArenaAlloc(arena, TONE_FRAMES * sizeof(short),
+                                          sizeof(short));
+        if (!tone) {
+            std::fprintf(stderr, "engine: no room for the tone\n");
+        } else {
+            GenerateTone(tone, TONE_FRAMES, 440.0, 0.25);
+
+            /* The bytes are checkable before they are audible: the first
+               frames of the tone, in the engine's own format. */
+            std::printf("engine: tone: %d frames at %d Hz, first frames:",
+                        TONE_FRAMES, AUDIO_RATE);
+            for (int i = 0; i < 4; ++i)
+                std::printf(" %d", (int)tone[i]);
+            std::printf("\n");
+
+            if (platform::SubmitSamples(audio.output, tone, TONE_FRAMES))
+                std::printf("engine: tone played\n");
+            else
+                std::fprintf(stderr,
+                             "engine: the output would not take the samples\n");
+        }
+    }
+
     /* The frame step: read news, update from polled state, draw, present —
        every phase measured, one record per frame. */
     int exit_code = 0;
@@ -284,6 +330,7 @@ int Run(void)
 
     if (platform::CloseRequested(opened.window))
         std::printf("engine: close reported\n");
+    platform::CloseAudioOutput(audio.output);
     platform::CloseWindow(opened.window);
     ArenaRelease(arena);
     std::printf("engine: closed\n");
diff --git a/src/platform.h b/src/platform.h
index 29e8dd9..8e0f1ba 100644
--- a/src/platform.h
+++ b/src/platform.h
@@ -137,6 +137,38 @@ bool Present(Window *window, const unsigned char *pixels, int width,
 /* Releases everything OpenWindow took from the OS. */
 void CloseWindow(Window *window);
 
+/* The audio output: the OS's sound device behind the seam, beside the
+   window. The sample format is this interface's contract, not any OS's —
+   `rate` mono sample frames per second, each frame one signed 16-bit
+   value. What the device itself wants is the implementation's business:
+   it maps these samples into the device's own layout, exactly as Present
+   maps the framebuffer's pixels into the window's. */
+enum AudioError {
+    AUDIO_OK = 0,
+    AUDIO_NO_DEVICE, /* the OS has no usable output */
+};
+
+struct AudioOutput;
+
+struct AudioResult {
+    AudioOutput *output; /* the open output, or 0 on failure */
+    AudioError error;    /* AUDIO_OK exactly when output is non-0 */
+};
+
+/* Opens the OS's audio output at the engine's fixed sample format, or
+   names the step that failed. A machine with no usable output is a typed
+   failure — and a run that hears it may continue without sound. */
+AudioResult OpenAudioOutput(int rate, int channels);
+
+/* Hands the device the next `frames` sample frames of mixed samples —
+   `frames` * `channels` values, one frame after another. The samples are
+   consumed before the call returns, so the buffer is the caller's again.
+   Returns false when the device could not take them. */
+bool SubmitSamples(AudioOutput *output, const short *samples, int frames);
+
+/* Releases everything OpenAudioOutput took from the OS. */
+void CloseAudioOutput(AudioOutput *output);
+
 } /* namespace platform */
 
 #endif
diff --git a/src/platform_alsa.cpp b/src/platform_alsa.cpp
new file mode 100644
index 0000000..59e1fe6
--- /dev/null
+++ b/src/platform_alsa.cpp
@@ -0,0 +1,144 @@
+// platform_alsa.cpp — the ALSA implementation of the audio output.
+//
+// Lesson 059: the sound device is OS business, like the window is. This
+// file and platform_x11.cpp are the two a second OS replaces (the boundary
+// check's implementation list names them), and this is the only place an
+// ALSA header or an ALSA call appears. The engine sees platform.h and
+// nothing else.
+//
+// Which device this run opens is this file's business too: a machine with
+// no usable output reports AUDIO_NO_DEVICE, and a machine that has none
+// still runs.
+#define _POSIX_C_SOURCE 200809L
+
+#include "platform.h"
+
+#include <alsa/asoundlib.h>
+
+#include <stdlib.h> /* getenv */
+
+namespace platform {
+
+/* What an audio output is made of on this OS. The definition lives here,
+   where ALSA is visible; the engine holds the pointer and never looks
+   inside. */
+struct AudioOutput {
+    snd_pcm_t *device;
+    int engine_channels; /* the seam's frame width, and the device's own */
+    int device_channels; /* layout, which this file maps between */
+};
+
+/* The one output, in static storage: no new, no delete — the language law
+   of lesson 026 keeps allocation out of the engine and this layer alike. */
+static AudioOutput audio_state;
+
+/* The device to open. ALSA's `default` is what a machine with sound
+   answers with; the software device that accepts and discards samples
+   (the headless check's `null`) is named here instead. The engine never
+   sees the name — choosing a device is OS business. */
+static const char *DeviceName(void)
+{
+    const char *named = getenv("ALSA_DEVICE");
+    return named && named[0] ? named : "default";
+}
+
+/* The device's own shape. The engine's samples are one channel wide; this
+   machine's outputs take interleaved frames across two, so the samples are
+   duplicated here — the mix across both channels, which is the device's
+   business and not the engine's. */
+constexpr int DEVICE_CHANNELS = 2;
+
+/* The conversion buffer, in chunks: the engine's mono frames become the
+   device's interleaved frames. Static, like the rest of this file. */
+constexpr int STAGE_FRAMES = 1024;
+static short stage[STAGE_FRAMES * DEVICE_CHANNELS];
+
+AudioResult OpenAudioOutput(int rate, int channels)
+{
+    AudioResult result = { 0, AUDIO_NO_DEVICE };
+
+    snd_pcm_t *device = 0;
+    if (snd_pcm_open(&device, DeviceName(), SND_PCM_STREAM_PLAYBACK, 0) < 0)
+        return result; /* no usable output: named, not hidden */
+
+    /* The engine's format, field by field — every one is a promise the
+       engine's samples rely on. A device that cannot keep the rate is not
+       the engine's output: there is no resampling here, so the open fails
+       typed rather than quietly playing at the wrong speed. */
+    snd_pcm_hw_params_t *hw = 0;
+    snd_pcm_hw_params_malloc(&hw);
+    snd_pcm_hw_params_any(device, hw);
+    unsigned device_rate = (unsigned)rate;
+    bool formatted =
+        snd_pcm_hw_params_set_access(device, hw,
+                                     SND_PCM_ACCESS_RW_INTERLEAVED) >= 0 &&
+        snd_pcm_hw_params_set_format(device, hw, SND_PCM_FORMAT_S16_LE) >= 0 &&
+        snd_pcm_hw_params_set_channels(device, hw,
+                                       (unsigned)DEVICE_CHANNELS) >= 0 &&
+        snd_pcm_hw_params_set_rate_near(device, hw, &device_rate, 0) >= 0 &&
+        (unsigned)rate == device_rate &&
+        snd_pcm_hw_params(device, hw) >= 0;
+    snd_pcm_hw_params_free(hw);
+
+    if (!formatted) {
+        snd_pcm_close(device);
+        return result;
+    }
+
+    audio_state.engine_channels = channels;
+    audio_state.device = device;
+    audio_state.device_channels = DEVICE_CHANNELS;
+    result.output = &audio_state;
+    result.error = AUDIO_OK;
+    return result;
+}
+
+bool SubmitSamples(AudioOutput *output, const short *samples, int frames)
+{
+    if (!output || !output->device)
+        return false;
+
+    const short *at = samples;
+    int left = frames;
+    while (left > 0) {
+        int chunk = left < STAGE_FRAMES ? left : STAGE_FRAMES;
+
+        /* The engine's frames become the device's. The device has its own
+           channel count and the engine has `engine_channels` values per
+           frame; each device channel takes one of the engine's, wrapping
+           back to the first. With the engine's one channel that is one
+           sample in every channel of its frame — the mix duplicated
+           across the device's channels. */
+        for (int f = 0; f < chunk; ++f) {
+            for (int c = 0; c < output->device_channels; ++c) {
+                int from = f * output->engine_channels +
+                           c % output->engine_channels;
+                stage[f * output->device_channels + c] = at[from];
+            }
+        }
+
+        /* The device takes what it has room for; the rest comes back
+           around. When it takes nothing at all, the caller hears false. */
+        snd_pcm_sframes_t took = snd_pcm_writei(
+            output->device, stage, (snd_pcm_uframes_t)chunk);
+        if (took < 0)
+            return false;
+        at += (int)took * output->engine_channels;
+        left -= (int)took;
+    }
+    return true;
+}
+
+void CloseAudioOutput(AudioOutput *output)
+{
+    if (!output || !output->device)
+        return;
+
+    /* Drain first: the device finishes what it already has before the
+       handle goes away — the samples handed over are the samples played. */
+    snd_pcm_drain(output->device);
+    snd_pcm_close(output->device);
+    output->device = 0;
+}
+
+} /* namespace platform */
diff --git a/tools/check-boundary.sh b/tools/check-boundary.sh
index 5b55848..7caad33 100755
--- a/tools/check-boundary.sh
+++ b/tools/check-boundary.sh
@@ -25,10 +25,10 @@ IMPL="src/platform_x11.cpp src/platform_alsa.cpp"
 
 # Headers that only an OS has. The language's own headers (<cstdio>,
 # <cstring>, <stddef.h>, ...) are fine anywhere — they are not an OS.
-OS_HEADERS='<X11/|<sys/|<unistd\.h>|<fcntl\.h>|<poll\.h>|<signal\.h>|<errno\.h>|<time\.h>'
+OS_HEADERS='<X11/|<alsa/|<sys/|<unistd\.h>|<fcntl\.h>|<poll\.h>|<signal\.h>|<errno\.h>|<time\.h>'
 
 # Calls only an OS answers. The list grows with the seam.
-OS_CALLS='(^|[^A-Za-z0-9_:])(X[A-Z][A-Za-z]+|mmap|munmap|mprotect|clock_gettime|nanosleep|sysconf|open|close|read|write|fstat|poll|signal)\s*\('
+OS_CALLS='(^|[^A-Za-z0-9_:])(X[A-Z][A-Za-z]+|snd_[a-z_]+|mmap|munmap|mprotect|clock_gettime|nanosleep|sysconf|open|close|read|write|fstat|poll|signal)\s*\('
 
 status=0
 
```

## Exercices

Deux exercices mixtes. Chacun se termine par sa solution — un diff contre l'état
final de cette leçon, plus une visite guidée — après l'énoncé.

### Exercice 1 — Les huit premières trames *(predict-the-output)*

L'exécution imprime les quatre premières trames de la tonalité, et ces quatre-là
sont sur cette page — de quoi vérifier votre arithmétique. Avant de lancer quoi
que ce soit, prédisez les **huit** premières trames d'échantillons que
`GenerateTone` écrit pour la tonalité de l'exécution (440 Hz, amplitude 0,25)
sous forme d'entiers exacts, ainsi que le numéro de la trame où atterrit
l'échantillon le plus fort du tampon. Étendez ensuite la sonde de tonalité de
l'exécution pour qu'elle imprime huit trames et l'échantillon le plus fort avec
son numéro de trame, lancez-la — aucun périphérique son n'est nécessaire — et
réconciliez chaque valeur avec la mise à l'échelle de `GenerateTone`.

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-059/ex1.md)

### Exercice 2 — L'écouter sur de vrais haut-parleurs *(port-to-your-own-machine)*

Toutes les vérifications de cette leçon ont tourné contre le périphérique `null`
d'ALSA ; aucun haut-parleur n'a émis de son. Emmenez la démo sur une machine
dotée de vrai matériel son et répondez à la question que `null` ne peut pas
trancher : la tonalité est-elle audible, et est-elle ce que devrait être une
demi-seconde de 440 Hz à l'amplitude 0,25 ? Faites rapporter à l'exécution le
périphérique qu'elle a ouvert, pour que le journal de votre machine consigne ce
qui a été choisi ; puis lancez, écoutez, et rapportez ce que vous avez entendu —
le périphérique, la durée, la hauteur et le volume de la tonalité — ainsi que
tout ce qui diffère de ce que cette leçon prédit.

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-059/ex2.md)

---

**Partie :** [Partie 3 — le son](../../index.md) ·
**Précédente :** [Leçon 058 — la table du budget de frames](../part-2/lesson-058-budget.md) ·
**Suivante :** [Leçon 060 — la forme du flux](lesson-060-stream.md) ·
**Étiquette de code :** [`lesson-059`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-059)

*Page traduite de la version anglaise `book/lessons/part-3/lesson-059-samples.md`, révision `8c2591b`.*

<!-- translation-source: book/lessons/part-3/lesson-059-samples.md @ 8c2591b -->
