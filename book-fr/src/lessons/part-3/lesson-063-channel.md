# Leçon 063 — un canal

{{#include ../../stability-horizon.md}}

## Prose

La leçon 062 jouait l'échantillon avec l'état de lecture dispersé dans toute
l'exécution : un curseur à côté de la boucle, un remplissage du tampon de flux,
une soustraction et un minimum pour le borner. Cette leçon nomme l'unité que
ces morceaux formaient déjà et la déplace dans `src/audio.h` : **un canal est
l'unité de lecture** — ce qui joue, où il en est dans l'échantillon, et à quel
volume. C'est une petite struct de valeurs simples, pas un périphérique. La
série du mixeur commence ici : les leçons 063 à 065 font grandir cette unique
struct jusqu'à un mixage de nombreux canaux et un pool qui les distribue.

### Quatre valeurs simples

`Channel` dans `src/audio.h` est quatre champs : `sample` — ce qu'il joue, ou
0 ; `cursor` — où il en est dans l'échantillon, la prochaine trame à lire ;
`volume` — le niveau sonore, sur l'échelle fixée plus bas ; `active` — en train
de jouer.

Le curseur mérite d'être lu deux fois. C'est une position en **trames
d'échantillons** — pas en octets, pas en secondes. `ChannelFill` l'avance d'une
trame par trame de sortie, et une trame fait `sample.channels` valeurs de
large, le pas que le chargeur a vérifié à la leçon 061. Du seul nombre du
curseur, on peut dire quelle part de l'échantillon a été dépensée.

Les deux fonctions sont le reste du canal. `ChannelPlay`, c'est tout « démarrer
un son » : quatre affectations — l'échantillon, le curseur à sa première trame,
le volume, le drapeau actif. Aucun périphérique n'est touché, rien n'est alloué,
et l'appel n'écrit qu'à travers la référence d'un seul canal — démarrer la
lecture sur un canal ne peut déranger aucun autre canal, car il n'existe aucun
chemin de l'appel vers un autre. `ChannelFill` est le remplissage
d'alimentation de la leçon 062, avec l'état dans la struct : tant que le canal
joue, il écrit les trames de l'échantillon mises à l'échelle par le volume, une
pour une, le curseur avançant ; passé la fin de l'échantillon, du silence.

### Le volume en virgule fixe

`AUDIO_VOLUME_FULL` vaut 256. L'échelle va de 0 à 256 — 0 est le silence, 256
la pleine échelle, et 128 exactement la moitié de l'échelle. La mise à l'échelle
est une seule expression, en arithmétique entière : `(frame * volume) /
AUDIO_VOLUME_FULL`.

Les nombres de l'exécution le montrent à l'œuvre. Les premières trames de
l'échantillon sont 0 513 1024 1531 2032 2525 3009 3480 ; le canal, démarré au
volume 128, rapporte ce qu'il émet :

```
engine: channel: playing 22050 frames at volume 128 of 256
engine: channel: first frames at that volume: 0 256 512 765 1016 1262 1504 1740
```

Chaque trame émise est la trame de l'échantillon × 128 / 256, tronquée vers
zéro : 513 devient 256 (à partir de 256,5), 1531 devient 765 (à partir de
765,5), 2525 devient 1262, 3009 devient 1504. Les moitiés exactes — 1024
devient 512, 2032 devient 1016, 3480 devient 1740 — sont la même arithmétique
qui tombe sur des entiers : la moitié de l'échelle est exacte, la moitié d'une
trame est tronquée.

Pourquoi des entiers. C'est la décision de conception D5 de la partie : le
mixage doit être une arithmétique qu'un apprenant peut suivre octet par octet.
Un volume en `double` est l'alternative rejetée — même comportement (513 × 0,5
fait 256,5 là aussi), une question de format de plus : ce qu'est la fraction en
binaire, ce que le transtypage vers `short` en fait. Les entiers gardent tout le
calcul à `(frame * volume) / 256`, et le papier et la machine sont d'accord.

### La fin d'un échantillon

`frame_count` est le fait qui met fin à la lecture — le même fait qui bornait
le remplissage de la leçon 062. `ChannelFill` écrit les trames de l'échantillon
tant que le curseur est à l'intérieur de l'échantillon ; la première trame de
sortie après la fin prend l'autre branche : du silence dans le tampon, et le
canal redevient **inactif**. Le « redevient » est tout l'intérêt — le canal
retourne à l'état dans lequel `ChannelPlay` l'a trouvé, et un canal dans cet
état est libre. C'est ce qui rend un canal réutilisable, et c'est exactement ce
sur quoi s'appuie le pool de la leçon 065 : inactif signifie que le canal peut
porter un autre son.

La trame exacte où cela se produit n'est pas la dernière trame de l'échantillon,
et l'écart mérite un regard attentif. Le remplissage qui dépense la dernière
trame de l'échantillon laisse le canal *encore actif* — son curseur est posé à
`frame_count`, et rien ne l'a encore regardé. Le regard a lieu à la première
trame de sortie du remplissage suivant, où la condition du curseur échoue.
L'arithmétique de cet asset est ronde — 30 tampons de 735 trames font 22050 —
si bien que la fin s'observe à la première trame du remplissage d'après le
trentième ; le fichier court de la leçon 062 (1000 trames) se terminerait au
milieu d'un remplissage, à la 266e trame du deuxième tampon. L'invariant tient
dans les deux cas : **le canal redevient inactif à la première trame de sortie
après la fin de l'échantillon, où qu'elle tombe.** Et ce que l'exécution
rapporte — `22050 frames fed in 30 buffers` — compte ce que le curseur a
dépensé, le compte propre de l'échantillon quelles que soient les frontières de
tampons.

### Un seul canal ne peut pas déborder le format

Regardez encore la trame émise : `(frame * volume) / 256`, avec un volume au
plus `AUDIO_VOLUME_FULL`. À pleine échelle, l'échantillon passe inchangé ; tout
volume inférieur ne fait que l'assourdir. La sortie d'un seul canal ne peut pas
quitter −32768..32767 — pas même aux extrêmes de l'échantillon : −32768 × 256 /
256 fait −32768, et la multiplication intermédiaire (qui culmine près de 8,4
millions) reste loin à l'intérieur d'un `int`.

Voilà pourquoi cette leçon n'a pas d'écrêtage. L'écrêtage est un problème de
*sommation* : deux canaux à pleine échelle, émettant chacun 20000, produisent
40000 à eux deux, et le format n'a pas de place pour ça. Seule la somme des
canaux peut dépasser la plage — et c'est exactement pourquoi l'écrêtage arrive
avec le mixage, à la leçon 064, et pas une leçon avant.

La garantie repose sur une hypothèse : le volume reste dans
0..`AUDIO_VOLUME_FULL`. Un canal à 512 émettrait deux fois l'échantillon et
quitterait le format à lui seul. `ChannelPlay` n'impose pas la plage — l'échelle est le contrat, tenu par
qui fixe un volume, comme les champs de `Sample` sont tenus par le refus du
chargeur.

### Ce que cette exécution a vérifié, et ce qu'elle n'a pas vérifié

Tous les nombres proviennent d'exécutions réelles de l'état final de cette
leçon sur cette machine, face au périphérique `null` d'ALSA :

- **Le canal démarre, le dit, et met à l'échelle à l'octet près.** Les lignes
  de démarrage nomment l'état du canal — `playing 22050 frames at volume 128
  of 256` — et les trames qu'il émet, 0 256 512 765 1016 1262 1504 1740, sont
  `(frame * 128) / 256` tronquées, chiffre pour chiffre.
- **Le canal dépense exactement l'échantillon.** La ligne de fin atterrit avec
  les nombres propres de l'échantillon :

  ```
  engine: sample: 22050 frames fed in 30 buffers — the sample's end; the stream is silence from here
  ```

  30 × 735 = 22050 = `frame_count`, comptés désormais sur le curseur du canal.
- **Le build porte exactement un avertissement :** `DrawScene` défini mais non
  utilisé dans `src/main.cpp` — la verrue de la partie 2 reportée à dessein —
  et cette leçon n'en ajoute aucun. `tools/check-boundary.sh` passe toujours :
  le canal est du code moteur et ne pose aucune question à l'OS.

Ce que cette exécution ne peut pas vérifier, c'est la part qui demande des
oreilles : aucun haut-parleur n'a émis de son — `null` prend les échantillons
et les jette. « Deux fois moins fort que la leçon 062 » est une affirmation sur
votre machine et vos oreilles. Ce que cette leçon affirme, c'est que les trames
ont été divisées par deux, et l'exécution les imprime.

## Étape de code

Une modification pour cette leçon, trois fichiers : `src/audio.h` et
`src/audio.cpp` font grandir le canal — `Channel`, `ChannelPlay`, `ChannelFill`
et l'échelle `AUDIO_VOLUME_FULL` — et `src/main.cpp` joue l'échantillon sur un
canal plutôt que de l'alimenter inline. L'alimentation remplit le flux depuis
`ChannelFill` ; la part de l'échantillon dépensée se lit sur le curseur du
canal ; la fin est nommée quand le canal redevient inactif. La barrière,
l'attente cadencée et l'horizon sont ceux de la leçon 062, intacts. Son état
final est étiqueté `lesson-063`.

```diff
diff --git a/src/audio.cpp b/src/audio.cpp
index 82223bd..eaf730b 100644
--- a/src/audio.cpp
+++ b/src/audio.cpp
@@ -187,4 +187,34 @@ SampleResult LoadSample(Arena &arena, const char *path)
     return result;
 }
 
+void ChannelPlay(Channel &channel, const Sample &sample, int volume)
+{
+    channel.sample = &sample;
+    channel.cursor = 0;
+    channel.volume = volume;
+    channel.active = true;
+}
+
+void ChannelFill(Channel &channel, short *out, int frame_count)
+{
+    for (int i = 0; i < frame_count; ++i) {
+        if (channel.active && channel.sample &&
+            channel.cursor < channel.sample->frame_count) {
+            /* One sample frame, scaled to the channel's volume. A frame
+               is sample.channels values wide; the engine's stream is one
+               channel wide, so it takes the frame's first value. */
+            int frame =
+                channel.sample->frames[channel.cursor *
+                                       channel.sample->channels];
+            out[i] = (short)((frame * channel.volume) / AUDIO_VOLUME_FULL);
+            channel.cursor += 1;
+        } else {
+            /* The sample's end — the fact frame_count carries. Silence
+               from here, and the channel is free again. */
+            channel.active = false;
+            out[i] = 0;
+        }
+    }
+}
+
 } /* namespace engine */
diff --git a/src/audio.h b/src/audio.h
index f05c1b0..aed24e6 100644
--- a/src/audio.h
+++ b/src/audio.h
@@ -67,6 +67,31 @@ struct SampleResult {
    what the engine keeps is its copy, and a refused load keeps nothing. */
 SampleResult LoadSample(Arena &arena, const char *path);
 
+/* Volume in fixed point: AUDIO_VOLUME_FULL is full scale and 0 is
+   silence. The scale runs 0-256 so the mix is integer arithmetic a
+   learner can follow byte for byte. */
+constexpr int AUDIO_VOLUME_FULL = 256;
+
+/* Lesson 063: one channel — the unit of playback. What it is playing,
+   where it is in the sample, and how loud: three plain values, not a
+   device. A channel plays its sample to its end and then is free again. */
+struct Channel {
+    const Sample *sample; /* what it is playing, or 0 */
+    int cursor;           /* the next sample frame to read */
+    int volume;           /* 0..AUDIO_VOLUME_FULL, fixed point */
+    bool active;          /* playing now */
+};
+
+/* Starts `sample` playing on this channel at `volume`, from its first
+   frame. Playing on one channel leaves every other channel alone. */
+void ChannelPlay(Channel &channel, const Sample &sample, int volume);
+
+/* Fills `out` with `frame_count` frames of this channel's output: the
+   sample's frames at the channel's volume, one for one, advancing the
+   cursor. Past the sample's end the channel writes silence and is
+   inactive again — frame_count is the fact that says when. */
+void ChannelFill(Channel &channel, short *out, int frame_count);
+
 } /* namespace engine */
 
 #endif
diff --git a/src/main.cpp b/src/main.cpp
index fbfd37d..732ac22 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -162,6 +162,22 @@ int Run(void)
         std::printf(" %d", (int)sample.frames[i]);
     std::printf("\n");
 
+    /* Lesson 063: the sample starts playing on one channel, at half
+       volume — the fixed-point scale is 0-256, so 128 is half. The
+       scaling is checkable byte for byte: at half volume every frame the
+       channel emits is the sample's frame halved, truncated. */
+    Channel channel = {};
+    ChannelPlay(channel, sample, AUDIO_VOLUME_FULL / 2);
+    std::printf("engine: channel: playing %d frames at volume %d of %d\n",
+                sample.frame_count, channel.volume, AUDIO_VOLUME_FULL);
+    std::printf("engine: channel: first frames at that volume:");
+    for (int i = 0; i < 8 && i < sample.frame_count; ++i)
+        std::printf(" %d",
+                    (int)(sample.frames[i * sample.channels] *
+                          channel.volume) /
+                        AUDIO_VOLUME_FULL);
+    std::printf("\n");
+
     double sprite_x = 312.0, sprite_y = 232.0;
     double started = platform::Now();
     double last = started;
@@ -222,7 +238,6 @@ int Run(void)
        has reached, what has been fed, and whether the end has been
        named. The sample's frame_count is the fact that says when the
        sample ends; nothing here assumes how long it is. */
-    int sample_cursor = 0;      /* the next frame the feed takes */
     int sample_fed = 0;         /* frames of sample handed to the device */
     int sample_feeds = 0;       /* feeds that carried sample frames */
     bool sample_end_named = false;
@@ -327,32 +342,22 @@ int Run(void)
            where no buffer was due — so the phase accounts for all of the
            frame's audio work.
 
-           Lesson 062: the stream is the sample. One buffer is filled from
-           the sample's frames where the sample has them and with silence
-           beyond its end — silence is a stream too, and the device keeps
-           getting its buffers. frame_count is the fact that says when the
-           sample ends; the fill never runs past it. */
+           Lesson 063: the stream is the channel's output. One buffer is
+           what the channel produces — its sample's frames at its volume,
+           and silence past the sample's end. frame_count is the fact that
+           says when the sample ends; the channel never runs past it. */
         double t_audio = platform::Now();
-        if (audio.output && sample.frames && t_audio >= next_feed) {
-            /* Each frame is sample.channels values wide — one here, the
-               loader refuses anything else — and the engine's stream is
-               one channel wide, so a frame is its first (only) channel. */
-            int left = sample.frame_count - sample_cursor;
-            int take = left < CHUNK_FRAMES ? left : CHUNK_FRAMES;
-            for (int i = 0; i < take; ++i)
-                stream[i] =
-                    sample.frames[(sample_cursor + i) * sample.channels];
-            for (int i = take; i < CHUNK_FRAMES; ++i)
-                stream[i] = 0;
+        if (audio.output && t_audio >= next_feed) {
+            int before = channel.cursor;
+            ChannelFill(channel, stream, CHUNK_FRAMES);
+            int take = channel.cursor - before;
 
             if (platform::SubmitSamples(audio.output, stream,
                                         CHUNK_FRAMES)) {
-                sample_cursor += take;
                 sample_fed += take;
                 if (take > 0)
                     sample_feeds += 1;
-                if (!sample_end_named &&
-                    sample_cursor == sample.frame_count) {
+                if (!sample_end_named && !channel.active) {
                     /* The end, named in the sample's own numbers: what was
                        fed before silence, and how many buffers carried it. */
                     sample_end_named = true;
```

## Exercices

Deux exercices mixtes. Chacun se termine par sa solution — un diff contre
l'état final de cette leçon plus une visite guidée — après l'énoncé.

### Exercice 1 — Quart de volume, au chiffre près *(predict-the-output)*

L'exécution démarre le canal à 128 sur 256 — la moitié de l'échelle — et les
premières trames qu'il émet sont celles de l'échantillon divisées par deux.
Passez le volume de départ du canal à 64 et prédisez son comportement au
chiffre près avant de rien lancer : les huit premières trames qu'il émet à
partir des huit premières de la tonalité (0 513 1024 1531 2032 2525 3009 3480),
troncature comprise ; la séquence des remplissages — combien de remplissages
portent des trames d'échantillons, combien de trames chacun porte, et où
exactement le canal redevient inactif : quel remplissage, quelle trame de
celui-ci, et ce que le tampon de ce remplissage contient trame par trame ; et
exactement ce que dira le rapport de fin. Donnez au remplissage une sonde — une
ligne par remplissage, nommant combien de trames d'échantillons il a portées et
si le canal est actif après — lancez-la au nouveau volume, et réconciliez chaque
nombre avec la mise à l'échelle, le curseur et `frame_count`. Terminez par la
frontière que le remplissage expose : pour un échantillon de 1000 trames à la
place, où le canal redeviendrait-il inactif, et que compterait le rapport ?

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-063/ex1.md)

### Exercice 2 — Le fondu vers le silence *(extend-the-code)*

Le volume est un état vivant : les champs du canal sont lus au moment du
remplissage, donc la lecture peut changer de niveau en cours de route. Faites
démarrer l'échantillon à pleine échelle (`AUDIO_VOLUME_FULL`) et appliquez-lui
un fondu — chaque fois que la boucle alimente le périphérique, le volume du
canal baisse de 16 sur 256 (jamais sous zéro), et l'exécution nomme le nouveau
volume à chaque baisse. Prédisez avant de lancer : les remplissages qui portent
du son et le remplissage exact où la sortie du canal devient du silence pur ;
ce que le canal fait à partir de ce remplissage — quand sa lecture se termine
réellement, et ce que dit le rapport de fin aux volumes du fondu ; et ce que
compte le `frames fed` du rapport une fois que les trames qu'il alimente sont du
silence. Puis lancez, et réconciliez le journal du fondu ligne par ligne avec la
séquence des remplissages et le rapport de fin.

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-063/ex2.md)

---

**Partie :** [Partie 3 — le son](../../index.md) ·
**Précédente :** [Leçon 062 — les faits de lecture de l'échantillon](lesson-062-playback.md) ·
**Suivante :** [Leçon 064 — le mixage](lesson-064-mix.md) ·
**Étiquette de code :** [`lesson-063`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-063)

*Page traduite de la version anglaise `book/lessons/part-3/lesson-063-channel.md`,
révision `95a98ec`.*

<!-- translation-source: book/lessons/part-3/lesson-063-channel.md @ 95a98ec -->
