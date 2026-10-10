# Leçon 062 — les faits de lecture de l'échantillon

{{#include ../../stability-horizon.md}}

## Prose

La leçon 061 a chargé l'échantillon et répondu de ses octets : 22050 trames
d'échantillons d'une tonalité de 440 Hz à l'amplitude 0,25, dans le format du
moteur, vérifiées déclaration par déclaration. Cette leçon les joue, et son
idée tient en une ligne : **les faits de lecture voyagent avec la donnée**. Un
échantillon
répond aux questions que pose la lecture — quand finit-il, que veut dire
l'indice d'une trame en temps, quelle est la largeur d'une trame — à partir de
ses propres champs, et jamais à partir de suppositions sur le fichier qui l'a
produit. Tout le reste ici, c'est cette ligne à l'œuvre.

### Joué jusqu'à sa fin

Le flux de la leçon 060 avait un curseur qui parcourait une tonalité sans fin.
Le flux, désormais, c'est l'échantillon, et il s'arrête. Chaque alimentation
remplit un tampon — le tampon `stream`, `CHUNK_FRAMES` trames de celui-ci — à
partir de `sample.frames` là où il reste des trames à l'échantillon, et avec du
**silence** au-delà de sa fin. Le remplissage est borné par une soustraction et
un minimum : ce qui reste vaut `sample.frame_count - sample_cursor`, et
l'alimentation en prélève autant, ou la contenance d'un tampon, au plus petit
des deux. Quand l'échantillon s'épuise au milieu d'un tampon, le reste du tampon
est à zéro ; quand il y a longtemps qu'il est fini, le tampon entier l'est. Le
silence est
un flux lui aussi — le périphérique continue de recevoir ses tampons, au rythme
de l'horizon, jusqu'à ce que l'exécution se termine.

Les nombres de l'exécution disent le reste. Quand le curseur atteint
`frame_count`, la fin est nommée dans les termes de l'échantillon :

```
engine: sample: 22050 frames fed in 30 buffers — the sample's end; the stream is silence from here
```

Trente tampons de 735 trames font 22050 — le nombre de trames de l'échantillon,
à la trame près. Rien n'a été lu au-delà de la fin (le remplissage ne quitte
jamais les trames de l'échantillon), et rien n'a été perdu (le compte n'est ni
22049 ni 22051). La ligne atterrit dans le journal de l'exécution avec la
trentième alimentation, entre les frames que l'exécution traversait de part et
d'autre — trente horizons de 16,7 ms après le début du flux, soit la
demi-seconde que prédit 22050 / 44100. Et l'exécution ne s'arrête pas quand le son s'arrête : le
journal garde ses lignes `frame N:` jusqu'à la fin de l'exécution, la phase
`audio` toujours à l'œuvre dedans — les tampons de silence continuent de couler
(cette exécution : 178 frames en trois secondes, chacune après la fin continuant
d'alimenter).

Un fait arithmétique mérite d'être nommé avant qu'un exercice ne s'appuie
dessus : 22050 / 735 = 30 exactement, c'est la longueur de *cet asset* face à
*cette* taille de tampon. Ce n'est pas une promesse que le moteur fait. Un
échantillon de 1000 trames ne se divise pas du tout en tampons ; sa dernière
alimentation porte 265 trames d'échantillons et 470 trames de silence. Le code
s'en moque — `frame_count` dit quand, le remplissage dépense exactement ce
nombre de trames, et le tampon fait toujours un horizon de profondeur.

### Les faits voyagent avec la donnée

Regardez où les champs de l'échantillon sont dépensés dans l'alimentation, car
c'est tout l'argument de cette leçon :

- `sample.frame_count` borne le remplissage — la *longueur* de l'échantillon
  est un fait de l'échantillon, pas une constante de l'exécution ni un nom de
  fichier ;
- `sample.rate` traduit le tampon en temps — la prochaine alimentation est due
  un `CHUNK_FRAMES / sample.rate` après la dernière, et l'exécution demande à
  l'échantillon la durée d'un tampon, pas au format ;
- `sample.channels` est le pas entre les trames — chaque trame fait cette
  largeur en valeurs.

Le mixeur des prochaines leçons n'ouvre jamais de fichier. Il ne peut pas : un
son est déjà en mémoire quand le mixage commence, et tout ce dont le mixage et
la lecture ont besoin est venu avec lui. C'est pour cela que `Sample` a été
dessiné ainsi à la leçon 061, et c'est pourquoi la struct porte ces trois champs
à côté des trames au lieu de les laisser implicites dans « le format du
moteur ».

L'autre moitié de la promesse, c'est l'échec typé de la leçon 061 : **un
échantillon dans un autre format échoue de façon typée au lieu d'être joué mal
lu.** Ce que donne à entendre une lecture erronée mérite d'être dit en toutes
lettres, car cela reste silencieux dans les journaux : ce chunk `fmt ` qui
annonce 22050 Hz, joué comme s'il s'agissait de 44100, est une note une octave
trop haute, pour moitié moins longtemps — et il *se joue très bien*. Pas de
plantage, pas de mauvais nombre dans un rapport ; juste le mauvais son, partout
et pour toujours. Le chargeur refuse un tel fichier à la porte, si bien que la
lecture n'a même jamais l'occasion de se tromper. Le format du moteur n'est pas
une préférence ; c'est la condition sous laquelle ces trois champs veulent dire
ce que la boucle de lecture croit qu'ils veulent dire.

Et puisque s'arrêter est désormais le défaut : *sauf boucle* est une politique,
pas un fait. Le curseur de la leçon 060 bouclait la tonalité à l'infini et le
retournement (wrap) était sans couture — une seconde de 440 Hz à 44100 fait
exactement 440 cycles, le fait de retournement de la leçon 059. Un canal qui
boucle un échantillon le choisit, et la couture qu'il obtient est sa propre
question.

### La fin n'est pas le retournement

Ce qui laisse un petit nombre bien réel à la frontière. La boucle se retournait
à 0 : la trame juste après la fin de la tonalité *était* de nouveau la trame 0,
donc aucun clic. Un arrêt n'a pas cette chance — les dernières trames de
l'échantillon valent −2032, −1531, −1024, −513, et le silence vaut 0. Le
haut-parleur passe de −513 au repos : environ 1,6 % de la plage du format, un
petit pas, et au plus un léger clic. Savoir si ce clic s'entend sur du vrai
matériel est une question à laquelle seul le matériel peut répondre ; le compte
rendu de l'exécution ci-dessous est honnête sur ce point : il ne l'a pas posée.

### Ce que cette exécution a vérifié, et ce qu'elle n'a pas vérifié

Tous les nombres proviennent d'exécutions réelles de l'état final de cette
leçon sur cette machine, face au périphérique `null` d'ALSA :

- **Le canal joue exactement le nombre de trames de l'échantillon et s'arrête
  à sa fin.** Le rapport de l'exécution : `22050 frames fed in 30 buffers` —
  30 × 735 = 22050 = `frame_count` — et la fin nommée avec l'alimentation qui
  l'a atteinte. L'alimentation précédant le silence est exactement
  l'échantillon ; les alimentations qui la suivent sont du silence pur.
- **Le silence maintient le flux en vie.** L'exécution de trois secondes a fait
  178 frames et chaque frame après la fin de l'échantillon montre encore la
  phase `audio` à l'œuvre — le périphérique n'a cessé de recevoir ses tampons,
  un par horizon, jusqu'à la fermeture.
- **L'échec typé est inchangé** — un fichier manquant ou malformé termine
  toujours l'exécution sous son nom, les exécutions de la leçon 061 ont été
  relancées contre cet état sans modification (le chargeur n'est pas touché par
  cette leçon).

Ce que cette exécution ne peut pas vérifier, c'est la part qui demande des
oreilles : **aucun haut-parleur n'a émis de son** — `null` prend les
échantillons et les jette. « Il a joué une demi-seconde puis s'est arrêté
proprement » est une affirmation sur votre machine et vos oreilles, et c'est
l'exercice 2 qui la fait. Cette leçon affirme les nombres, et les nombres
disent que les trames de l'échantillon ont été dépensées exactement.

## Étape de code

Une modification pour cette leçon, un fichier : `src/main.cpp` remplace le
curseur de tonalité de la leçon 060 par l'échantillon. L'alimentation remplit un
tampon de flux à partir de `sample.frames` — borné par `sample.frame_count`,
réduisant au silence ce que l'échantillon ne couvre pas — et nomme la fin de
l'échantillon dans ses propres nombres quand il y arrive. La barrière, la phase
`audio`, l'attente cadencée et l'horizon sont exactement tels que la leçon 060
les a laissés ; ce qui change, c'est ce que le flux *est*. La tonalité calculée
quitte l'exécution — `GenerateTone` reste dans `src/audio.cpp` comme l'exemple
travaillé qui a produit `assets/tone.wav` à l'origine. Son état final est
étiqueté `lesson-062`.

```diff
diff --git a/src/main.cpp b/src/main.cpp
index e46c5bd..fbfd37d 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -29,21 +29,21 @@ namespace engine {
    per-frame step. */
 constexpr double SPRITE_SPEED = 240.0; /* pixels per second */
 
-/* Lesson 059: the run's sound is half a second of tone — 22050 frames.
-   One second of a 440 Hz tone is exactly 440 cycles at AUDIO_RATE, so
-   this buffer holds a whole 220 cycles and its end meets its beginning
-   with no click: the wrap lesson 059 found, which lesson 060's feeding
-   cursor leans on when it hands the same run of frames to the device
-   again and again. */
-constexpr int TONE_FRAMES = AUDIO_RATE / 2; /* 22050 */
-
 /* Lesson 060: one buffer of stream per feed — one sixtieth of a second,
-   the horizon the loop keeps queued. The cursor relies on the tone's
-   length dividing evenly into buffers: 22050 / 735 = 30 exactly, so it
-   wraps at a buffer boundary and no feed ever has to copy across the
-   tone's end. */
+   the horizon the loop keeps queued. Lesson 062: a feed is always
+   exactly this much stream — the sample's frames where the sample has
+   them, silence beyond its end — so the horizon arithmetic is untouched
+   whatever the sample's length is. The sample's own length is the file's
+   fact: playback stops where its frame_count says it stops, not where a
+   constant here would. */
 constexpr int CHUNK_FRAMES = AUDIO_RATE / 60;   /* 735 */
 
+/* Lesson 062: the buffer of stream one feed hands the device, filled
+   from the sample (or with silence) as the feed is due. Static, like the
+   platform layer's own staging buffers — the language law of lesson 026
+   keeps allocation out of the run. */
+static short stream[CHUNK_FRAMES];
+
 /* Lesson 054: the scene, drawn through the camera. The camera's summed
    offset is applied once, at each draw's origin — the map's and the
    sprite's. The HUD is not scene and does not pass through here. */
@@ -177,16 +177,14 @@ int Run(void)
     std::printf("engine: arrow keys move the sprite, space shakes the camera; close the window to stop\n");
     std::printf("engine: sprite at %d,%d\n", (int)sprite_x, (int)sprite_y);
 
-    /* Lesson 059: the run's sound. A sample is a frame of amplitude at
-       the engine's rate — here a tone computed by code instead of read
-       from a file — and the seam's audio output is what puts those frames
-       in front of a device. Lesson 060: those frames are a *stream*, not
-       one submission done at startup — the frame loop feeds the device
-       buffer by buffer, for as long as the run lasts. */
+    /* Lesson 059: the run's sound is a run of amplitude at the engine's
+       rate, and the seam's audio output is what puts those frames in
+       front of a device. Lesson 062: the frames are the sample loaded at
+       startup — a file's bytes, played to their end — and the loop feeds
+       them as the stream lesson 060 shaped: buffer by buffer, at the
+       horizon's pace, silence once the sample is done. */
     platform::AudioResult audio =
         platform::OpenAudioOutput(AUDIO_RATE, AUDIO_OUTPUT_CHANNELS);
-    short *tone = 0;
-    int tone_cursor = 0; /* where the stream's next buffer starts */
     if (!audio.output) {
         switch (audio.error) {
         case platform::AUDIO_NO_DEVICE:
@@ -200,26 +198,11 @@ int Run(void)
            still runs — this one continues without sound. */
         std::fprintf(stderr, "engine: continuing without sound\n");
     } else {
-        tone = (short *)ArenaAlloc(arena, TONE_FRAMES * sizeof(short),
-                                   sizeof(short));
-        if (!tone) {
-            std::fprintf(stderr, "engine: no room for the tone\n");
-        } else {
-            GenerateTone(tone, TONE_FRAMES, 440.0, 0.25);
-
-            /* The bytes are checkable before they are audible: the first
-               frames of the tone, in the engine's own format. */
-            std::printf("engine: tone: %d frames at %d Hz, first frames:",
-                        TONE_FRAMES, AUDIO_RATE);
-            for (int i = 0; i < 4; ++i)
-                std::printf(" %d", (int)tone[i]);
-            std::printf("\n");
-
-            /* And what the loop does with them: one buffer of stream per
-               feed — the horizon the paced wait keeps queued. */
-            std::printf("engine: stream: %d-frame buffers, horizon %.1f ms; the loop feeds one when it is due\n",
-                        CHUNK_FRAMES, 1e3 * CHUNK_FRAMES / AUDIO_RATE);
-        }
+        /* What the loop does with the sample: one buffer of stream per
+           feed — the horizon the paced wait keeps queued. The buffer's
+           length in time is the sample's own rate answering. */
+        std::printf("engine: stream: %d-frame buffers, horizon %.1f ms; the loop feeds one when it is due\n",
+                    CHUNK_FRAMES, 1e3 * CHUNK_FRAMES / sample.rate);
     }
 
     /* The frame step: read news, update from polled state, feed the
@@ -234,6 +217,15 @@ int Run(void)
        the engine's rate, so the next buffer is due one horizon from the
        last one — and the loop knows that without asking the platform. */
     double next_feed = platform::Now();
+
+    /* Lesson 062: the channel's place in the sample — how far playback
+       has reached, what has been fed, and whether the end has been
+       named. The sample's frame_count is the fact that says when the
+       sample ends; nothing here assumes how long it is. */
+    int sample_cursor = 0;      /* the next frame the feed takes */
+    int sample_fed = 0;         /* frames of sample handed to the device */
+    int sample_feeds = 0;       /* feeds that carried sample frames */
+    bool sample_end_named = false;
     while (!platform::CloseRequested(opened.window)) {
         platform::PumpEvents(opened.window);
         if (platform::CloseRequested(opened.window))
@@ -333,12 +325,40 @@ int Run(void)
            extra audio, or the run would bury the device in buffers instead
            of pacing them. The step is measured on every frame — it is ~0
            where no buffer was due — so the phase accounts for all of the
-           frame's audio work. */
+           frame's audio work.
+
+           Lesson 062: the stream is the sample. One buffer is filled from
+           the sample's frames where the sample has them and with silence
+           beyond its end — silence is a stream too, and the device keeps
+           getting its buffers. frame_count is the fact that says when the
+           sample ends; the fill never runs past it. */
         double t_audio = platform::Now();
-        if (audio.output && tone && t_audio >= next_feed) {
-            if (platform::SubmitSamples(audio.output, tone + tone_cursor,
+        if (audio.output && sample.frames && t_audio >= next_feed) {
+            /* Each frame is sample.channels values wide — one here, the
+               loader refuses anything else — and the engine's stream is
+               one channel wide, so a frame is its first (only) channel. */
+            int left = sample.frame_count - sample_cursor;
+            int take = left < CHUNK_FRAMES ? left : CHUNK_FRAMES;
+            for (int i = 0; i < take; ++i)
+                stream[i] =
+                    sample.frames[(sample_cursor + i) * sample.channels];
+            for (int i = take; i < CHUNK_FRAMES; ++i)
+                stream[i] = 0;
+
+            if (platform::SubmitSamples(audio.output, stream,
                                         CHUNK_FRAMES)) {
-                tone_cursor = (tone_cursor + CHUNK_FRAMES) % TONE_FRAMES;
+                sample_cursor += take;
+                sample_fed += take;
+                if (take > 0)
+                    sample_feeds += 1;
+                if (!sample_end_named &&
+                    sample_cursor == sample.frame_count) {
+                    /* The end, named in the sample's own numbers: what was
+                       fed before silence, and how many buffers carried it. */
+                    sample_end_named = true;
+                    std::printf("engine: sample: %d frames fed in %d buffers — the sample's end; the stream is silence from here\n",
+                                sample_fed, sample_feeds);
+                }
             } else {
                 /* A device that will not take the samples is named once,
                    not once per frame: the run closes the output and carries
@@ -351,7 +371,7 @@ int Run(void)
             /* The schedule restarts from now, not from the missed slot: a
                long frame is caught up by one buffer, never by a backlog. */
             next_feed = platform::Now() +
-                        (double)CHUNK_FRAMES / (double)AUDIO_RATE;
+                        (double)CHUNK_FRAMES / (double)sample.rate;
         }
         frame.audio = platform::Now() - t_audio;
 
```

## Exercices

Deux exercices mixtes. Chacun se termine par sa solution — un diff contre
l'état final de cette leçon plus une visite guidée — après l'énoncé.

### Exercice 1 — L'échantillon qui ne fait pas trente tampons *(predict-the-output)*

Les nombres de l'exécution sont suspectement ronds : 22050 trames, 30 tampons —
la longueur de l'échantillon se divise exactement par l'horizon, et rien dans
la lecture ne peut s'appuyer là-dessus. Prouvez-le. Réduisez `assets/tone.wav`
à 1000 trames — raccourcissez son chunk `data` et corrigez à la fois la taille
de `data` et la taille RIFF, pour que le conteneur soit bien formé mais court
(les retouches d'octets de la leçon 061). Avant de lancer quoi que ce soit,
prédisez toute la séquence d'alimentations : combien d'alimentations portent
des trames d'échantillons et combien de trames chacune porte, ce que le tampon
de la deuxième alimentation contient trame par trame, exactement ce que dira le
rapport de fin, et combien de trames d'échantillons le périphérique reçoit au
total. Donnez ensuite à l'alimentation une sonde — une ligne par tampon, nommant
combien de trames viennent de l'échantillon et combien du silence — lancez-la
contre votre fichier court, et réconciliez chaque nombre avec `frame_count`, le
remplissage et `CHUNK_FRAMES`. Terminez par la question à laquelle le tampon de queue
répond : qu'aurait fait à ce fichier une alimentation d'une ligne de
`CHUNK_FRAMES` trames à partir de `sample.frames + cursor` ?

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-062/ex1.md)

### Exercice 2 — L'entendre s'arrêter *(port-to-your-own-machine)*

Toutes les vérifications de cette leçon ont tourné face au périphérique `null`
d'ALSA — le périphérique qui prend les échantillons et les jette — si bien que
la seule chose que les nombres ne peuvent pas dire, c'est ce que le *stop*
donne à entendre. Emmenez la démo sur une machine dotée de vrai matériel
sonore et répondez : la tonalité joue une fois, une demi-seconde de 440 Hz, puis
s'arrête pendant que l'exécution continue de battre et d'alimenter en silence.
Faites en sorte que l'exécution rende compte du flux à sa fermeture — les
trames et les tampons de l'échantillon, et ce qui a été alimenté après la fin —
pour que votre journal porte la preuve à côté de vos oreilles ; rapportez
ensuite le périphérique, ce que vous avez entendu (la hauteur, la durée, l'arrêt
— propre ou un clic), et comment la demi-seconde se compare à trente tampons de
16,7 ms.

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-062/ex2.md)

---

**Partie :** [Partie 3 — le son](../../index.md) ·
**Précédente :** [Leçon 061 — le conteneur WAV](lesson-061-wav.md) ·
**Suivante :** [Leçon 063 — un canal](lesson-063-channel.md) ·
**Étiquette de code :** [`lesson-062`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-062)

*Page traduite de la version anglaise `book/lessons/part-3/lesson-062-playback.md`,
révision `fb99fc9`.*

<!-- translation-source: book/lessons/part-3/lesson-062-playback.md @ fb99fc9 -->
