# Leçon 068 — la musique et les effets ensemble

{{#include ../../stability-horizon.md}}

## Prose

La leçon 066 a donné sa musique au jeu — un seul son qui tourne jusqu'à ce
qu'on lui dise de s'arrêter. La leçon 067 lui a donné ses effets — des sons qui
jouent une fois et rendent leurs canaux. Cette leçon, c'est les deux dans une
même exécution, en même temps, à travers le même mixeur. Rien de nouveau ne se
construit ici, et c'est tout l'intérêt : **voici la ligne audio du MVD —
l'obligation O7 — livrée.** Le son du jeu, c'est de la musique et des effets
routés comme des canaux à travers un seul mixeur, sommés par un seul
`MixBuffer` dans un seul flux. La leçon d'intégration de la partie 5 câblera
ça aux événements du jeu. Elle ne le redessinera pas.

### Un mixeur, un mixage, un format

L'exécution démarre sa musique sur le canal de la musique, en boucle, à plein
volume, et déclenche des effets par-dessus sur les canaux du pool, à un quart
de volume chacun. Le journal nomme chaque route et chaque atterrissage :

```
engine: mix: music   -> channel  0 (looping, volume 256 of 256)
engine: mix: effect  1 -> channel  1 (volume 64 of 256)
engine: mix: first frames (music + effect 1, summed): 0 584 1163 1732 2286 2820 3330 3813
engine: mix: effect  2 -> channel  2 (volume 64 of 256)
engine: mix: effect  3 -> channel  3 (volume 64 of 256)
engine: mix: effect  4 -> channel  1 (volume 64 of 256)
```

Les trames mixées sont la leçon en une ligne. Chaque trame de sortie est la
trame suivante de la musique plus la trame suivante de l'effet à son volume —
277 et 307 font 584 ; 554 et 609 font 1163 ; 831 et 901 font 1732 — chaque
contribution tronquée avant la somme, l'ordre de la leçon 064, exactement comme
quand un seul type de son joue. Les premières trames de la musique et celles de
l'effet sont toutes deux dans les lignes de faits de cette leçon ; la somme
ci-dessus se réconcilie trame par trame à partir d'elles.

Regardez les canaux et voyez les deux contrats à l'œuvre en même temps. La
rafale d'effets prend les premiers canaux libres du pool — 1, 2, 3 — puis les
rend : l'effet 4 atterrit de nouveau sur le canal 1, le pool l'a rendu. Tout du
long, le canal 0 n'est jamais proposé, jamais volé, jamais dérangé. C'est la
réservation de la leçon 065 sous charge, qui tient exactement comme promis.

### Rien dans le mixage ne sait qui est qui

Voici la phrase vers laquelle ce lot construit depuis le début : **la musique
et les effets ne diffèrent que par le canal sur lequel ils sont et par le fait
que leur curseur boucle ou non.** La musique est un canal avec le drapeau de
boucle activé sur le canal réservé ; un effet est un canal avec le drapeau de
boucle éteint sur un canal du pool. Le mixage tire une trame par canal et par
trame de sortie et les additionne — il n'a aucune idée de quelle contribution
vient d'une mélodie et de laquelle vient d'un bruit de pas. Volumes, écrêtage,
pool fixe, politique de vol : tout est partagé, tout est déjà enseigné. Un
« mixeur de musique » et un « mixeur d'effets » seraient deux machines à
construire, à alimenter et à garder en phase ; ce moteur n'en a qu'un, et un
seul suffit.

Les formats s'accordent aussi, et ce n'est pas une coïncidence.
`assets/music.wav` et `assets/effect.wav` sont du même format — mono, signés
16 bits, à `AUDIO_RATE`, dans le conteneur défini par la leçon 061 — parce que
le mixeur somme des échantillons et que le format est un. Il n'y a aucune
conversion nulle part dans le chemin du son : le chargeur refuse tout ce qui
est typé autrement, donc ce que le mixage additionne ensemble est toujours le
même genre de nombre.

### La musique sous les effets

L'exécution continue de surveiller le curseur en boucle pendant que les effets
tirent, et les lignes de retournement (wrap) montrent les deux rythmes qui
s'entrelacent :

```
engine: loop: music wrapped on channel 0 — wrap 1, 133035 frames played, cursor 735 of 132300
engine: mix: effect 13 -> channel  1 (volume 64 of 256)
...
engine: loop: music wrapped on channel 0 — wrap 2, 265335 frames played, cursor 735 of 132300
...
engine: loop: music wrapped on channel 0 — wrap 3, 397635 frames played, cursor 735 of 132300
```

Vingt-neuf effets tirent dans l'exécution citée — une rafale de six, puis un
par seconde — et la musique boucle à travers tous, sur sa propre arithmétique :
133035, 265335, 397635 trames jouées, trois passes complètes de sa boucle de
132300 trames et 735 trames entamant une quatrième, curseur 735 à chaque
retournement observé. Pas un effet n'a touché le canal 0 et pas un n'a changé le
curseur de la musique. La seconde la plus chargée de cette exécution est
exactement la seconde que la leçon 065 disait que la musique doit survivre, et
elle survit, par construction : le pool n'en a jamais eu l'option.

### Le coût du mixage, dans l'enregistrement de frame

La phase `audio` est mesurée à l'intérieur de chaque frame depuis la leçon 060 ;
avec les deux types de son qui jouent, elle porte tout le chemin du son — le
mixage et sa soumission. Extrait du journal de frames de l'exécution citée :

```
frame 1: update 0.001 ms, audio 0.033 ms, render 1.632 ms (sprites 0.001, text 0.006, tilemap 0.936), present 0.461 ms, total 2.127 ms
frame 2: update 0.001 ms, audio 0.034 ms, render 1.368 ms (sprites 0.001, text 0.008, tilemap 0.945), present 0.598 ms, total 2.001 ms
frame 4: update 0.001 ms, audio 0.072 ms, render 1.742 ms (sprites 0.001, text 0.007, tilemap 0.970), present 0.768 ms, total 2.583 ms
```

Sur les 702 frames de l'exécution, la phase fait en moyenne 0.036 ms — plancher
0.025, pire 0.331 — contre une frame moyenne de 2.082 ms. Réconciliez ça avec
le travail : un tampon fait 735 trames de sortie et le mixeur parcourt tous les
`AUDIO_MIXER_CHANNELS` = 16 à chaque trame de sortie, donc un tampon représente
11 760 tirages de canal plus 735 écrêtages et écritures. 0.036 ms sur tout ça,
c'est environ 3 ns par tirage de canal et environ 50 ns par trame de sortie. Le
coût du mixage est **linéaire en nombre de canaux et invisible dans la frame** —
moins de deux pour cent d'une frame moyenne à soixante frames par seconde.
Chaque frame de cette exécution a alimenté un tampon — l'attente cadencée
réveille la boucle à l'horizon du tampon — donc il n'y avait aucune frame audio
inactive à comparer ; une frame qui ne mixe rien, ce sont essentiellement les
deux lectures d'horloge autour d'un test. Sur cette machine, la soumission
revient immédiatement (`null` prend les échantillons et les jette), donc la
phase se lit comme le coût du mixage ; sur du vrai matériel, une soumission
peut attendre de la place dans le tampon du périphérique, de la même façon que
`present` porte la synchronisation de la copie. L'exercice 1 fait de cette
mesure la vôtre, sur votre machine.

### Ce que cette exécution a vérifié, et ce qu'elle n'a pas vérifié

Tous les nombres viennent de vraies exécutions de l'état final de cette leçon
sur cette machine, contre le périphérique `null` d'ALSA :

- **La musique et les effets se mixent ensemble comme documenté.** La musique
  boucle sur le canal 0 pendant que les effets tirent sur les canaux 1, 2, 3 —
  et de retour au 1 quand le pool le rend — et les premières trames du tampon
  mixé sont les trames des deux sons additionnées :
  `0 584 1163 1732 2286 2820 3330 3813`, réconciliables depuis les lignes de
  faits au chiffre près. Un flux, un mixage, un format.
- **La musique survit à la seconde chargée.** Trois retournements sont observés —
  133035, 265335, 397635 trames jouées — à travers vingt-neuf effets, et le
  canal 0 n'est jamais pris ni volé.
- **Le format laisse l'écrêtage de côté.** Le moment le plus chargé que le
  script atteint — le pic 10442 de la musique plus trois effets à un quart de
  leur pic 9770 — somme au plus à 17768 sur 32767. L'écrêtage est là pour
  quand un jeu demande plus que ce que le format contient ; cette exécution ne
  le fait jamais.
- **L'enregistrement de frame le mesure.** La phase `audio` — dans le journal
  depuis la leçon 060 — fait en moyenne 0.036 ms sur 702 frames, en accord avec
  la taille du tampon et le nombre de canaux à environ 3 ns par tirage de
  canal, et environ 1,7 % de la frame moyenne.
- **Le build porte exactement un avertissement :** `DrawScene` définie mais non
  utilisée dans `src/main.cpp` — la verrue de la partie 2 portée à dessein — et
  cette leçon n'en ajoute aucun. `tools/check-boundary.sh` passe toujours :
  l'exécution n'a ajouté aucune question d'OS — et `openspec validate --all`
  rapporte 11 passed, 0 failed.

Ce que cette exécution ne peut pas vérifier, c'est la partie qui demande des
oreilles. Aucun haut-parleur n'a produit de son — `null` prend les échantillons
et les jette. Si un effet à un quart de volume ressort sur une musique à plein
volume, et ce que vaut la combinaison à l'écoute, sont des questions pour du
matériel qui fait du son. Les octets disent que la somme est ce qu'elle doit
être ; l'écoute est à vous.

## Étape de code

Un seul changement pour cette leçon, un seul fichier : `src/main.cpp` —
l'exécution joue le son du jeu tel que le jeu le porte, la musique en boucle
sur le canal de la musique et les effets déclenchés par-dessus sur les canaux du
pool, chacun sommé par le même `MixBuffer` dans le flux unique, avec la
vérification des premières trames et la surveillance du retournement qui rendent
cet ensemble vérifiable. Le mixeur, les routes et les assets sont ceux des
leçons 065-067, intacts : cette leçon ne câble rien de nouveau — elle fait
tourner ce qui est déjà là, ensemble. Son état final est étiqueté `lesson-068`.

```diff
diff --git a/src/main.cpp b/src/main.cpp
index 9f825c3..a8264b5 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -193,28 +193,22 @@ int Run(void)
     PrintSample("music", music);
     PrintSample("effect", effect);
 
-    /* Lesson 067: the effect route's contract, in front of the reader.
-       Two effects of one sample at different volumes, fired together —
-       one sample, two different sounds — and then one more after they
-       have played to their end, so the pool's answer is visible: the
-       channel the first effect had comes back. The music is loaded and
-       silent here; lesson 068 starts it. */
+    /* Lesson 068: the game's sound as the game has it — the music
+       looping on the music channel and effects firing over it on the
+       pool's channels, every one of them summed by the same MixBuffer
+       into the one stream. Nothing in the mix knows which is which. */
     Mixer mixer;
     MixerInit(mixer);
-    int first_channel = MixerPlayEffect(mixer, effect, AUDIO_VOLUME_FULL);
-    std::printf("engine: mix: effect 1 -> channel %2d (volume %d of %d)\n",
-                first_channel, mixer.channels[first_channel].volume,
+    MixerPlayMusic(mixer, music, AUDIO_VOLUME_FULL);
+    std::printf("engine: mix: music   -> channel %2d (looping, volume %d of %d)\n",
+                AUDIO_MUSIC_CHANNEL, mixer.channels[AUDIO_MUSIC_CHANNEL].volume,
                 AUDIO_VOLUME_FULL);
-    int second_channel = MixerPlayEffect(mixer, effect, AUDIO_VOLUME_FULL / 4);
-    std::printf("engine: mix: effect 2 -> channel %2d (volume %d of %d)\n",
-                second_channel, mixer.channels[second_channel].volume,
-                AUDIO_VOLUME_FULL);
-    std::printf("engine: mix: one sample, two volumes — two different sounds\n");
 
-    /* The script's one decision: when both effects have played to their
-       end, fire one more. Its channel is the pool's own answer. */
-    int third_channel = -1;
-    int effect_step = 0;
+    /* The run's rhythm: a burst of effects at the start — up to three in
+       flight — then one a second, all at a quarter volume so the music
+       and the busiest moment still sum inside the format. */
+    int effect_count = 0;
+    int music_wraps = 0;
 
     double sprite_x = 312.0, sprite_y = 232.0;
     double started = platform::Now();
@@ -272,13 +266,11 @@ int Run(void)
        last one — and the loop knows that without asking the platform. */
     double next_feed = platform::Now();
 
-    /* Lesson 067: the run's account of its sounds, in the mix's own
-       numbers — how many buffers have been fed and how many carried
-       sound, and whether the final silence has been named. Nothing here
-       assumes how long a sound is. */
+    /* Lesson 068: the run's bookkeeping — how many buffers have been
+       handed to the device, which drives the rhythm above. The mix's own
+       account of what it carried is the frame record's audio phase now,
+       measured like every other phase of the frame. */
     int feeds = 0;         /* buffers handed to the device */
-    int sound_feeds = 0;   /* buffers that carried sound */
-    bool silence_named = false;
     while (!platform::CloseRequested(opened.window)) {
         platform::PumpEvents(opened.window);
         if (platform::CloseRequested(opened.window))
@@ -388,69 +380,49 @@ int Run(void)
            difference. */
         double t_audio = platform::Now();
         if (audio.output && t_audio >= next_feed) {
-            /* Did this buffer carry sound? A channel speaks in it exactly
-               when it is active and has frames left to give: a one-shot at
-               its end gives none, and a looping channel at its end gives
-               everything again. */
-            bool any = false;
-            for (int c = 0; c < AUDIO_MIXER_CHANNELS; ++c) {
-                const Channel &ch = mixer.channels[c];
-                if (ch.active && ch.sample && ch.sample->frame_count > 0 &&
-                    (ch.loop || ch.cursor < ch.sample->frame_count))
-                    any = true;
+            /* The run's rhythm, in the game's own terms: an effect every
+               fifth buffer through the opening burst, then one every
+               second — each one a MixerPlayEffect on the pool's channels,
+               over the music that keeps looping. */
+            bool fire = (feeds < 30 && feeds % 5 == 0) ||
+                        (feeds >= 30 && feeds % 30 == 0);
+            if (fire) {
+                int ch = MixerPlayEffect(mixer, effect,
+                                         AUDIO_VOLUME_FULL / 4);
+                effect_count += 1;
+                std::printf("engine: mix: effect %2d -> channel %2d (volume %d of %d)\n",
+                            effect_count, ch, mixer.channels[ch].volume,
+                            AUDIO_VOLUME_FULL);
             }
 
+            int music_before = mixer.channels[AUDIO_MUSIC_CHANNEL].cursor;
+
             MixBuffer(mixer, stream, CHUNK_FRAMES);
 
             if (feeds == 0) {
-                /* The mix's own bytes while the two effects play: every
-                   output frame is their two contributions added. */
-                std::printf("engine: mix: first frames (summed):");
+                /* The mix's own bytes with both kinds of sound in it:
+                   every frame is the music's and the effect's next frame
+                   added — the same sum either way. */
+                std::printf("engine: mix: first frames (music + effect 1, summed):");
                 for (int i = 0; i < 8 && i < CHUNK_FRAMES; ++i)
                     std::printf(" %d", (int)stream[i]);
                 std::printf("\n");
             }
 
-            /* The one-shot contract's second half, observed: a channel
-               whose sound has played to its end is free again. The run's
-               next effect is fired the moment the pair is done. */
-            if (effect_step == 0 && !mixer.channels[first_channel].active &&
-                !mixer.channels[second_channel].active) {
-                effect_step = 1;
-                std::printf("engine: effect: both ended in %d buffers — channels %d and %d are free again\n",
-                            (effect.frame_count + CHUNK_FRAMES - 1) /
-                                CHUNK_FRAMES,
-                            first_channel, second_channel);
-                third_channel =
-                    MixerPlayEffect(mixer, effect, AUDIO_VOLUME_FULL);
-                std::printf("engine: mix: effect 3 -> channel %2d (volume %d of %d)%s\n",
-                            third_channel,
-                            mixer.channels[third_channel].volume,
-                            AUDIO_VOLUME_FULL,
-                            third_channel == first_channel
-                                ? " — the pool returned the first effect's channel"
-                                : "");
-            } else if (effect_step == 1 && third_channel >= 0 &&
-                       !mixer.channels[third_channel].active) {
-                effect_step = 2;
-                std::printf("engine: effect: effect 3 ended in %d buffers — channel %d is free again\n",
-                            (effect.frame_count + CHUNK_FRAMES - 1) /
-                                CHUNK_FRAMES,
-                            third_channel);
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
             }
 
-            if (platform::SubmitSamples(audio.output, stream,
-                                        CHUNK_FRAMES)) {
-                if (any)
-                    sound_feeds += 1;
-                /* The account closes when the script is done, in the mix's
-                   own numbers: how many buffers carried sound. */
-                if (!silence_named && !any && effect_step == 2) {
-                    silence_named = true;
-                    std::printf("engine: mix: %d buffers of sound, then silence\n",
-                                sound_feeds);
-                }
-            } else {
+            if (!platform::SubmitSamples(audio.output, stream,
+                                         CHUNK_FRAMES)) {
                 /* A device that will not take the samples is named once,
                    not once per frame: the run closes the output and carries
                    on in silence — its wait unbounded again. */
```

## Exercices

Deux exercices mixtes. Chacun se termine par sa solution — un diff contre
l'état final de cette leçon, plus une visite guidée — après l'énoncé.

### Exercice 1 — La phase audio, mesurée *(measure-the-performance)*

La phase `audio` est dans l'enregistrement de frame depuis la leçon 060, et
l'exécution de cette leçon mixe musique et effets à travers elle. Mesurez ce
que le mixage coûte, sur votre machine. À partir d'une vraie exécution de
l'état final de cette leçon, prenez les nombres `audio` du journal
`frame N:` et rapportez leur plancher, leur moyenne et leur pire — et ce que
la phase coûte sur une frame qui a alimenté un tampon par rapport à une qui ne
l'a pas fait (si votre exécution n'en a aucune du second type, dites pourquoi,
à partir de l'attente cadencée). Réconciliez ensuite le coût d'une frame qui
alimente avec le travail que représente un tampon : 735 trames de sortie fois
les canaux que le mixeur parcourt — par trame de sortie et par trame de canal.
Terminez par la question du budget : le coût du mixage est-il visible dans la
frame moyenne à soixante frames par seconde, et que devrait-il se passer pour
qu'il le devienne ?

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-068/ex1.md)

### Exercice 2 — La musique qui ducke *(extend-the-code)*

Quand un effet fort se déclenche par-dessus la musique, les jeux baissent la
musique un instant puis la ramènent — le ducking. Rendez-le réel à côté de
l'exécution de cette leçon : quand un effet démarre, le volume du canal de la
musique tombe à la moitié du sien pour dix tampons, puis remonte là où il
était sur dix tampons de plus. Gardez l'arithmétique dans les volumes en
virgule fixe du mixeur et laissez les routes et `MixerStop` intacts. Pilotez
un ducking sur un mixeur jetable et rapportez le volume du canal de la musique
à chaque tampon — la chute, le plancher, la remontée — et, à partir des octets
du mixage lui-même, une trame de la musique au plancher du ducking à côté de la
même trame à plein volume. Terminez par une phrase qui nomme ce que le ducking
coûte au mixage.

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-068/ex2.md)

---

**Partie :** [Partie 3 — le son](../../index.md) ·
**Précédente :** [Leçon 067 — les effets comme des one-shots](lesson-067-effects.md) ·
**Suivante :** [Leçon 069 — la démo de clôture](lesson-069-demo.md) ·
**Étiquette de code :** [`lesson-068`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-068)

*Page traduite de la version anglaise `book/lessons/part-3/lesson-068-together.md`,
révision `a6c470d`.*

<!-- translation-source: book/lessons/part-3/lesson-068-together.md @ a6c470d -->
