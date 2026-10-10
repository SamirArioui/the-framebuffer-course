# Leçon 060 — la forme du flux

{{#include ../../stability-horizon.md}}

## Prose

La leçon 059 a fait du son avec des nombres et en a remis une demi-seconde à un
périphérique, une fois. Cette leçon porte exactement deux idées, et ensemble
elles constituent la forme de chaque son que ce moteur jouera jamais : **le
format et la fréquence de la sortie**, et **la boucle qui l'alimente — l'attente
cadencée qui empêche le périphérique de mourir de faim**. La première dit ce
qu'un son *est* au moment de sortir du moteur ; la seconde dit ce qu'une
exécution doit à un périphérique pendant qu'elle fait tout le reste de ce
qu'elle fait.

### Un flux, pas une soumission

La sortie est un **flux** : un format fixe et une fréquence fixe, convenus par
la couture. Pas « ce qui plaît au périphérique » — le contrat, ce sont les
échantillons du moteur lui-même, le format de la leçon 059 de bout en bout :
`AUDIO_RATE` vaut 44100 trames par seconde, le mixage du moteur tient sur un
canal (`AUDIO_OUTPUT_CHANNELS`), une trame, un `short`. Ce que veut à la place
le périphérique d'une machine donnée est l'affaire de la plateforme, et c'est le
mappage de la leçon 059 qui s'en charge — chaque trame mono dupliquée dans la
disposition stéréo entrelacée du périphérique à l'intérieur du fichier ALSA,
exactement comme `Present` projette les pixels du framebuffer dans la fenêtre.
La forme propre au périphérique ne franchit jamais la couture ; la forme du flux
est celle du *moteur*.

La fréquence est la moitié du contrat qui importe ici. 44100 trames font une
seconde — la trame `i` s'entend à `i / 44100` secondes — et un périphérique
consomme exactement à cette fréquence, trame après trame, au fil de l'horloge.
Remettez-lui une demi-seconde d'échantillons et il est occupé une demi-seconde ;
après quoi il n'a plus rien. C'est exactement ce que faisait la démo de la leçon
059 : une soumission au démarrage (`engine: tone played`), puis le silence. Un
tampon laissé derrière soi n'est pas un flux. Un flux, c'est la même suite de
trames remise *encore et encore* — un curseur qui parcourt le tampon de
tonalité, chaque alimentation prenant les `CHUNK_FRAMES` trames suivantes.

Deux faits de divisibilité rendent cette marche sans couture, et tous deux sont
consignés à côté des constantes sur lesquelles le curseur s'appuie :

- `TONE_FRAMES / CHUNK_FRAMES` vaut 22050 / 735 = 30 exactement, donc le curseur
  se retourne sur une frontière de tampon et aucune alimentation n'a jamais à
  copier à cheval sur la fin de la tonalité — la marche est une addition de
  pointeur et un modulo.
- une seconde d'une tonalité à 440 Hz fait exactement 440 cycles — le fait de
  retournement de la leçon 059 — donc là où le curseur se retourne, la fin de la
  tonalité rejoint son début sans clic.

Aucun des deux faits n'est de la chance : 44100 se divise par 60 et par 2, donc
la frontière de tampon est un vrai fait arithmétique et non une approximation ;
et 440 Hz, c'est 440 cycles entiers à chaque seconde entière, donc une
demi-seconde en fait 220 — voilà pourquoi le retournement se referme. Changez
sans précaution la fréquence de la tonalité ou la taille du tampon et c'est le
premier fait qui casse — un curseur qui se retourne au milieu d'un tampon a
besoin d'une copie scindée, et une tonalité dont les cycles ne se referment pas
claque à la jointure.

### La famine

Un périphérique consomme à sa propre cadence. L'exécution, non : elle met le
monde à jour, dessine, présente, attend les nouvelles. Si l'exécution s'absente
et fait un autre travail plus longtemps que ne durent les échantillons en file,
le périphérique tombe à court d'échantillons et le son saccade ou s'arrête.
C'est la **famine** (starvation), et ce n'est pas un défaut du périphérique —
c'est de l'arithmétique. Les échantillons sont consommés à 44100 par seconde,
donc la longueur d'une file est un *temps* : un tampon de 735 trames, c'est
735 / 44100 = un soixantième de seconde, pas plus.

La boucle doit donc garder des tampons en file : alimenter le périphérique selon
un calendrier, avant que la file ne s'assèche. Et c'est « selon un calendrier »
qui fait que le son change ce moteur.

### L'attente cadencée

Le correctif a deux moitiés, et une seule est visible dans `main.cpp`. La boucle
**alimente** le périphérique en tampons selon un calendrier — et l'attente des
nouvelles de l'exécution **ne peut plus bloquer indéfiniment**. `PumpEvents` dort
depuis la leçon 028 dans une attente qui se termine sur une nouvelle ou sur
l'interruption ; avec une sortie ouverte, elle doit aussi se terminer quand les
échantillons en file sont sur le point de s'épuiser. L'attente est bornée par la
durée de vie des échantillons en file — l'**horizon de tampon** de cette leçon —
et c'est le son qui force ce changement. Le son est la première chose, dans ce
moteur, qui exige que la boucle se réveille *selon un calendrier* ; tout ce qui
précède pouvait dormir entre les nouvelles pour toujours.

Le moteur n'en sait rien, et c'est délibéré. La borne est un état propre à la
couche plateforme. Quand `SubmitSamples` remet des trames au périphérique, le
fichier ALSA note quand le périphérique aura consommé ce qu'il a reçu — une
échéance sur l'horloge de la plateforme, cumulée pour que des soumissions qui se
chevauchent ne puissent que la repousser :

```
deadline = (deadline > now ? deadline : now) + frames_taken / rate
```

`deadline > now` conserve la fin de tout ce qui reste à jouer ; `now` est le
point où la consommation commence quand le périphérique a déjà rattrapé son
retard. Et `AudioWaitSeconds` rapporte le temps restant jusqu'à cette échéance.
Elle vit dans `src/platform_internal.h`, un en-tête explicitement **hors du
contrat de la couture** — `platform.h` reste la vue entière du moteur sur l'OS,
un second OS doit exactement ce que cet en-tête déclare et rien de plus, et rien
dans l'en-tête interne ne nomme un OS. `PumpEvents`, dans le fichier X11,
transforme la réponse en délai d'attente de `poll` : une valeur négative conserve
l'ancienne attente non bornée exactement telle quelle, toute autre valeur borne
`poll`. L'attente se termine plus tôt parce que la sortie a besoin de son
prochain tampon, pas parce qu'il s'est passé quelque chose. Le pliage de
l'interruption, la vidange des événements, tout cela est inchangé.

L'autre moitié du correctif est la nouvelle **étape audio** de la boucle de
frames, et elle est conditionnée : la boucle ne remet au périphérique le prochain
tampon du flux que lorsque le tampon est *dû* — l'exécution tient son propre
calendrier, un horizon par tampon, et une frame qui arrive en avance ne trouve
rien à faire. Une nouvelle d'entrée peut réveiller une frame en avance — une
frame réveillée en avance ne doit pas mettre d'audio supplémentaire en file,
sinon l'exécution ensevelirait le périphérique sous les tampons au lieu de les
cadencer. C'est exactement ce que montre une exécution à entrées scriptées : en
pilotant la fenêtre avec des événements clavier pendant trois secondes, 218
frames ont tourné mais 175 seulement ont alimenté — les 43 autres se sont
réveillées sur une nouvelle et ont consigné `audio 0.000 ms`, la barrière
fermée. Les alimentations sont restées à la cadence de l'horizon (58 par seconde)
quelle que soit l'activité que les entrées imposaient à la boucle. La file reste
profonde d'un tampon ; c'est tout l'intérêt du cadencement.

### La phase audio

La frame paie pour tout cela, donc l'enregistrement de frame accueille un champ :
`FrameRecord.audio`, aux côtés d'`update`, `render` et `present` — l'étape audio
de l'exécution, qui mixe et soumet le flux. `FrameStats` l'additionne comme toute
autre phase, l'étape est chronométrée à chaque frame (elle lit ~0 là où aucun
tampon n'était dû), et la ligne de journal `frame N:` la porte dans l'ordre
propre à l'enregistrement : `update`, `audio`, `render (sprites, text,
tilemap)`, `present`, `total`. À partir de cette leçon, l'arithmétique de clôture
de la leçon 058 a quatre termes et l'enregistrement les porte tous les quatre.

Ce qui ne change délibérément *pas*, c'est la table du budget de frames. Sa
ligne audio appartient à la **leçon 070**, avec les lignes son que cette leçon
fera grandir ; d'ici là, les lignes de la table couvrent
`update + render + present` et non la frame entière, et c'est en 070 que cet
écart se referme. La phase est mesurée dès maintenant précisément pour que la
ligne arrive comme une somme mesurée le moment venu — la même discipline de
mesure anticipée dont les sous-phases de render ont bénéficié.

Le contrôle de frontière de la leçon 042 garde tout ceci honnête, et il imprime
toujours son compte rendu inchangé — `platform_internal.h` n'inclut aucun
en-tête d'OS et n'appelle aucune fonction d'OS :

```
boundary: OK — OS headers and OS calls appear only in src/platform_x11.cpp src/platform_alsa.cpp
boundary: 17 single-line declarations there (multi-line ones are in the header)
```

### Ce que coûte désormais une exécution au repos

Énonçons clairement le compromis, car la conception le consigne comme un risque :
une exécution au repos avec une sortie ouverte **bat à la cadence de l'horizon de
tampon au lieu de dormir entre les nouvelles**. Mesuré ici : une exécution au
repos de trois secondes contre le périphérique `null` a tourné 176 frames — les
frames 1 à 176, environ une toutes les 17 ms face aux 16,7 ms de l'horizon, à peu
près soixante frames par seconde là où la même exécution, avant le son, aurait
tourné une frame puis attendu (une exécution répétée bat à la même cadence avec
175 — la cadence est l'affirmation, pas le compte). Le réveil tombe juste après
l'échéance parce que l'attente arrondit à la milliseconde entière ; l'alimentation
suivante est due immédiatement et le cycle se répète.

C'est le prix à payer pour alimenter le périphérique sans threads : l'exécution
se réveille au rythme de la sortie même quand rien d'autre ne se passe, et paie
une frame d'update/render/present par tampon. Sur cette machine, la frame est bon
marché (~2 ms de travail contre un horizon de 16,7 ms) ; un moteur plus lourd
voudrait de plus gros tampons et un horizon plus long, ce qui est le bouton que
tourne l'exercice 1. Sans sortie ouverte, rien ne change du tout — l'attente est
l'ancienne, non bornée, et l'exécution dort entre les nouvelles exactement comme
avant le son.

### Pas de threads

La loi du langage de la leçon 026 n'a jamais admis les threads, et cette leçon
n'en fait pas passer un en contrebande. C'est une boucle, des appels synchrones,
et de la mémoire venue de l'arena — la borne d'`AudioWaitSeconds` est ce qui rend
cela économiquement possible tout court. Les alternatives évidentes ont été
rejetées pour des raisons plus anciennes que cette leçon : un thread de mixage
demanderait des tampons partagés, un verrou et un second domaine d'horloge —
trois nouveaux modes de défaillance pour acheter un sommeil — et un callback de
périphérique serait pire que coûteux : il **inverserait la frontière**. La
couture est une ligne que le moteur traverse en appelant ; le code du moteur
appelle vers le bas, l'OS répond. Un callback ferait appeler le code du moteur
par l'OS — la plateforme tendant la main vers le moteur, tenant un pointeur vers
l'état du moteur, sur un thread que le moteur n'a pas créé. L'attente cadencée
garde la direction de l'appel exactement là où la leçon 027 l'a posée, et le coût
est celui nommé plus haut.

### Ce que cette exécution a vérifié, et ce qu'elle n'a pas vérifié

Cette machine n'a pas de matériel son, et son périphérique `null` d'ALSA accepte
les échantillons instantanément et les jette — il ne repousse jamais. Voici ce
que cela laisse vérifiable, et qui a été vérifié ici :

- **L'attente est bornée par l'horizon de tampon.** Une exécution au repos avec
  la sortie ouverte bat à la cadence audio au lieu de bloquer pour toujours entre
  les nouvelles : 176 frames en trois secondes, une frame environ toutes les
  17 ms face à l'horizon de 16,7 ms (les numéros de frame et leurs instants
  d'arrivée sont dans le journal de l'exécution — frames 1–176, 0,000 s à
  2,985 s ; une exécution répétée en bat 175, à la même cadence).
- **La barrière cadence les alimentations.** Sous entrées scriptées, l'exécution
  continue de battre et continue d'alimenter — 218 frames, dont 175 alimentations
  à la cadence de l'horizon (58 par seconde), 43 frames réveillées par une
  nouvelle qui n'ont rien mis en file. Aucune rafale d'audio en file.
- **Sans sortie, la borne disparaît.** Pointez l'exécution vers un nom de
  périphérique qui n'existe pas : elle rapporte l'échec typé et continue — et son
  attente redevient non bornée : la même exécution de trois secondes a tourné
  **une** frame (la nouvelle de mappage de la fenêtre au démarrage) puis a
  attendu des nouvelles jusqu'à l'interruption, contre 176 pour l'exécution sur
  `null`. La borne ne s'applique que lorsqu'il y a quelque chose à alimenter.

Ce que cette machine ne peut pas vérifier, c'est l'affirmation pour laquelle
l'attente cadencée existe : **« le périphérique n'a pas subi de famine »** exige
du matériel qui repousse — des échantillons consommés en temps réel, une file
qui se vide, des oreilles qui entendent une saccade quand la boucle est en
retard. `null` accepte tout instantanément et ne prouve que le côté moteur du
calendrier. Le côté périphérique est un territoire de vrai matériel, et c'est
l'exercice 2 qui lui donne son témoin.

## Étape de code

Un seul changement pour cette leçon, du flux au calendrier : `src/frame.h` /
`src/frame.cpp` accueillent la phase `audio` dans l'enregistrement, les sommes et
la ligne de journal (et délibérément pas dans la table du budget) ;
`src/platform_alsa.cpp` accueille la comptabilité d'échéance et
`AudioWaitSeconds` ; `src/platform_internal.h` est le nouvel en-tête interne qui
la partage ; l'attente de `src/platform_x11.cpp` devient bornée par elle ; le
contrat de `PumpEvents` dans `src/platform.h` nomme la borne ; et `src/main.cpp`
remplace la soumission unique par une boucle qui alimente le flux — curseur,
barrière, phase, rapport de démarrage. Le monde de la démo et le chemin de
dessin ne sont pas touchés. Son état final est étiqueté `lesson-060`.

```diff
diff --git a/src/frame.cpp b/src/frame.cpp
index 3f9f2dd..d15549b 100644
--- a/src/frame.cpp
+++ b/src/frame.cpp
@@ -12,6 +12,7 @@ void AccountFrame(FrameStats &stats, const FrameRecord &frame)
 {
     stats.frames += 1;
     stats.update_sum += frame.update;
+    stats.audio_sum += frame.audio;
     stats.render_sum += frame.render;
     stats.present_sum += frame.present;
     stats.total_sum += frame.total;
@@ -39,7 +40,9 @@ void PrintFrameBudget(const FrameStats &stats)
 
     /* The attribution: every row a measured sum, every share of the
        average frame. The named phases live inside render — they say
-       where it went, they do not replace it. */
+       where it went, they do not replace it. The audio phase (lesson
+       060) is summed like the rest but gets no row here: the table's
+       sound rows are lesson 070's, and that is deliberate. */
     double update = stats.update_sum / n * 1e3;
     double render = stats.render_sum / n * 1e3;
     double sprites = stats.sprites_sum / n * 1e3;
diff --git a/src/frame.h b/src/frame.h
index 8a998b4..f4b45e3 100644
--- a/src/frame.h
+++ b/src/frame.h
@@ -14,6 +14,8 @@ namespace engine {
 struct FrameRecord {
     long number;   /* the frame's count since the run started */
     double update; /* reading state, moving the world */
+    double audio;  /* lesson 060: the run's audio step — mixing and
+                      submitting the stream */
     double render; /* drawing the scene into the framebuffer */
     double present;/* the copy to the window, sync included */
     double total;  /* the whole frame step */
@@ -31,6 +33,7 @@ struct FrameRecord {
 struct FrameStats {
     long frames;
     double update_sum;
+    double audio_sum; /* lesson 060's phase, summed like the rest */
     double render_sum;
     double present_sum;
     double total_sum;
diff --git a/src/main.cpp b/src/main.cpp
index a848cad..5044acc 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -29,8 +29,20 @@ namespace engine {
    per-frame step. */
 constexpr double SPRITE_SPEED = 240.0; /* pixels per second */
 
-/* Lesson 059: the run's first sound is half a second of tone. */
-constexpr int TONE_FRAMES = AUDIO_RATE / 2;
+/* Lesson 059: the run's sound is half a second of tone — 22050 frames.
+   One second of a 440 Hz tone is exactly 440 cycles at AUDIO_RATE, so
+   this buffer holds a whole 220 cycles and its end meets its beginning
+   with no click: the wrap lesson 059 found, which lesson 060's feeding
+   cursor leans on when it hands the same run of frames to the device
+   again and again. */
+constexpr int TONE_FRAMES = AUDIO_RATE / 2; /* 22050 */
+
+/* Lesson 060: one buffer of stream per feed — one sixtieth of a second,
+   the horizon the loop keeps queued. The cursor relies on the tone's
+   length dividing evenly into buffers: 22050 / 735 = 30 exactly, so it
+   wraps at a buffer boundary and no feed ever has to copy across the
+   tone's end. */
+constexpr int CHUNK_FRAMES = AUDIO_RATE / 60;   /* 735 */
 
 /* Lesson 054: the scene, drawn through the camera. The camera's summed
    offset is applied once, at each draw's origin — the map's and the
@@ -129,12 +141,16 @@ int Run(void)
     std::printf("engine: arrow keys move the sprite, space shakes the camera; close the window to stop\n");
     std::printf("engine: sprite at %d,%d\n", (int)sprite_x, (int)sprite_y);
 
-    /* Lesson 059: the run's first sound. A sample is a frame of amplitude
-       at the engine's rate — here a tone computed by code instead of read
+    /* Lesson 059: the run's sound. A sample is a frame of amplitude at
+       the engine's rate — here a tone computed by code instead of read
        from a file — and the seam's audio output is what puts those frames
-       in front of a device. */
+       in front of a device. Lesson 060: those frames are a *stream*, not
+       one submission done at startup — the frame loop feeds the device
+       buffer by buffer, for as long as the run lasts. */
     platform::AudioResult audio =
         platform::OpenAudioOutput(AUDIO_RATE, AUDIO_OUTPUT_CHANNELS);
+    short *tone = 0;
+    int tone_cursor = 0; /* where the stream's next buffer starts */
     if (!audio.output) {
         switch (audio.error) {
         case platform::AUDIO_NO_DEVICE:
@@ -148,8 +164,8 @@ int Run(void)
            still runs — this one continues without sound. */
         std::fprintf(stderr, "engine: continuing without sound\n");
     } else {
-        short *tone = (short *)ArenaAlloc(arena, TONE_FRAMES * sizeof(short),
-                                          sizeof(short));
+        tone = (short *)ArenaAlloc(arena, TONE_FRAMES * sizeof(short),
+                                   sizeof(short));
         if (!tone) {
             std::fprintf(stderr, "engine: no room for the tone\n");
         } else {
@@ -163,20 +179,25 @@ int Run(void)
                 std::printf(" %d", (int)tone[i]);
             std::printf("\n");
 
-            if (platform::SubmitSamples(audio.output, tone, TONE_FRAMES))
-                std::printf("engine: tone played\n");
-            else
-                std::fprintf(stderr,
-                             "engine: the output would not take the samples\n");
+            /* And what the loop does with them: one buffer of stream per
+               feed — the horizon the paced wait keeps queued. */
+            std::printf("engine: stream: %d-frame buffers, horizon %.1f ms; the loop feeds one when it is due\n",
+                        CHUNK_FRAMES, 1e3 * CHUNK_FRAMES / AUDIO_RATE);
         }
     }
 
-    /* The frame step: read news, update from polled state, draw, present —
-       every phase measured, one record per frame. */
+    /* The frame step: read news, update from polled state, feed the
+       stream, draw, present — every phase measured, one record per
+       frame. */
     int exit_code = 0;
     long frame_number = 0;
     FrameStats stats = {};
     Camera camera = { 0, 0, 0, 0 };
+
+    /* Lesson 060: the run's own feeding schedule. The device consumes at
+       the engine's rate, so the next buffer is due one horizon from the
+       last one — and the loop knows that without asking the platform. */
+    double next_feed = platform::Now();
     while (!platform::CloseRequested(opened.window)) {
         platform::PumpEvents(opened.window);
         if (platform::CloseRequested(opened.window))
@@ -269,6 +290,35 @@ int Run(void)
         }
 
         frame.update = platform::Now() - t0;
+
+        /* Lesson 060: the audio step — the loop feeds the device the next
+           buffer of the stream, and only when the buffer is due. Input
+           news can wake a frame early; a frame woken early must not queue
+           extra audio, or the run would bury the device in buffers instead
+           of pacing them. The step is measured on every frame — it is ~0
+           where no buffer was due — so the phase accounts for all of the
+           frame's audio work. */
+        double t_audio = platform::Now();
+        if (audio.output && tone && t_audio >= next_feed) {
+            if (platform::SubmitSamples(audio.output, tone + tone_cursor,
+                                        CHUNK_FRAMES)) {
+                tone_cursor = (tone_cursor + CHUNK_FRAMES) % TONE_FRAMES;
+            } else {
+                /* A device that will not take the samples is named once,
+                   not once per frame: the run closes the output and carries
+                   on in silence — its wait unbounded again. */
+                std::fprintf(stderr,
+                             "engine: the output would not take the samples\n");
+                platform::CloseAudioOutput(audio.output);
+                audio.output = 0;
+            }
+            /* The schedule restarts from now, not from the missed slot: a
+               long frame is caught up by one buffer, never by a backlog. */
+            next_feed = platform::Now() +
+                        (double)CHUNK_FRAMES / (double)AUDIO_RATE;
+        }
+        frame.audio = platform::Now() - t_audio;
+
         double t1 = platform::Now();
 
         /* Render: every frame draws the whole scene — clear, the world
@@ -313,9 +363,11 @@ int Run(void)
         AccountFrame(stats, frame);
 
         /* The frame log: one line per record — the format grows its named
-           fields, one per subsystem, as the parts name them. */
-        std::printf("frame %ld: update %.3f ms, render %.3f ms (sprites %.3f, text %.3f, tilemap %.3f), present %.3f ms, total %.3f ms\n",
-                    frame.number, frame.update * 1e3, frame.render * 1e3,
+           fields, one per subsystem, as the parts name them. The audio
+           phase (lesson 060) joins in the record's own order. */
+        std::printf("frame %ld: update %.3f ms, audio %.3f ms, render %.3f ms (sprites %.3f, text %.3f, tilemap %.3f), present %.3f ms, total %.3f ms\n",
+                    frame.number, frame.update * 1e3, frame.audio * 1e3,
+                    frame.render * 1e3,
                     frame.sprites * 1e3, frame.text * 1e3,
                     frame.tilemap * 1e3, frame.present * 1e3,
                     frame.total * 1e3);
diff --git a/src/platform.h b/src/platform.h
index 8e0f1ba..4473e68 100644
--- a/src/platform.h
+++ b/src/platform.h
@@ -117,7 +117,9 @@ FileError WriteFile(const char *path, const unsigned char *data, size_t size);
 
 /* Reads whatever news the OS has about this window and folds it into the
    platform layer's state. The engine never sees an event object — it polls
-   state afterwards. Blocks until there is news or the run is interrupted. */
+   state afterwards. Blocks until there is news or the run is interrupted —
+   or until an open audio output needs its next buffer, whichever comes
+   first. */
 void PumpEvents(Window *window);
 
 /* True once the user has asked for this window to close. An interrupted run
diff --git a/src/platform_alsa.cpp b/src/platform_alsa.cpp
index 59e1fe6..fdb7fb6 100644
--- a/src/platform_alsa.cpp
+++ b/src/platform_alsa.cpp
@@ -9,9 +9,16 @@
 // Which device this run opens is this file's business too: a machine with
 // no usable output reports AUDIO_NO_DEVICE, and a machine that has none
 // still runs.
+//
+// Lesson 060: the deadline bookkeeping. A device consumes samples at its
+// own rate, so this file also knows when the device will have consumed
+// what it was given — the bound the run's paced wait obeys. That number is
+// platform state, shared with the event pump through platform_internal.h
+// and not part of the seam's contract.
 #define _POSIX_C_SOURCE 200809L
 
 #include "platform.h"
+#include "platform_internal.h"
 
 #include <alsa/asoundlib.h>
 
@@ -26,6 +33,10 @@ struct AudioOutput {
     snd_pcm_t *device;
     int engine_channels; /* the seam's frame width, and the device's own */
     int device_channels; /* layout, which this file maps between */
+    int rate;            /* the sample rate it was opened at */
+    double deadline;     /* lesson 060: when the device will have consumed
+                            everything submitted so far — the boundary
+                            AudioWaitSeconds reports */
 };
 
 /* The one output, in static storage: no new, no delete — the language law
@@ -88,6 +99,9 @@ AudioResult OpenAudioOutput(int rate, int channels)
     audio_state.engine_channels = channels;
     audio_state.device = device;
     audio_state.device_channels = DEVICE_CHANNELS;
+    audio_state.rate = rate;
+    /* Nothing is queued yet, so the first buffer is due at once. */
+    audio_state.deadline = 0.0;
     result.output = &audio_state;
     result.error = AUDIO_OK;
     return result;
@@ -126,9 +140,35 @@ bool SubmitSamples(AudioOutput *output, const short *samples, int frames)
         at += (int)took * output->engine_channels;
         left -= (int)took;
     }
+
+    /* Lesson 060: when will the device have consumed what this call gave
+       it? On the platform clock, the answer is the device's own deadline —
+       and it accumulates: a submit made while an earlier one is still
+       playing lands after that backlog, never before it, so overlapping
+       submits can only move the deadline later. `deadline > now` keeps the
+       backlog's end when there is one; `now` is where consumption starts
+       when the device has already caught up (a call that waited for room
+       finds its deadline in the past). */
+    double now = Now();
+    output->deadline = (output->deadline > now ? output->deadline : now) +
+                       (double)frames / (double)output->rate;
     return true;
 }
 
+double AudioWaitSeconds(void)
+{
+    if (!audio_state.device)
+        return -1.0; /* no output to feed: the wait is unbounded, as before
+                        sound — and a negative answer means exactly that */
+
+    /* The time left until the device needs what comes next — zero once the
+       buffer is already due. Zero, and never a negative: zero bounds the
+       wait at once, a negative removes the bound, and confusing the two
+       would make a due buffer sleep instead of feed. */
+    double left = audio_state.deadline - Now();
+    return left > 0.0 ? left : 0.0;
+}
+
 void CloseAudioOutput(AudioOutput *output)
 {
     if (!output || !output->device)
@@ -139,6 +179,7 @@ void CloseAudioOutput(AudioOutput *output)
     snd_pcm_drain(output->device);
     snd_pcm_close(output->device);
     output->device = 0;
+    output->deadline = 0.0; /* a closed output stops bounding the wait */
 }
 
 } /* namespace platform */
diff --git a/src/platform_internal.h b/src/platform_internal.h
new file mode 100644
index 0000000..b55adef
--- /dev/null
+++ b/src/platform_internal.h
@@ -0,0 +1,24 @@
+// platform_internal.h — the platform layer's own state. Not the seam's
+// contract.
+//
+// Lesson 060: platform.h is the engine's whole view of the OS — that header
+// is the seam's contract, and a second OS owes exactly what it declares and
+// nothing more. Nothing here crosses it: no OS header is included, no OS
+// type is named, no device or display handle appears. What lives here is
+// the platform layer's own bookkeeping — how long this platform's event
+// pump may sleep before the sound output needs feeding — shared between the
+// audio implementation and the wait that obeys it. A second OS defines
+// these in its own implementation files, beside its platform.h functions.
+#ifndef PLATFORM_INTERNAL_H
+#define PLATFORM_INTERNAL_H
+
+namespace platform {
+
+/* How long the run may wait before the output needs its next buffer —
+   the bound PumpEvents puts on its wait. Negative when there is no
+   output to feed: the wait is then unbounded, exactly as before sound. */
+double AudioWaitSeconds(void);
+
+} /* namespace platform */
+
+#endif
diff --git a/src/platform_x11.cpp b/src/platform_x11.cpp
index 17dae4e..a251f11 100644
--- a/src/platform_x11.cpp
+++ b/src/platform_x11.cpp
@@ -15,9 +15,15 @@
 // Lesson 037: whole-file reads. File I/O is OS surface too — POSIX here,
 // a second OS's own calls there. Everything in this file is one
 // implementation behind the seam.
+//
+// Lesson 060: the paced wait. The wait for news is no longer unbounded —
+// with an audio output open it is bounded by how long the queued samples
+// will last, so the run wakes to feed the device on schedule. The bound is
+// platform state, asked for through platform_internal.h.
 #define _POSIX_C_SOURCE 200809L
 
 #include "platform.h"
+#include "platform_internal.h"
 
 #include <X11/XKBlib.h>
 #include <X11/Xlib.h>
@@ -357,9 +363,18 @@ void PumpEvents(Window *window)
     /* Wait for news where a signal can wake us. Lesson 028 slept inside
        XNextEvent, where Ctrl+C could not reach it; poll on the OS
        connection returns when there is news *or* when a signal interrupts
-       it — then the flag below is folded in like any other news. */
+       it — then the flag below is folded in like any other news.
+
+       Lesson 060: the wait is bounded. An open audio output needs its next
+       buffer before long, so the wait may not outlast it — and when it
+       ends early it ends because the output needs feeding, not because
+       anything happened. The ceiling to whole milliseconds keeps the wait
+       from ending before the buffer is due; AudioWaitSeconds is negative
+       with no output to feed, and the wait is then the old unbounded one. */
+    double wait = AudioWaitSeconds();
+    int timeout_ms = wait < 0.0 ? -1 : (int)(wait * 1000.0 + 0.999);
     struct pollfd pfd = { ConnectionNumber(window->display), POLLIN, 0 };
-    poll(&pfd, 1, -1);
+    poll(&pfd, 1, timeout_ms);
 
     if (interrupted)
         window->close_requested = true;
```

## Exercices

Deux exercices mixtes. Chacun se termine par sa solution — un diff contre l'état
final de cette leçon, plus une visite guidée — après l'énoncé.

### Exercice 1 — L'horizon, doublé *(measure-the-performance)*

L'exécution garde exactement un tampon de flux en file — `CHUNK_FRAMES`, un
soixantième de seconde — et une exécution au repos avec la sortie ouverte bat à
la cadence de cet horizon. Élargissez l'horizon : faites de chaque alimentation
un **trentième** de seconde de flux au lieu d'un soixantième. À partir
d'exécutions de même durée, mesurez avant et après : la cadence des frames au
repos (frames et écarts), et le coût propre de la phase `audio` sur une frame qui
alimente contre une frame qui n'a rien alimenté. Réconciliez la cadence avec
l'arithmétique de l'horizon, et vérifiez que le retournement du curseur n'exige
toujours aucune copie scindée — que vaut `TONE_FRAMES / CHUNK_FRAMES` maintenant ?
Rapportez les nombres de vos exécutions.

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-060/ex1.md)

### Exercice 2 — La famine sur de vrais haut-parleurs *(port-to-your-own-machine)*

Toutes les vérifications de cette leçon ont tourné contre le périphérique `null`
d'ALSA — le périphérique qui accepte les échantillons instantanément et ne
repousse jamais. « Le périphérique n'a pas subi de famine » est donc exactement
l'affirmation que cette machine ne peut pas contrôler, et c'est l'affirmation
pour laquelle l'attente cadencée existe. Emmenez la démo sur une machine dotée de
vrai matériel son et répondez à la question que `null` ne peut pas trancher :
quand l'exécution s'active — le sprite conduit dans un mur, la caméra qui
tremble, la frame à son plus lourd — le flux tient-il, ou entendez-vous des
saccades ? Faites consigner par l'exécution la vigueur avec laquelle le
périphérique a repoussé pendant la lecture, pour que le journal porte la preuve à
côté de vos oreilles ; puis rapportez le périphérique, l'exécution chargée contre
une au repos, ce que vous avez entendu, et où, dans la comptabilité d'échéance,
une saccade se serait manifestée en premier.

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-060/ex2.md)

---

**Partie :** [Partie 3 — le son](../../index.md) ·
**Précédente :** [Leçon 059 — le son comme échantillons](lesson-059-samples.md) ·
**Suivante :** [Leçon 061 — le conteneur WAV](lesson-061-wav.md) ·
**Étiquette de code :** [`lesson-060`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-060)

*Page traduite de la version anglaise `book/lessons/part-3/lesson-060-stream.md`, révision `f3ae17f`.*

<!-- translation-source: book/lessons/part-3/lesson-060-stream.md @ f3ae17f -->
