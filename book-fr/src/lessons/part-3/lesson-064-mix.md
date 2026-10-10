# Leçon 064 — le mixage

{{#include ../../stability-horizon.md}}

## Prose

La leçon 063 a donné au moteur un canal : ce qui joue, où il en est, à quel
volume. Un canal remplit un tampon, et le tampon *est* le flux. Cette leçon
ajoute le grand nombre, et l'idée tient en une phrase : **plusieurs canaux, un
seul flux** — chaque trame de sortie est la somme de la trame suivante de chaque
canal actif à son volume, écrêtée à la plage du format au lieu d'y être
retournée. La leçon du milieu du lot consacré au mixeur est la somme elle-même ;
la leçon 065 distribue les canaux.

### Le mixage, à chaque trame de sortie

`MixBuffer` dans `src/audio.cpp`, ce sont deux boucles. La boucle externe
parcourt les trames de sortie ; dedans, un accumulateur 32 bits — `int sum` —
recueille la trame suivante de chaque canal actif à son volume, un tirage par
canal. Puis l'écrêtage, et la trame est écrite en `short`. Tout le mixage est là :
tirer, additionner, écrêter, écrire — `frame_count` fois.

L'exécution rend la somme vérifiable à la main. Deux canaux jouent la même
tonalité — celle de la leçon 059, depuis `assets/tone.wav` — aux volumes 128 et
64 sur 256, et le démarrage imprime les premières trames de l'échantillon à côté
des premières trames du mixage :

```
engine: sample: 22050 frames at 44100 Hz, 1 channel, first frames: 0 513 1024 1531 2032 2525 3009 3480
engine: mix: channel 0 at volume 128, channel 1 at volume 64 of 256
engine: mix: first frames (summed): 0 384 768 1147 1524 1893 2256 2610
```

Chaque trame mixée est deux contributions additionnées : 513 devient 256 au
volume 128 et 128 au volume 64 — 384. 2525 devient 1262 et 631 — 1893. Regardez
1531 une fois encore : ses contributions sont 765 et 382, et le mixage dit
**1147**. Mettre la trame à l'échelle une seule fois à 192 sur 256 aurait imprimé
1148. Les canaux tronquent **avant** la somme, chaque contribution pour
elle-même — le mixage additionne ce que les canaux ont émis, pas ce qu'ils
auraient pu émettre en une seule étape, et le nombre de l'exécution dit quel
ordre a été utilisé.

### Écrêté, jamais retourné

Une somme de canaux forts sort du format. Deux canaux à pleine échelle, émettant
chacun 20000, en produisent 40000 à eux deux, et `short` n'a pas la place pour
ça. Ce qui suit est la distinction la plus importante de la leçon. Un
**retournement (wrap)** tronque la somme à 16 bits et la laisse rentrer par
l'autre côté : 40000 devient −25536, la crête la plus forte du son se change en
trou profond — le pire artefact en audio. Un **écrêtage (clamp)** atterrit sur la
limite : 40000 devient 32767, le plus fort que le format puisse dire. Écrêté,
jamais retourné — c'est le scénario de la spécification elle-même pour le
mixage. L'écrêtage, ce sont deux limites, dans les mêmes nombres que ceux des
échantillons : `SAMPLE_LIMIT_HI` à 32767, `SAMPLE_LIMIT_LO` à −32768.

L'écrêtage a été vérifié à travers le `MixBuffer` du moteur lui-même — une sonde
d'essai pilotant un vrai `Mixer`, `ChannelPlay` et `MixBuffer`, qui ne fait pas
partie du dépôt — additionnant des trames connues et relisant la sortie :

```
one channel at the tone's peak             sum   8191 -> out   8191
four channels at the peak                  sum  32764 -> out  32764
five channels at the peak (over)           sum  40955 -> out  32767
five channels at the trough                sum -40955 -> out -32768
three at full scale positive               sum  98301 -> out  32767
15 idle channels change nothing            byte-identical
```

La crête de la tonalité est 8191 parce que `assets/tone.wav` est la tonalité de
la leçon 059 à l'amplitude 0,25. Quatre canaux à cette crête atterrissent sur
32764 — sous la limite de trois unités, et c'est encore la vraie somme. Le
cinquième est ce qui force l'écrêtage. Et le creux s'écrête à −32768, pas vers
une valeur positive : l'écrêtage tient chaque côté de la plage de son propre
côté.

### Pourquoi la somme tient dans 32 bits

L'écrêtage ne peut faire son travail que s'il voit la vraie somme, donc
l'accumulateur fait 32 bits de large exprès. Seize canaux d'échantillons 16 bits
culminent à 16 × 32768 = 524288 — loin à l'intérieur d'un `int`, quoi que les
volumes en fassent. Aucun intermédiaire ne peut se retourner en chemin vers
l'écrêtage ; l'écrêtage est la seule chose entre la somme et le format, et le
transtypage vers `short` après lui est sûr exactement parce que l'écrêtage est
passé en premier.

C'est aussi ici que la dernière section de la leçon 063 porte ses fruits. Un
canal seul ne peut pas déborder du format — le volume ne dépasse jamais
`AUDIO_VOLUME_FULL`, donc un canal émet au plus l'échantillon qu'il joue. Seule
la somme des canaux peut pousser une trame hors de la plage. C'est pourquoi
cette leçon a l'écrêtage et que la leçon 063 n'en avait pas : écréter n'est pas
un problème de canal, c'est celui du mixage.

### Le silence n'apporte rien

La plupart des canaux sont silencieux la plupart du temps — pas encore démarrés,
ou joués jusqu'à leur fin. Un canal au repos n'apporte **rien du tout** : son
tirage n'ajoute rien à l'accumulateur, et le mixage écrit le flux exactement une
fois par trame de sortie, à partir de la somme. C'est différent d'écrire des
zéros. Le remplissage de la leçon 063 écrivait `0` dans chaque trame de son
tampon au-delà de la fin de l'échantillon ; ici rien n'est jamais écrit au nom
d'un canal — le silence est l'absence d'une contribution, pas une valeur. La
vérification est comportementale et exacte : quinze canaux au repos à côté d'un
canal qui joue produisent un flux identique octet pour octet.

### Le remplissage est devenu un tirage

Dites-le à voix haute, parce que c'est le vrai geste de la leçon. Le
`ChannelFill` de la leçon 063 remplissait tout le tampon d'un canal — le canal
écrivait sa sortie, trame par trame, dans `out`. Un mixage ne peut pas
fonctionner ainsi : additionner trame de sortie par trame de sortie a besoin de
la **seule** trame suivante de chaque canal, toutes au même instant. Alors
`ChannelFill` est devenu `ChannelFrame` — désormais privé à `src/audio.cpp` — et
le sens de l'appel s'est inversé. Un canal ne remplit plus un tampon ; le mixage
**tire** une trame de chaque canal et les additionne. La struct est intacte ; la
forme de ce qu'elle fait avec ne l'est pas — la même habitude que les
refactorisations précédentes : l'unité n'a pas changé, le code a rattrapé ce
qu'il devait devenir.

### Seize canaux, décidés d'avance

`Mixer` est seize copies du `Channel` de la leçon 063 dans un seul tableau —
`AUDIO_MIXER_CHANNELS = 16`, une struct plate de valeurs plates. Le compte est
fixé à la compilation et `MixerInit` le parcourt une fois, chaque canal au repos
et libre. Rien n'est alloué pendant que le son joue : aucun canal n'est créé
quand un son démarre, aucun n'est détruit quand un son se termine. C'est
l'habitude de l'arena appliquée au mixage — **la capacité est une décision, pas
un événement** — et le nombre seize est exactement cette décision, prise
d'avance. Comment une exécution chargée répartit les seize est l'affaire de la
leçon 065 ; ici ils ne sont qu'un pool, et le mixage ne se soucie pas de savoir
lesquels jouent.

### Ce que cette exécution a vérifié, et ce qu'elle n'a pas vérifié

Tous les nombres proviennent d'exécutions réelles de l'état final de cette leçon
sur cette machine, face au périphérique `null` d'ALSA :

- **Le mixage additionne à l'octet près.** Les trames des deux canaux — 384,
  768, 1147, 1524, 1893, 2256, 2610 — sont chaque contribution tronquée puis
  additionnée, chiffre pour chiffre, à côté des trames de l'échantillon
  lui-même.
- **L'écrêtage atterrit sur la limite, et les canaux au repos ne changent
  rien.** La sonde ci-dessus a poussé le `MixBuffer` du moteur lui-même en
  surcharge délibérée : 40955 devient 32767, −40955 devient −32768, et les
  sommes qui tiennent — 8191 et 32764 — passent intactes. Quinze canaux
  silencieux à côté d'un qui joue : identiques octet pour octet.
- **L'échantillon se termine toujours là où il s'est toujours terminé.** La
  ligne de fin est celle de la leçon 062, inchangée par le mixage :

  ```
  engine: sample: 22050 frames fed in 30 buffers — the sample's end; the stream is silence from here
  ```

- **Le build porte exactement un avertissement :** `DrawScene` défini mais non
  utilisé dans `src/main.cpp` — la verrue de la partie 2 reportée à dessein —
  et cette leçon n'en ajoute aucun. `tools/check-boundary.sh` passe toujours :
  le mixage est du code moteur et ne fait aucun appel à l'OS.

Ce que cette exécution ne peut pas vérifier, c'est la part qui demande des
oreilles : aucun haut-parleur n'a émis de son — `null` prend les échantillons et
les jette. « Deux sons à la fois » est une affirmation sur votre machine et vos
oreilles. Ce que cette leçon affirme, c'est que les trames ont été additionnées
et écrêtées, et l'exécution les imprime.

## Étape de code

Une modification pour cette leçon, trois fichiers : `src/audio.h` et
`src/audio.cpp` font grandir le mixeur — `Mixer`, `MixerInit`, `MixBuffer`, et
le pool `AUDIO_MIXER_CHANNELS` — et le `ChannelFill` de la leçon 063 devient
`ChannelFrame`, le tirage privé, trame par trame, que le mixage additionne.
`src/main.cpp` joue la tonalité sur deux canaux aux volumes 128 et 64 et imprime
la somme à la main, la même arithmétique que celle que le mixage exécute.
L'horizon, l'attente cadencée et la barrière de l'alimentation sont ceux de la leçon 060,
intacts. Son état final est étiqueté `lesson-064`.

```diff
diff --git a/src/audio.cpp b/src/audio.cpp
index eaf730b..58c3af8 100644
--- a/src/audio.cpp
+++ b/src/audio.cpp
@@ -195,25 +195,62 @@ void ChannelPlay(Channel &channel, const Sample &sample, int volume)
     channel.active = true;
 }
 
-void ChannelFill(Channel &channel, short *out, int frame_count)
+namespace {
+
+/* One channel's next output frame at its volume — and the cursor moves.
+   An idle channel, or one whose sample has ended, contributes nothing:
+   silence is not a value here, it is the absence of a contribution. */
+int ChannelFrame(Channel &channel)
+{
+    if (channel.active && channel.sample &&
+        channel.cursor < channel.sample->frame_count) {
+        /* One sample frame, scaled to the channel's volume. A frame is
+           sample.channels values wide; the engine's stream is one
+           channel wide, so it takes the frame's first value. */
+        int frame = channel.sample->frames[channel.cursor *
+                                           channel.sample->channels];
+        channel.cursor += 1;
+        return (frame * channel.volume) / AUDIO_VOLUME_FULL;
+    }
+
+    /* The sample's end — the fact frame_count carries. */
+    channel.active = false;
+    return 0;
+}
+
+/* The format's range, in the same numbers the samples use. */
+constexpr int SAMPLE_LIMIT_HI = 32767;
+constexpr int SAMPLE_LIMIT_LO = -32768;
+
+} /* namespace */
+
+void MixerInit(Mixer &mixer)
+{
+    for (int c = 0; c < AUDIO_MIXER_CHANNELS; ++c) {
+        mixer.channels[c].sample = 0;
+        mixer.channels[c].cursor = 0;
+        mixer.channels[c].volume = 0;
+        mixer.channels[c].active = false;
+    }
+}
+
+void MixBuffer(Mixer &mixer, short *out, int frame_count)
 {
     for (int i = 0; i < frame_count; ++i) {
-        if (channel.active && channel.sample &&
-            channel.cursor < channel.sample->frame_count) {
-            /* One sample frame, scaled to the channel's volume. A frame
-               is sample.channels values wide; the engine's stream is one
-               channel wide, so it takes the frame's first value. */
-            int frame =
-                channel.sample->frames[channel.cursor *
-                                       channel.sample->channels];
-            out[i] = (short)((frame * channel.volume) / AUDIO_VOLUME_FULL);
-            channel.cursor += 1;
-        } else {
-            /* The sample's end — the fact frame_count carries. Silence
-               from here, and the channel is free again. */
-            channel.active = false;
-            out[i] = 0;
-        }
+        /* The 32-bit accumulator for this output frame: sixteen channels
+           of 16-bit samples cannot overflow it, so the clamp below sees
+           the true sum and not a wrapped one. */
+        int sum = 0;
+        for (int c = 0; c < AUDIO_MIXER_CHANNELS; ++c)
+            sum += ChannelFrame(mixer.channels[c]);
+
+        /* Clamped, never wrapped: a sum past the range lands on the
+           limit rather than jumping to the opposite extreme. */
+        if (sum > SAMPLE_LIMIT_HI)
+            sum = SAMPLE_LIMIT_HI;
+        if (sum < SAMPLE_LIMIT_LO)
+            sum = SAMPLE_LIMIT_LO;
+        out[i] = (short)sum;
     }
 }
 
diff --git a/src/audio.h b/src/audio.h
index aed24e6..2b33c0f 100644
--- a/src/audio.h
+++ b/src/audio.h
@@ -86,11 +86,24 @@ struct Channel {
    frame. Playing on one channel leaves every other channel alone. */
 void ChannelPlay(Channel &channel, const Sample &sample, int volume);
 
-/* Fills `out` with `frame_count` frames of this channel's output: the
-   sample's frames at the channel's volume, one for one, advancing the
-   cursor. Past the sample's end the channel writes silence and is
-   inactive again — frame_count is the fact that says when. */
-void ChannelFill(Channel &channel, short *out, int frame_count);
+/* The mixer's fixed set of channels. */
+constexpr int AUDIO_MIXER_CHANNELS = 16;
+
+/* The mixer: one fixed pool of channels, decided up front — nothing is
+   allocated while sound plays. */
+struct Mixer {
+    Channel channels[AUDIO_MIXER_CHANNELS];
+};
+
+/* Every channel idle and free. */
+void MixerInit(Mixer &mixer);
+
+/* The mix: `frame_count` frames of stream, each one the sum of every
+   active channel's next frame at its volume, clamped to the format's
+   range. Clamped, never wrapped — a sum past the range lands on the
+   limit instead of jumping to the opposite extreme. Idle and ended
+   channels contribute nothing at all. */
+void MixBuffer(Mixer &mixer, short *out, int frame_count);
 
 } /* namespace engine */
 
diff --git a/src/main.cpp b/src/main.cpp
index 732ac22..25ce44c 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -162,20 +162,23 @@ int Run(void)
         std::printf(" %d", (int)sample.frames[i]);
     std::printf("\n");
 
-    /* Lesson 063: the sample starts playing on one channel, at half
-       volume — the fixed-point scale is 0-256, so 128 is half. The
-       scaling is checkable byte for byte: at half volume every frame the
-       channel emits is the sample's frame halved, truncated. */
-    Channel channel = {};
-    ChannelPlay(channel, sample, AUDIO_VOLUME_FULL / 2);
-    std::printf("engine: channel: playing %d frames at volume %d of %d\n",
-                sample.frame_count, channel.volume, AUDIO_VOLUME_FULL);
-    std::printf("engine: channel: first frames at that volume:");
-    for (int i = 0; i < 8 && i < sample.frame_count; ++i)
+    /* Lesson 064: two channels playing the same sample at different
+       volumes, so the mix is a real sum and the numbers are checkable —
+       each output frame is the two contributions added, then clamped. */
+    Mixer mixer;
+    MixerInit(mixer);
+    ChannelPlay(mixer.channels[0], sample, AUDIO_VOLUME_FULL / 2);
+    ChannelPlay(mixer.channels[1], sample, AUDIO_VOLUME_FULL / 4);
+    std::printf("engine: mix: channel 0 at volume %d, channel 1 at volume %d of %d\n",
+                mixer.channels[0].volume, mixer.channels[1].volume,
+                AUDIO_VOLUME_FULL);
+    std::printf("engine: mix: first frames (summed):");
+    for (int i = 0; i < 8 && i < sample.frame_count; ++i) {
+        int v = sample.frames[i * sample.channels];
         std::printf(" %d",
-                    (int)(sample.frames[i * sample.channels] *
-                          channel.volume) /
-                        AUDIO_VOLUME_FULL);
+                    (v * mixer.channels[0].volume) / AUDIO_VOLUME_FULL +
+                        (v * mixer.channels[1].volume) / AUDIO_VOLUME_FULL);
+    }
     std::printf("\n");
 
     double sprite_x = 312.0, sprite_y = 232.0;
@@ -342,22 +345,22 @@ int Run(void)
            where no buffer was due — so the phase accounts for all of the
            frame's audio work.
 
-           Lesson 063: the stream is the channel's output. One buffer is
-           what the channel produces — its sample's frames at its volume,
-           and silence past the sample's end. frame_count is the fact that
-           says when the sample ends; the channel never runs past it. */
+           Lesson 064: the stream is the mix. One buffer is every active
+           channel's next frames summed and clamped — silence where no
+           channel has anything to say. frame_count is the fact that says
+           when a sample ends; no channel ever runs past it. */
         double t_audio = platform::Now();
         if (audio.output && t_audio >= next_feed) {
-            int before = channel.cursor;
-            ChannelFill(channel, stream, CHUNK_FRAMES);
-            int take = channel.cursor - before;
+            int before = mixer.channels[0].cursor;
+            MixBuffer(mixer, stream, CHUNK_FRAMES);
+            int take = mixer.channels[0].cursor - before;
 
             if (platform::SubmitSamples(audio.output, stream,
                                         CHUNK_FRAMES)) {
                 sample_fed += take;
                 if (take > 0)
                     sample_feeds += 1;
-                if (!sample_end_named && !channel.active) {
+                if (!sample_end_named && !mixer.channels[0].active) {
                     /* The end, named in the sample's own numbers: what was
                        fed before silence, and how many buffers carried it. */
                     sample_end_named = true;
```

## Exercices

Deux exercices mixtes. Chacun se termine par sa solution — un diff contre l'état
final de cette leçon plus une visite guidée — après l'énoncé.

### Exercice 1 — Cinq canaux à la crête *(predict-the-output)*

Les deux canaux de l'exécution culminent à 6142 — l'écrêtage est la seule pièce
du mixage que cette exécution ne montre jamais à l'œuvre. Placez cinq canaux à
`AUDIO_VOLUME_FULL` sur la tonalité et prédisez, avant de rien lancer : les huit
premières trames mixées au chiffre près, et si l'écrêtage en change ne serait-ce
qu'une ; la trame de crête de la tonalité et sa trame de creux — la somme de
chacune et ce que le flux contient là — et ce qu'un accumulateur qui se retourne
(la somme tronquée aux 16 bits du format) mettrait à la place dans le flux à ces
deux trames ; la plus grande somme qui passe encore l'écrêtage sans être
touchée ; et quelle trame de l'échantillon l'écrêtage change en premier, avec
combien de ses 22050 trames il en change en tout. Donnez à l'exécution une
sonde — tout l'échantillon à travers le `MixBuffer` du moteur lui-même sur un
mixeur d'essai, cinq canaux à plein volume, les trames que l'écrêtage change
nommées et comptées — et réconciliez chaque nombre avec la somme, le seuil et
l'écrêtage. Terminez par la frontière : un échantillon d'amplitude 1,0 — sa
crête 32767 — combien de canaux à plein volume une crête porte-t-elle avant que
l'écrêtage s'enclenche, et que fait un canal seul ?

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-064/ex1.md)

### Exercice 2 — Ce que coûte le mixage *(measure-the-performance)*

Le mixage parcourt les seize canaux à chaque trame de sortie, qu'un seul d'entre
eux joue ou non — le pool est fixe, et le parcours l'est aussi. Mesurez ce que
cela coûte à mesure que des canaux s'ajoutent : chronométrez `MixBuffer` seul
avec 0, 1, 2, 4, 8 et 16 canaux actifs — assez de tampons par configuration pour
voir au-delà de la granularité de l'horloge — et rapportez le coût par tampon et
le coût par trame de sortie à chaque compte. Mettez les nombres en face du
budget de frame : la frame de 16,7 ms, et la phase audio que le journal de
frames rapporte lors d'une alimentation. Dites ce que les mesures montrent — quelle part
du mixage grandit avec le nombre de canaux actifs et quelle part ne grandit
pas — et ce que votre sonde doit faire pour garder les canaux en train de jouer
pendant qu'elle chronomètre. Terminez par ce qu'une sonde comme celle-ci ne peut
pas cacher sur elle-même.

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-064/ex2.md)

---

**Partie :** [Partie 3 — le son](../../index.md) ·
**Précédente :** [Leçon 063 — un canal](lesson-063-channel.md) ·
**Suivante :** [Leçon 065 — l'allocation des canaux](lesson-065-allocation.md) ·
**Étiquette de code :** [`lesson-064`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-064)

*Page traduite de la version anglaise `book/lessons/part-3/lesson-064-mix.md`,
révision `36cc491`.*

<!-- translation-source: book/lessons/part-3/lesson-064-mix.md @ 36cc491 -->
