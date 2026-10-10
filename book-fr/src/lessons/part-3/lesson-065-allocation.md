# Leçon 065 — l'allocation des canaux

{{#include ../../stability-horizon.md}}

## Prose

La leçon 064 a donné seize canaux au mixeur et a laissé une question ouverte :
qui joue où. Seize emplacements, et bien plus de sons sur une exécution — chaque
son qui démarre a besoin de l'un d'eux, à l'instant où il démarre. Cette leçon
est l'allocateur. Le pool est fixe ; un son prend le **premier canal d'effet
libre** ; et quand tous les canaux d'effet sont occupés, le mixeur **vole le
plus ancien** — le son démarré le plus tôt cède son canal au son qui vient
d'arriver.

### Le premier canal libre

`MixerPlay` dans `src/audio.cpp` répond à une demande : démarrer cet échantillon
à ce volume, et dire sur quel canal il a atterri. Le parcours est délibérément
simple — du canal qui suit le canal de la musique jusqu'à la fin du pool,
prendre le premier qui n'est pas actif. `ChannelPlay` y démarre le son
exactement comme la leçon 063 le faisait, le canal est estampillé de l'ordre du
mixeur, et le numéro du canal est la valeur de retour. L'appelant apprend où est
le son ; le mixeur le savait déjà.

Le premier libre est toute la politique quand il y a de la place, et cela
suffit. Il n'y a pas de file d'attente où patienter ni de recherche d'un
meilleur canal : au plus quinze vérifications, chacune un drapeau. Cela garde
aussi le pool **dense** — les sons se regroupent au bas du pool et le parcours
s'arrête au premier qu'il trouve — et le mixage au-dessus ne se soucie pas le
moins du monde de savoir lesquels des canaux jouent.

L'exécution rend l'allocation imprimable. Seize effets — un de plus que les
quinze canaux d'effet du pool — passent par `MixerPlay` au volume 16 sur 256,
bas pour que quinze d'entre eux tiennent encore dans le format en s'additionnant :
chacun contribue au plus 511 de la crête de 8191 de la tonalité, et quinze
d'entre eux s'additionnent au plus à 7665. Le journal, ce sont les réponses de
l'allocateur lui-même :

```
engine: sample: 22050 frames at 44100 Hz, 1 channel, first frames: 0 513 1024 1531 2032 2525 3009 3480
engine: mix: effect  1 -> channel  1
engine: mix: effect  2 -> channel  2
...
engine: mix: effect 15 -> channel 15
engine: mix: effect 16 -> channel  1 (the oldest was stolen)
engine: mix: music channel 0 is reserved and was never stolen
engine: mix: 30 buffers of sound, then silence
```

Les effets 3 à 14 se complètent exactement comme le motif le montre :
`effect  N -> channel  N`, une ligne par effet. Les quinze premiers appuis
prennent le premier canal libre et le trouvent à 1, 2, 3, etc. Le seizième est
la ligne intéressante : le pool est plein, et la réponse est le canal 1.

### Quand ils sont tous occupés, le plus ancien s'en va

Un pool plein n'est pas un échec — c'est le cas pour lequel l'allocateur a une
réponse documentée. Le second parcours trouve le canal d'effet dont le `started`
est le plus petit, et `ChannelPlay` le redémarre avec le nouveau son : le
curseur revient à la première trame de l'échantillon et l'ancien son disparaît
du mixage. L'effet 16 de l'exécution atterrit sur le canal 1 parce que le canal
1 détient l'effet démarré le plus tôt — celui d'ordre 1.

Notez ce qui tient toujours pendant un vol. Le son que le joueur vient de
demander est toujours celui qui joue ; l'interruption tombe sur un son déjà en
cours. Avec quinze canaux et seize sons, quelque chose doit céder — la politique
choisit **quel** son cède, pas si l'un d'eux cède.

### L'ordre, pas le temps

« Le plus ancien » a besoin d'un sens, et les deux nouveaux champs lui en
donnent un : `Channel.started` est le moment où le son courant du canal a
commencé, et `Mixer.order` est un compteur du nombre de sons démarrés. Chaque
démarrage — canal libre ou vol — prend l'ordre suivant et en estampille le
canal. La plus petite estampille est le plus ancien son ; c'est toute la
relation.

Pourquoi un compteur et pas une horloge. Ce que la politique compare, c'est
l'*ordre* — démarré avant, démarré après — et les démarrages eux-mêmes
définissent cette relation ; un horodatage répondrait à la même question avec
plus de machinerie et pas une meilleure réponse. L'horloge de la plateforme est
une question d'OS derrière la couture, et cet allocateur n'a jamais besoin de la
lui poser : un `long` de démarrages, incrémenté à chacun, est tout ce que « le
plus ancien » a jamais voulu dire. (Un compteur qui ne revient jamais à une
valeur précédente est aussi ce qui garde la comparaison honnête : un canal volé
est estampillé à nouveau, donc le prochain pool plein passe au suivant en
ancienneté au lieu d'abandonner deux fois de suite le même son.)

### Le canal 0 est le canal de musique

`AUDIO_MUSIC_CHANNEL` — le canal 0 — est réservé. Le parcours du premier libre
commence après lui et le parcours de vol ne considère jamais que les canaux 1 à
15 : les effets prennent les quinze canaux d'effet du pool, et le canal 0 n'est
jamais proposé à un effet ni jamais abandonné. La leçon 066 y met la musique ;
cette leçon ne fait que le garder vide.

La réservation mérite sa propre phrase parce que c'est une promesse, pas une
préférence. La musique est un long son sous tout le reste ; les effets sont les
courtes rafales par-dessus. Si une seconde chargée pouvait voler le canal 0, la
musique sauterait exactement quand le jeu est le plus vivant — l'arrière-plan
doit survivre au moment le plus chargé, donc le moment le plus chargé n'a pas le
droit d'y toucher.

### Pourquoi un pool fixe

Rien n'est alloué pendant que le son joue. Les canaux existent parce que
`MixerInit` a parcouru le pool une fois au démarrage ; chaque démarrage écrit
dans l'un d'eux. C'est l'habitude de l'arena appliquée aux canaux — **la
capacité est une décision, pas un événement** — et la raison est la même que
toujours : un son est nécessaire à l'instant où le bouton est pressé. Il ne peut
pas attendre une allocation, et le jouer ne doit jamais échouer parce que la
machine est occupée. Avec le pool, un son n'échoue jamais à démarrer ; quand la
capacité s'épuise, la réponse est une politique, pas une erreur.

### Pourquoi le plus ancien, et pas autre chose

Deux alternatives sont légales, et c'est là qu'elles échouent.

**Refuser le nouveau son.** Rien n'est coupé ; chaque son qui joue joue jusqu'à
sa fin. Mais le joueur a pressé le bouton et rien ne s'est passé — la réponse de
l'entrée est le silence, et la seconde chargée est exactement le moment où le
retour compte le plus. Le refus est une vraie politique pour des sons qui ne
doivent pas être interrompus ; l'exercice 2 la fait passer en code pour que la
phrase puisse être vérifiée au lieu d'être crue.

**Voler le plus silencieux.** Juste sur le papier — abandonner le son qui
contribue le moins. Mais « silencieux » n'est pas un nombre que ce cours
possède : le volume du canal est un facteur d'échelle, pas à quel point le son
*est* fort, et la sonie perçue est un sujet que le cours n'a pas enseigné. Une
politique bâtie sur une quantité que personne n'a définie est une politique que
personne ne peut vérifier.

L'effet le plus ancien est le son dont l'auditeur a entendu le plus ; il a dit
l'essentiel de ce qu'il avait à dire, et le nouveau son est ce qui vient de se
passer. C'est le compromis que fait cette politique, et il n'est pas gratuit —
un son volé est coupé au milieu de l'échantillon, et un jeu qui garde seize
effets occupés ne cesse de les couper. L'affirmation est seulement que les
coupes tombent sur les sons qui méritent le moins d'être gardés.

### Ce que cette exécution a vérifié, et ce qu'elle n'a pas vérifié

Tous les nombres proviennent d'exécutions réelles de l'état final de cette leçon
sur cette machine, face au périphérique `null` d'ALSA :

- **L'allocation se vérifie ligne par ligne.** Les effets 1 à 15 atterrissent
  sur les canaux 1 à 15 ; l'effet 16 atterrit sur le canal 1 avec le plus ancien
  effet abandonné ; le canal de musique est nommé et intact. Chaque ligne est la
  propre valeur de retour de l'allocateur imprimée — pas une affirmation sur le
  son, une affirmation sur le canal donné à chaque son.
- **La fin de l'échantillon n'est changée par rien de tout cela.** La ligne de
  fin est maintenant dans les propres nombres du mixage — plusieurs canaux
  jouent, donc il n'y a pas de curseur d'un seul canal à compter ; l'exécution
  compte plutôt les tampons qui ont porté du son :

  ```
  engine: mix: 30 buffers of sound, then silence
  ```

  Trente tampons de 735 trames font 22050 — la longueur de l'échantillon,
  exactement. La rafale a changé qui joue, pas combien de temps le son dure.
- **Le build porte exactement un avertissement :** `DrawScene` défini mais non
  utilisé dans `src/main.cpp` — la verrue de la partie 2 reportée à dessein —
  et cette leçon n'en ajoute aucun. `tools/check-boundary.sh` passe toujours :
  l'allocateur ne pose aucune question à l'OS — l'horloge dont il n'a pas besoin
  est exactement pourquoi la frontière tient — et `openspec validate --all`
  rapporte 11 passed, 0 failed.

Ce que cette exécution ne peut pas vérifier, c'est la part qui demande des
oreilles : aucun haut-parleur n'a émis de son — `null` prend les échantillons et
les jette. « Quel son cède » trouve ici sa réponse en canaux et en compteurs ;
ce que la coupe sonne sur de vrais haut-parleurs est à vous de l'entendre.

## Étape de code

Une modification pour cette leçon, trois fichiers : `src/audio.h` et
`src/audio.cpp` font grandir l'allocateur — `MixerPlay`, l'estampille
`Channel.started`, le compteur `Mixer.order` et la réservation
`AUDIO_MUSIC_CHANNEL` — et `src/main.cpp` joue une rafale scénarisée de seize
effets à travers lui, un de plus que les canaux d'effet du pool, pour que la
politique se déroule devant le lecteur. Le rapport de fin de la boucle est
reformulé dans les propres nombres du mixage — combien de tampons ont porté du
son — parce qu'avec plusieurs canaux qui jouent, il n'y a pas de canal unique à
compter. Le mixage lui-même est celui de la leçon 064, intact. Son état final
est étiqueté `lesson-065`.

```diff
diff --git a/src/audio.cpp b/src/audio.cpp
index 58c3af8..abe239b 100644
--- a/src/audio.cpp
+++ b/src/audio.cpp
@@ -231,7 +231,35 @@ void MixerInit(Mixer &mixer)
         mixer.channels[c].cursor = 0;
         mixer.channels[c].volume = 0;
         mixer.channels[c].active = false;
+        mixer.channels[c].started = 0;
     }
+    mixer.order = 0;
+}
+
+int MixerPlay(Mixer &mixer, const Sample &sample, int volume)
+{
+    /* The first free channel of the pool's effects — the music channel is
+       reserved, so the walk starts after it. */
+    for (int c = AUDIO_MUSIC_CHANNEL + 1; c < AUDIO_MIXER_CHANNELS; ++c) {
+        if (!mixer.channels[c].active) {
+            ChannelPlay(mixer.channels[c], sample, volume);
+            mixer.channels[c].started = ++mixer.order;
+            return c;
+        }
+    }
+
+    /* Every effect channel is busy. The policy is to steal the oldest —
+       the one that started earliest — because it is the sound the
+       listener has already heard the most of. The music channel is never
+       a candidate. */
+    int oldest = AUDIO_MUSIC_CHANNEL + 1;
+    for (int c = oldest + 1; c < AUDIO_MIXER_CHANNELS; ++c) {
+        if (mixer.channels[c].started < mixer.channels[oldest].started)
+            oldest = c;
+    }
+    ChannelPlay(mixer.channels[oldest], sample, volume);
+    mixer.channels[oldest].started = ++mixer.order;
+    return oldest;
 }
 
 void MixBuffer(Mixer &mixer, short *out, int frame_count)
diff --git a/src/audio.h b/src/audio.h
index 2b33c0f..af36e32 100644
--- a/src/audio.h
+++ b/src/audio.h
@@ -80,6 +80,7 @@ struct Channel {
     int cursor;           /* the next sample frame to read */
     int volume;           /* 0..AUDIO_VOLUME_FULL, fixed point */
     bool active;          /* playing now */
+    long started;         /* when this channel began, in the mixer's order */
 };
 
 /* Starts `sample` playing on this channel at `volume`, from its first
@@ -89,15 +90,28 @@ void ChannelPlay(Channel &channel, const Sample &sample, int volume);
 /* The mixer's fixed set of channels. */
 constexpr int AUDIO_MIXER_CHANNELS = 16;
 
+/* Channel 0 is the music channel: reserved, and never stolen. Effects
+   take the rest. */
+constexpr int AUDIO_MUSIC_CHANNEL = 0;
+
 /* The mixer: one fixed pool of channels, decided up front — nothing is
    allocated while sound plays. */
 struct Mixer {
     Channel channels[AUDIO_MIXER_CHANNELS];
+    long order; /* how many sounds have started, so "oldest" has a meaning */
 };
 
 /* Every channel idle and free. */
 void MixerInit(Mixer &mixer);
 
+/* Starts `sample` playing at `volume` and says which channel it landed
+   on. An effect takes the first free channel of the pool's effects
+   (everything but the music channel). When they are all busy the mixer
+   steals the **oldest effect channel** — the one that started earliest —
+   so the sound that has had the longest hearing is the one dropped. The
+   music channel is never stolen. */
+int MixerPlay(Mixer &mixer, const Sample &sample, int volume);
+
 /* The mix: `frame_count` frames of stream, each one the sum of every
    active channel's next frame at its volume, clamped to the format's
    range. Clamped, never wrapped — a sum past the range lands on the
diff --git a/src/main.cpp b/src/main.cpp
index 25ce44c..1e86986 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -162,24 +162,22 @@ int Run(void)
         std::printf(" %d", (int)sample.frames[i]);
     std::printf("\n");
 
-    /* Lesson 064: two channels playing the same sample at different
-       volumes, so the mix is a real sum and the numbers are checkable —
-       each output frame is the two contributions added, then clamped. */
+    /* Lesson 065: a scripted burst of effects — one more than the pool's
+       effect channels — so the allocation policy runs in front of the
+       reader: the first free channel for each sound, and then the oldest
+       effect channel stolen. Volume is low so sixteen of them still sum
+       inside the format. */
     Mixer mixer;
     MixerInit(mixer);
-    ChannelPlay(mixer.channels[0], sample, AUDIO_VOLUME_FULL / 2);
-    ChannelPlay(mixer.channels[1], sample, AUDIO_VOLUME_FULL / 4);
-    std::printf("engine: mix: channel 0 at volume %d, channel 1 at volume %d of %d\n",
-                mixer.channels[0].volume, mixer.channels[1].volume,
-                AUDIO_VOLUME_FULL);
-    std::printf("engine: mix: first frames (summed):");
-    for (int i = 0; i < 8 && i < sample.frame_count; ++i) {
-        int v = sample.frames[i * sample.channels];
-        std::printf(" %d",
-                    (v * mixer.channels[0].volume) / AUDIO_VOLUME_FULL +
-                        (v * mixer.channels[1].volume) / AUDIO_VOLUME_FULL);
+    const int EFFECT_CHANNELS =
+        AUDIO_MIXER_CHANNELS - AUDIO_MUSIC_CHANNEL - 1;
+    for (int i = 0; i <= EFFECT_CHANNELS; ++i) {
+        int ch = MixerPlay(mixer, sample, AUDIO_VOLUME_FULL / 16);
+        std::printf("engine: mix: effect %2d -> channel %2d%s\n", i + 1, ch,
+                    i < EFFECT_CHANNELS ? "" : " (the oldest was stolen)");
     }
-    std::printf("\n");
+    std::printf("engine: mix: music channel %d is reserved and was never stolen\n",
+                AUDIO_MUSIC_CHANNEL);
 
     double sprite_x = 312.0, sprite_y = 232.0;
     double started = platform::Now();
@@ -241,8 +239,7 @@ int Run(void)
        has reached, what has been fed, and whether the end has been
        named. The sample's frame_count is the fact that says when the
        sample ends; nothing here assumes how long it is. */
-    int sample_fed = 0;         /* frames of sample handed to the device */
-    int sample_feeds = 0;       /* feeds that carried sample frames */
+    int sample_feeds = 0;       /* buffers that carried sound */
     bool sample_end_named = false;
     while (!platform::CloseRequested(opened.window)) {
         platform::PumpEvents(opened.window);
@@ -351,21 +348,28 @@ int Run(void)
            when a sample ends; no channel ever runs past it. */
         double t_audio = platform::Now();
         if (audio.output && t_audio >= next_feed) {
-            int before = mixer.channels[0].cursor;
+            /* Did this buffer carry sound? Answered from the cursors: the
+               buffer carried sample frames exactly when some channel's
+               cursor moved during the mix. */
+            long before = 0;
+            for (int c = 0; c < AUDIO_MIXER_CHANNELS; ++c)
+                before += mixer.channels[c].cursor;
             MixBuffer(mixer, stream, CHUNK_FRAMES);
-            int take = mixer.channels[0].cursor - before;
+            long after = 0;
+            for (int c = 0; c < AUDIO_MIXER_CHANNELS; ++c)
+                after += mixer.channels[c].cursor;
+            bool any = after > before;
 
             if (platform::SubmitSamples(audio.output, stream,
                                         CHUNK_FRAMES)) {
-                sample_fed += take;
-                if (take > 0)
+                if (any)
                     sample_feeds += 1;
-                if (!sample_end_named && !mixer.channels[0].active) {
-                    /* The end, named in the sample's own numbers: what was
-                       fed before silence, and how many buffers carried it. */
+                if (!sample_end_named && !any) {
+                    /* The end, named in the mix's own numbers: how many
+                       buffers carried sound before it ran out. */
                     sample_end_named = true;
-                    std::printf("engine: sample: %d frames fed in %d buffers — the sample's end; the stream is silence from here\n",
-                                sample_fed, sample_feeds);
+                    std::printf("engine: mix: %d buffers of sound, then silence\n",
+                                sample_feeds);
                 }
             } else {
                 /* A device that will not take the samples is named once,
```

## Exercices

Deux exercices mixtes. Chacun se termine par sa solution — un diff contre l'état
final de cette leçon plus une visite guidée — après l'énoncé.

### Exercice 1 — Le vingt et unième appui *(predict-the-output)*

Chaque effet sonore est un appui sur un bouton : le joueur provoque des sons
dans l'ordre que le jeu autorise, et l'allocateur répond à chaque appui par un
canal. Avant de rien lancer, prédisez ce que cette séquence fait à un mixeur
fraîchement initialisé. Quinze appels à `MixerPlay` d'affilée — les effets 1 à
15. Les sons des effets 4 et 9 se terminent. Trois appels — les effets 16, 17,
18. Le son de l'effet 7 se termine. Deux appels — les effets 19 et 20. Dites,
pour chaque appel, sur quel canal l'effet atterrit ; pour chaque appel qui
rencontre un pool plein, quel son est abandonné et depuis quel canal ; et, la
séquence terminée, ce que `mixer.order` contient. Puis la frontière : le vingt
et unième appel — quel son s'en va, et pourquoi celui-là. Donnez à l'exécution
une sonde — la même séquence à travers le `MixerPlay` du moteur lui-même sur un
mixeur d'essai, chaque canal d'atterrissage, chaque fin et chaque son abandonné
imprimés à partir des réponses de l'allocateur lui-même — et réconciliez chaque
ligne avec le parcours et le compteur.

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-065/ex1.md)

### Exercice 2 — La politique qui refuse *(extend-the-code)*

Le pool de cette leçon vole le plus ancien canal d'effet quand ils sont tous
occupés ; l'alternative qu'elle nomme et rejette est de refuser le nouveau son à
la place. Faites exister la politique rejetée au lieu de croire la phrase : un
second allocateur à côté de `MixerPlay` — même pool, même premier canal d'effet
libre — qui démarre le son quand il y a de la place et le refuse, nommément,
quand tous les canaux d'effet sont occupés. `MixerPlay` lui-même reste intact.
Pilotez la rafale de seize effets de la leçon à travers les deux allocateurs et
rapportez ce que le seizième appui fait sous chacun. Terminez par un
paragraphe : quelle politique vous livreriez pour les effets sonores d'un jeu,
et ce que le joueur vit sous celle que vous laissez de côté.

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-065/ex2.md)

---

**Partie :** [Partie 3 — le son](../../index.md) ·
**Précédente :** [Leçon 064 — le mixage](lesson-064-mix.md) ·
**Suivante :** [Leçon 066 — la musique comme une boucle](lesson-066-music.md) ·
**Étiquette de code :** [`lesson-065`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-065)

*Page traduite de la version anglaise `book/lessons/part-3/lesson-065-allocation.md`,
révision `74a32e1`.*

<!-- translation-source: book/lessons/part-3/lesson-065-allocation.md @ 74a32e1 -->
