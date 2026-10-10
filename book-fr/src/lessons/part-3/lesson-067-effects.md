# Leçon 067 — les effets comme des one-shots

{{#include ../../stability-horizon.md}}

## Prose

La leçon 066 a donné sa musique au jeu : un son qui joue jusqu'à ce qu'on lui
dise d'arrêter. Cette leçon est l'autre moitié — l'effet sonore, la salve qui
joue jusqu'à sa fin puis disparaît. Deux sons, deux comportements, et le
contrat de la route que le moteur réserve à chacun s'écrit là où le code peut
être vérifié contre lui. La route elle-même n'est pas nouvelle. **L'allocateur
de la leçon 065 reçoit son nom ici — `MixerPlay` devient `MixerPlayEffect` —
parce qu'il a toujours été le chemin des effets.** Le parcours, la politique de
vol et le canal de musique réservé sont exactement ceux de la leçon 065 ; le
nom dit désormais de quelle route il s'agit.

### Joue une fois, puis se libère

Le contrat du one-shot, en entier : l'effet joue **une fois**, jusqu'à la fin
de l'échantillon — puis son canal devient inactif et retourne au pool, libre
pour le son suivant. Trois clauses, et la troisième est celle qui compte.
« Joue une fois », c'est le comportement de la leçon 065 avec le drapeau de
boucle à zéro ; « jusqu'à sa fin », c'est `frame_count` qui fait ce qu'il a toujours
fait. Mais « puis se libère » est la promesse du contrat au son *suivant* : le
canal ne s'attarde pas, ne s'estompe pas, n'attend pas qu'on le remarque. Il
devient inactif, et le parcours du premier canal libre le prend comme n'importe
quel autre.

Ce retour est ce qui rend les effets bon marché à déclencher. Rien n'est alloué
quand on appuie sur un bouton — le pool a été décidé au démarrage et les canaux
changent simplement de mains — et rien n'est libéré non plus : le canal qui
s'est terminé est le canal sur lequel le prochain effet joue. Un jeu peut
déclencher des effets à n'importe quel rythme que le pool peut absorber, au
prix d'un parcours et d'une écriture de structure par son. C'est encore
l'habitude de l'arena, dans sa plus petite forme : la capacité est une
décision, et la réutilisation est instantanée.

### La seconde vie du pool

L'exécution rend le retour visible. Deux effets d'un même échantillon se
déclenchent ensemble, à des volumes différents ; quand les deux ont joué
jusqu'à leur fin, un troisième se déclenche. Le journal, ce sont les réponses
de l'allocateur et des canaux eux-mêmes :

```
engine: mix: effect 1 -> channel  1 (volume 256 of 256)
engine: mix: effect 2 -> channel  2 (volume 64 of 256)
engine: mix: one sample, two volumes — two different sounds
engine: mix: first frames (summed): 0 1536 3047 4508 5898 7196 8378 9430
engine: effect: both ended in 12 buffers — channels 1 and 2 are free again
engine: mix: effect 3 -> channel  1 (volume 256 of 256) — the pool returned the first effect's channel
engine: effect: effect 3 ended in 12 buffers — channel 1 is free again
engine: mix: 24 buffers of sound, then silence
```

Suivez les canaux. La paire prend les deux premiers canaux libres du pool — 1
et 2, le parcours de la leçon 065. L'effet fait 8820 trames et les tampons 735,
donc douze tampons le portent exactement ; le tirage au-delà de sa fin est
ce qui met le canal inactif, et le tampon suivant trouve les deux canaux
libres. Le troisième effet demande un canal et atterrit sur **1** — le même
canal que le premier effet. Le pool l'a rendu. Cette ligne est la troisième
clause du contrat en train de se produire devant le lecteur, et le compte se
clôt dans les propres nombres du mixage : 24 tampons ont porté du son — douze
avec la paire jouant ensemble, douze avec le troisième effet — puis le silence.

### Le volume par son

Les deux effets de la paire jouent le même échantillon, commencent au même
instant et se terminent au même instant — et le mixage contient deux sons
différents. La différence est le `volume` que chacun porte dans son canal :
l'un joue l'échantillon à pleine échelle et l'autre au quart. Les premières
trames du tampon mixé le disent chiffre par chiffre :

```
engine: mix: first frames (summed): 0 1536 3047 4508 5898 7196 8378 9430
```

Chaque trame est deux contributions additionnées, chacune tronquée vers zéro
avant la somme — l'ordre de la leçon 064. La trame 1229 devient 1229 à plein
volume et 307 au quart — 1536. La trame 2438 devient 2438 et 609 — 3047. Le
même échantillon, et la sortie n'est pas l'échantillon : c'est l'échantillon
**à un volume**, et deux volumes sont deux sons différents. C'est pourquoi
`MixerPlayEffect` prend un volume par appel et non par mixeur : la sonie
appartient au son que le jeu vient de demander, pas au canal sur lequel il a
atterri par hasard.

### Le nom, et ce qu'il ne change pas

Un renommage est la plus petite étape de code que ce cours ait eue, et il vaut
la peine de dire ce qu'elle n'est pas. Le parcours de l'allocateur est
inchangé — premier canal d'effet libre, le plus ancien volé sous pression. La
réservation du canal de la musique est inchangée. Le mixage est inchangé. Ce
qui change, c'est que la route se lit désormais comme ce qu'elle est :
`MixerPlayEffect` est le chemin des effets, `MixerPlayMusic` est le chemin de
la musique, et la différence entre eux est le canal et le drapeau de boucle —
pas l'arithmétique, pas le pool, pas la somme. La leçon 065 a enseigné
l'allocateur avant l'heure du nommage ; cette leçon clôt le compte.

### Ce que cette exécution a vérifié, et ce qu'elle n'a pas vérifié

Tous les nombres proviennent d'exécutions réelles de l'état final de cette
leçon sur cette machine, face au périphérique `null` d'ALSA :

- **La lecture one-shot et le retour du canal, comme documenté.** Les effets 1
  et 2 atterrissent sur les canaux 1 et 2 par le parcours du premier canal
  libre du pool lui-même, les deux se terminent en douze tampons — 8820 trames
  de 735, exactement — et le pool rapporte les deux canaux de nouveau libres.
  Le troisième effet atterrit sur le canal 1 : le pool a rendu le canal du
  premier effet. Chaque numéro de canal est la valeur de retour de l'allocateur
  lui-même, imprimée.
- **Le volume par son produit une sortie différente.** Le même échantillon à
  plein et au quart se somme en `0 1536 3047 4508 5898 7196 8378 9430` —
  réconciliable trame par trame à partir des premières trames de l'échantillon,
  chaque contribution tronquée avant la somme.
- **Le compte se clôt.** 24 tampons de 735 trames ont porté du son — douze
  avec la paire, douze avec le troisième effet — puis le silence, compté par
  l'exécution à partir de l'état des canaux eux-mêmes.
- **Le build porte exactement un avertissement :** `DrawScene` défini mais non
  utilisé dans `src/main.cpp` — la verrue de la partie 2 reportée à dessein —
  et cette leçon n'en ajoute aucun. `tools/check-boundary.sh` passe toujours :
  le renommage ne touche aucune couture et ne pose aucune question à l'OS — et
  `openspec validate --all` rapporte 11 passed, 0 failed.

Ce que cette exécution ne peut pas vérifier, c'est la part qui exige des
oreilles. Aucun haut-parleur n'a émis de son : `null` prend les échantillons et
les jette. Qu'un effet au quart de volume *sonne* comme un son différent, et ce
que peut bien sonner un effet interrompu, sont des questions pour du matériel
qui fait du son — les canaux, les volumes et les octets sont tous vérifiés ici.

## Étape de code

Une modification pour cette leçon, trois fichiers : `src/audio.h` et
`src/audio.cpp` nomment la route des effets — `MixerPlay` devient
`MixerPlayEffect`, et l'en-tête énonce désormais le contrat du one-shot en
entier : joue une fois jusqu'à la fin de l'échantillon, son canal devient
inactif et retourne au pool, et le volume qu'il prend est celui du son lui-même
— et `src/main.cpp` pilote ce contrat devant le lecteur : deux effets d'un même
échantillon à des volumes différents, puis un de plus après leur fin, pour que
la réponse du pool soit une ligne du journal. La boucle, le mixage et les
parcours de l'allocateur sont ceux des leçons 065 et 066, intacts. Son état
final est étiqueté `lesson-067`.

```diff
diff --git a/src/audio.cpp b/src/audio.cpp
index 0849b9d..a1f4e47 100644
--- a/src/audio.cpp
+++ b/src/audio.cpp
@@ -256,7 +256,7 @@ void MixerInit(Mixer &mixer)
     mixer.order = 0;
 }
 
-int MixerPlay(Mixer &mixer, const Sample &sample, int volume)
+int MixerPlayEffect(Mixer &mixer, const Sample &sample, int volume)
 {
     /* The first free channel of the pool's effects — the music channel is
        reserved, so the walk starts after it. */
diff --git a/src/audio.h b/src/audio.h
index e20f6d6..7576f59 100644
--- a/src/audio.h
+++ b/src/audio.h
@@ -112,13 +112,23 @@ struct Mixer {
 /* Every channel idle and free. */
 void MixerInit(Mixer &mixer);
 
-/* Starts `sample` playing at `volume` and says which channel it landed
-   on. An effect takes the first free channel of the pool's effects
+/* The effect route — lesson 065's allocator, named at last: it was
+   always the effect path. Starts `sample` playing at `volume` and says
+   which channel it landed on.
+
+   The one-shot contract, in full: the effect plays **once**, to the
+   sample's end — then its channel goes inactive and returns to the
+   pool, free for the next sound. Play once, then free; that is what
+   makes effects cheap to fire. `volume` is the sound's own, carried
+   into its channel, so two effects of one sample at different volumes
+   are different sounds.
+
+   An effect takes the first free channel of the pool's effects
    (everything but the music channel). When they are all busy the mixer
    steals the **oldest effect channel** — the one that started earliest —
    so the sound that has had the longest hearing is the one dropped. The
    music channel is never stolen. */
-int MixerPlay(Mixer &mixer, const Sample &sample, int volume);
+int MixerPlayEffect(Mixer &mixer, const Sample &sample, int volume);
 
 /* Starts `sample` playing as the run's music: on the music channel,
    looping, at `volume`. That is what the reserved channel was reserved
diff --git a/src/main.cpp b/src/main.cpp
index 10cdc13..9f825c3 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -193,28 +193,28 @@ int Run(void)
     PrintSample("music", music);
     PrintSample("effect", effect);
 
-    /* Lesson 066: the music as a loop and one effect as a one-shot in the
-       same run — the difference this lesson is about. The music takes the
-       music channel and runs until the run stops it; the effect takes a
-       pool channel and runs to its end. Both are the engine's format and
-       sum through the same mix. */
+    /* Lesson 067: the effect route's contract, in front of the reader.
+       Two effects of one sample at different volumes, fired together —
+       one sample, two different sounds — and then one more after they
+       have played to their end, so the pool's answer is visible: the
+       channel the first effect had comes back. The music is loaded and
+       silent here; lesson 068 starts it. */
     Mixer mixer;
     MixerInit(mixer);
-    MixerPlayMusic(mixer, music, AUDIO_VOLUME_FULL);
-    std::printf("engine: mix: music  -> channel %2d (looping, volume %d of %d)\n",
-                AUDIO_MUSIC_CHANNEL, mixer.channels[AUDIO_MUSIC_CHANNEL].volume,
+    int first_channel = MixerPlayEffect(mixer, effect, AUDIO_VOLUME_FULL);
+    std::printf("engine: mix: effect 1 -> channel %2d (volume %d of %d)\n",
+                first_channel, mixer.channels[first_channel].volume,
                 AUDIO_VOLUME_FULL);
-    int effect_channel = MixerPlay(mixer, effect, AUDIO_VOLUME_FULL);
-    std::printf("engine: mix: effect -> channel %2d (one-shot, volume %d of %d)\n",
-                effect_channel, mixer.channels[effect_channel].volume,
+    int second_channel = MixerPlayEffect(mixer, effect, AUDIO_VOLUME_FULL / 4);
+    std::printf("engine: mix: effect 2 -> channel %2d (volume %d of %d)\n",
+                second_channel, mixer.channels[second_channel].volume,
                 AUDIO_VOLUME_FULL);
-    std::printf("engine: mix: the effect plays to its end; the music plays until the run stops it\n");
+    std::printf("engine: mix: one sample, two volumes — two different sounds\n");
 
-    /* The run's script: the music stops after this many buffers. 600
-       buffers of 735 frames are 441000 frames of music — three and a
-       third times around its 132300-frame loop. The stop is the run's
-       decision; the loop itself would go on. */
-    constexpr int MUSIC_STOP_FEEDS = 600;
+    /* The script's one decision: when both effects have played to their
+       end, fire one more. Its channel is the pool's own answer. */
+    int third_channel = -1;
+    int effect_step = 0;
 
     double sprite_x = 312.0, sprite_y = 232.0;
     double started = platform::Now();
@@ -272,14 +272,12 @@ int Run(void)
        last one — and the loop knows that without asking the platform. */
     double next_feed = platform::Now();
 
-    /* Lesson 066: the loop's own bookkeeping, in the run's numbers — how
-       many buffers have been fed and how many carried sound, how far the
-       music has played (its wraps and its cursor say), and whether the
-       silence after the stop has been named. Nothing here assumes how
-       long the loop is. */
+    /* Lesson 067: the run's account of its sounds, in the mix's own
+       numbers — how many buffers have been fed and how many carried
+       sound, and whether the final silence has been named. Nothing here
+       assumes how long a sound is. */
     int feeds = 0;         /* buffers handed to the device */
     int sound_feeds = 0;   /* buffers that carried sound */
-    int music_wraps = 0;   /* times the looping cursor returned to frame 0 */
     bool silence_named = false;
     while (!platform::CloseRequested(opened.window)) {
         platform::PumpEvents(opened.window);
@@ -390,59 +388,64 @@ int Run(void)
            difference. */
         double t_audio = platform::Now();
         if (audio.output && t_audio >= next_feed) {
-            /* The run's script, one decision in it: at the 600th buffer
-               the run stops the music. A looping channel does not end on
-               its own, so ending it is the run's call — and this is the
-               call, made in front of the reader. */
-            if (feeds == MUSIC_STOP_FEEDS) {
-                MixerStop(mixer, AUDIO_MUSIC_CHANNEL);
-                std::printf("engine: mix: MixerStop ended the music on channel %d — %d frames in %d buffers, %d wraps, cursor %d of %d\n",
-                            AUDIO_MUSIC_CHANNEL, feeds * CHUNK_FRAMES, feeds,
-                            music_wraps,
-                            mixer.channels[AUDIO_MUSIC_CHANNEL].cursor,
-                            music.frame_count);
-            }
-
-            /* Does this buffer carry sound, and did the music wrap? Both
-               answered from the channels' own state: a channel active when
-               the mix starts speaks in this buffer, and a looping cursor
-               going backwards is the wrap. */
+            /* Did this buffer carry sound? A channel speaks in it exactly
+               when it is active and has frames left to give: a one-shot at
+               its end gives none, and a looping channel at its end gives
+               everything again. */
             bool any = false;
-            for (int c = 0; c < AUDIO_MIXER_CHANNELS; ++c)
-                any = any || mixer.channels[c].active;
-            int music_before = mixer.channels[AUDIO_MUSIC_CHANNEL].cursor;
-            bool effect_before = mixer.channels[effect_channel].active;
+            for (int c = 0; c < AUDIO_MIXER_CHANNELS; ++c) {
+                const Channel &ch = mixer.channels[c];
+                if (ch.active && ch.sample && ch.sample->frame_count > 0 &&
+                    (ch.loop || ch.cursor < ch.sample->frame_count))
+                    any = true;
+            }
 
             MixBuffer(mixer, stream, CHUNK_FRAMES);
 
-            if (effect_before && !mixer.channels[effect_channel].active) {
-                /* The one-shot's end, named in the sample's own numbers:
-                   it played once, to its end, and its channel is free. */
-                std::printf("engine: loop: the effect ended on channel %d — %d frames in %d buffers, the one-shot played to its end\n",
-                            effect_channel, effect.frame_count,
-                            (effect.frame_count + CHUNK_FRAMES - 1) /
-                                CHUNK_FRAMES);
+            if (feeds == 0) {
+                /* The mix's own bytes while the two effects play: every
+                   output frame is their two contributions added. */
+                std::printf("engine: mix: first frames (summed):");
+                for (int i = 0; i < 8 && i < CHUNK_FRAMES; ++i)
+                    std::printf(" %d", (int)stream[i]);
+                std::printf("\n");
             }
-            if (mixer.channels[AUDIO_MUSIC_CHANNEL].active &&
-                mixer.channels[AUDIO_MUSIC_CHANNEL].cursor < music_before) {
-                /* The wrap: the cursor went backwards — the loop's own
-                   arithmetic, visible from outside the mixer. */
-                music_wraps += 1;
-                int cursor = mixer.channels[AUDIO_MUSIC_CHANNEL].cursor;
-                std::printf("engine: loop: music wrapped on channel %d — wrap %d, %ld frames played, cursor %d of %d\n",
-                            AUDIO_MUSIC_CHANNEL, music_wraps,
-                            (long)music_wraps * music.frame_count + cursor,
-                            cursor, music.frame_count);
+
+            /* The one-shot contract's second half, observed: a channel
+               whose sound has played to its end is free again. The run's
+               next effect is fired the moment the pair is done. */
+            if (effect_step == 0 && !mixer.channels[first_channel].active &&
+                !mixer.channels[second_channel].active) {
+                effect_step = 1;
+                std::printf("engine: effect: both ended in %d buffers — channels %d and %d are free again\n",
+                            (effect.frame_count + CHUNK_FRAMES - 1) /
+                                CHUNK_FRAMES,
+                            first_channel, second_channel);
+                third_channel =
+                    MixerPlayEffect(mixer, effect, AUDIO_VOLUME_FULL);
+                std::printf("engine: mix: effect 3 -> channel %2d (volume %d of %d)%s\n",
+                            third_channel,
+                            mixer.channels[third_channel].volume,
+                            AUDIO_VOLUME_FULL,
+                            third_channel == first_channel
+                                ? " — the pool returned the first effect's channel"
+                                : "");
+            } else if (effect_step == 1 && third_channel >= 0 &&
+                       !mixer.channels[third_channel].active) {
+                effect_step = 2;
+                std::printf("engine: effect: effect 3 ended in %d buffers — channel %d is free again\n",
+                            (effect.frame_count + CHUNK_FRAMES - 1) /
+                                CHUNK_FRAMES,
+                            third_channel);
             }
 
             if (platform::SubmitSamples(audio.output, stream,
                                         CHUNK_FRAMES)) {
                 if (any)
                     sound_feeds += 1;
-                if (!silence_named && !any) {
-                    /* The silence after the stop, named in the mix's own
-                       numbers: how many buffers carried sound before the
-                       run stopped the last sound. */
+                /* The account closes when the script is done, in the mix's
+                   own numbers: how many buffers carried sound. */
+                if (!silence_named && !any && effect_step == 2) {
                     silence_named = true;
                     std::printf("engine: mix: %d buffers of sound, then silence\n",
                                 sound_feeds);
```

## Exercices

Deux exercices mixtes. Chacun se termine par sa solution — un diff contre
l'état final de cette leçon plus une visite guidée — après l'énoncé.

### Exercice 1 — Un échantillon, deux volumes *(predict-the-output)*

Un volume par son signifie que le même échantillon peut être n'importe quel
nombre de sons différents. Avant de lancer quoi que ce soit, prédisez les
octets. Déclenchez `assets/effect.wav` deux fois au même instant via
`MixerPlayEffect` — une fois au volume 192 sur 256 et une fois au volume 48 —
sur un mixeur neuf, et prédisez : le canal sur lequel chaque effet atterrit ;
les huit premières trames du tampon mixé, contribution par contribution, chaque
contribution tronquée vers zéro avant la somme ; et ce que le mixage contient
dans le tampon après que les deux effets ont joué jusqu'à leur fin, avec l'état
des canaux à côté. Donnez ensuite à l'exécution une sonde qui fait exactement
cela sur un mixeur d'essai et réconciliez chaque trame de ses huit premières
avec vos nombres.

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-067/ex1.md)

### Exercice 2 — L'effet qui ne doit pas s'empiler *(extend-the-code)*

Certains sons sont un seul événement : une porte dont on demande trois
claquements dans la même seconde doit sonner comme un seul claquement. Rendez
cette route réelle à côté de `MixerPlayEffect` — `MixerPlayEffectSolo` — avec
le même pool, le même volume par son, et une barrière devant elle : si
l'échantillon joue déjà sur un canal d'effet, l'appel refuse, en le nommant, au
lieu de démarrer une seconde copie. Faites passer trois appuis d'un même effet
par elle et rapportez le canal de chaque appui ou son refus ; puis faites
passer les trois mêmes appuis par `MixerPlayEffect` et rapportez la même
chose. Terminez par un paragraphe : quelle route les bruits de pas d'un jeu
devraient prendre, quelle route ses claquements de porte, et ce que le joueur
entend sous le mauvais choix.

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-067/ex2.md)

---

**Partie :** [Partie 3 — le son](../../index.md) ·
**Précédente :** [Leçon 066 — la musique comme une boucle](lesson-066-music.md) ·
**Suivante :** [Leçon 068 — la musique et les effets ensemble](lesson-068-together.md) ·
**Étiquette de code :** [`lesson-067`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-067)

*Page traduite de la version anglaise `book/lessons/part-3/lesson-067-effects.md`,
révision `b3b0598`.*

<!-- translation-source: book/lessons/part-3/lesson-067-effects.md @ b3b0598 -->
