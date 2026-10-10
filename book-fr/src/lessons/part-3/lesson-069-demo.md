# Leçon 069 — la démo de clôture

{{#include ../../stability-horizon.md}}

## Prose

Il y a onze leçons, le moteur savait dessiner un monde et ne jouait rien ; il y
a dix leçons, le son était des octets. Voici à quoi ressemble la promesse en
train de tourner : **une boucle de frame mesurée qui fait tout le travail du
moteur** — le monde de la partie 2 dessiné par le moteur de rendu du moteur, à
côté du son de la partie 3 joué par son mixeur, côte à côte, chaque phase
chronométrée. Rien de nouveau n'est inventé aujourd'hui ; aujourd'hui les
parties *s'emboîtent*, et l'emboîtement est le sujet. C'est « le son terminé ».

### La démo

```
$ DISPLAY=:99 ALSA_DEVICE=null timeout -s INT 10 ./build/game
engine: music: 132300 frames at 44100 Hz, 1 channel, peak 10442, first frames: 0 277 554 831 1107 1381 1655 1927, last frame 0
engine: effect: 8820 frames at 44100 Hz, 1 channel, peak 9770, first frames: 0 1229 2438 3607 4719 5757 6703 7544, last frame 0
engine: mix: music   -> channel  0 (looping, volume 256 of 256)
engine: part 3 done — the world draws and the sound plays
engine: world 48x32 cells (768x512 px), 3 kinds; 96 glyphs; sprite 16x16
engine: sound 132300-frame music looping on channel 0, 8820-frame effect on the pool; one mixer of 16 channels
engine: arrow keys move the sprite, space shakes the camera; close the window to stop
engine: sprite at 312,232
engine: stream: 735-frame buffers, horizon 16.7 ms; the loop feeds one when it is due
engine: mix: effect  1 -> channel  1 (volume 64 of 256)
engine: mix: first frames (music + effect 1, summed): 0 584 1163 1732 2286 2820 3330 3813
engine: mix: effect  2 -> channel  2 (volume 64 of 256)
engine: mix: effect  3 -> channel  3 (volume 64 of 256)
engine: mix: effect  4 -> channel  1 (volume 64 of 256)   <- the pool returned it
...
engine: sprite at 442,232 (t=1.754)
engine: camera base 128,0 (t=1.754)
...
engine: sprite blocked at 603,479 (t=3.490)               <- the mover meets the wall
engine: sprite unblocked at 603,480 (t=3.507)
...
engine: camera additive 6,0 (shake starts)
engine: camera additive 0,0 (at rest)
...
engine: loop: music wrapped on channel 0 — wrap 1, 133035 frames played, cursor 735 of 132300
engine: loop: music wrapped on channel 0 — wrap 2, 265335 frames played, cursor 735 of 132300
engine: loop: music wrapped on channel 0 — wrap 3, 397635 frames played, cursor 735 of 132300
...
frame 1: update 0.001 ms, audio 0.028 ms, render 1.613 ms (sprites 0.001, text 0.006, tilemap 0.912), present 0.834 ms, total 2.476 ms
frame 2: update 0.001 ms, audio 0.033 ms, render 1.307 ms (sprites 0.002, text 0.009, tilemap 0.904), present 0.838 ms, total 2.180 ms
...
engine: demo: 656 frames measured, 582 buffers fed, 25 effects fired, 3 music wraps
engine: frame budget — 656 frames, avg 2.080 ms, worst 3.620 ms (frame 596)
engine:   subsystem   avg ms    share
engine:   update       0.003       0%
engine:   render       1.374      66%
engine:     sprites    0.002       0%
engine:     text       0.007       0%
engine:     tilemap    0.951      46%
engine:   present      0.671      32%
engine:   total        2.080     100%
engine: arena: 1534080 of 33554432 bytes used
engine: close reported
engine: closed
```

L'exécution citée a été pilotée par une entrée scriptée — des flèches et une
frappe d'espace envoyées à la fenêtre — donc les rapports du monde sont dans le
journal à côté de ceux du son. Une exécution non pilotée a la même forme, moins
les lignes de mouvement. Lisez l'exécution comme une visite de la partie :

- **Le monde** est celui de la partie 2, intact (leçons 044-056) : la carte,
  les tuiles, la police et le sprite chargés d'un bloc au démarrage, le mover
  qui fait avancer le sprite à travers le monde jusqu'à ce que le mur gagne —
  `sprite blocked at 603,479` est le rapport de la leçon 056, exactement là où
  le monde dit non — la caméra qui suit à travers sa base (`camera base 128,0`
  est la vue qui glisse à travers la carte) et secoue à travers son décalage
  additif sur la touche espace, le texte du HUD disposé par-dessus le tout.
- **Le son** est celui de la partie 3 (leçons 059-068) : les octets de deux
  fichiers chargés d'un bloc et imprimés en haut — les 132300 trames de la
  musique et les 8820 de l'effet, tous deux mono, 16 bits, à la fréquence du
  moteur — routés comme des canaux à travers un seul mixeur : la musique en
  boucle sur le canal 0 à plein volume, les effets sur les canaux du pool à un
  quart de volume chacun. Les octets mixés sont la même somme que celle montrée
  par la leçon 068 — `0 584 1163 1732 …` est le `0 277 554 831 …` de la
  musique plus le `0 1229 2438 …` de l'effet à son volume.
- **Les deux à la fois** est le propos de la démo. La musique s'est retournée
  trois fois — 133035, 265335, 397635 trames jouées, curseur 735 à chaque
  retournement observé — pendant que le monde était parcouru, secoué et
  bloqué, et que vingt-cinq effets se déclenchaient par-dessus tout ça. Rien
  n'a mis une moitié en pause pour faire tourner l'autre ; les phases de la
  boucle portent les deux, et le pool n'a jamais eu l'option de toucher au
  canal de la musique.
- **La mesure** (leçons 036, 046, 051, 053, 060), ce sont les phases nommées de
  l'enregistrement de frame, une ligne par frame — `update, audio, render
  (sprites, text, tilemap), present, total` — et le cumul à la fin : le compte
  de la démo sur ce que l'exécution a fait, puis la table du budget de frames
  de la leçon 058. Notez ce que la table ne montre *pas encore* : la phase
  `audio` est dans chaque ligne de frame — `0.028`, `0.033`, `0.034` — mais les
  lignes du budget ne couvrent toujours que `update + render + present`. La
  ligne du son est celle de la leçon 070, à dessein, et la leçon suivante
  referme l'écart que cette table laisse ouvert.

### Ce que signifie « terminé »

Terminé ne veut pas dire « fini » — le moteur ne contient encore aucun jeu.
Terminé veut dire : les obligations de la partie 3 du MVD, **livrées et
démontrables** :

| Obligation | Livrée en | Démontrée par |
| ---------- | --------- | ------------- |
| O7 — lecture par canal | leçons 063-065 | des canaux avec curseurs, volumes, drapeaux de boucle ; les octets du mixage lui-même par trame de sortie |
| musique et effets routés comme des canaux | leçons 066-068 | le canal 0 en boucle à travers trois retournements tandis que les one-shots du pool vont et viennent |
| son chargé par les E/S de fichier entier de la couture | leçons 059, 061-062 | le WAV analysé à la main dans l'arena ; des échecs typés pour les fichiers manquants ou malformés |
| la sortie audio de la couture | leçon 060 | ouvrir au format du moteur, soumettre, fermer — et l'attente cadencée qui garde le périphérique alimenté |
| le mixage mesuré, pas deviné | leçon 060 | la phase `audio` dans chaque enregistrement de frame, sur l'horloge de la plateforme |

et les limites sont nommées, comme à la clôture de la partie 2 :

- **aucun haut-parleur n'a produit de son.** Chaque exécution ici va vers le
  périphérique `null` d'ALSA, qui accepte les échantillons et les jette. Le
  chemin de soumission est vérifié ; ce que le son *est* — si un effet à un
  quart de volume ressort sur une musique à plein volume — ne l'est pas, et ne
  peut pas l'être depuis cette machine. L'écoute est celle de l'exercice 1.
- **la contre-pression de la soumission n'est pas vérifiée.** `null` prend les
  échantillons instantanément ; du vrai matériel peut faire attendre une
  soumission après de la place dans son tampon. L'enregistrement de frame
  mesure la phase dans les deux cas — la leçon 070 dit ce que la ligne inclut
  de ce fait.
- **les non-objectifs du mixeur tiennent** — volumes, boucles et écrêtage, et
  rien d'autre. Pas de suite d'effets, pas de rééchantillonnage, pas d'audio
  positionnel ; cette retenue est pourquoi un seul mixeur a suffi (la règle de
  la leçon 068).
- **aucune optimisation n'a atterri dans la partie 3** — les plongées en
  profondeur ont mesuré et nommé les coûts ; la correction est le menu en trois
  passes de la partie 5.

### La clôture, et le code mort qu'elle laisse derrière elle

La démo est le dernier regard de la partie sur l'exécution, et le code mort
n'est pas ce qu'une clôture devrait laisser derrière elle. `DrawScene` —
l'auxiliaire sur lequel la leçon 054 a bâti les vérifications de caméra — est
définie mais jamais utilisée depuis que la leçon 057 a remplacé les blocs de
vérification par la démo, et c'était le seul avertissement du build. Cette
leçon la supprime : la scène est dessinée là où la démo la dessine, dans la
boucle de frame, à travers les mêmes appels que l'auxiliaire enveloppait. Une
démo de clôture devrait montrer ce que le moteur *est*, et une fonction que
rien n'appelle n'en fait pas partie. Le build est désormais sans
avertissement — le seul avertissement que le compilateur portait depuis que la
leçon 057 a retiré ses blocs de vérification a disparu — et il le restera.

### La forme que le reste hérite

La boucle, ce sont les quatre mêmes mouvements fixés par la leçon 043 — pump,
update, draw, present — avec l'étape audio de la leçon 060 entre update et
draw, et les services de la partie 4 vivent à côté d'eux, pas dedans. Ce que la
partie 5 hérite est délibérément déjà en place : les phases nommées de
l'enregistrement de frame (les données brutes du profileur et les lignes de la
table du budget de frames, la ligne du son incluse dès la leçon suivante), un
seul chemin de dessin à optimiser et un seul mixeur à garder — pas trois de
l'un ni de l'autre — des formats d'assets qui ne peuvent pas être chamboulés
(PPM, TXT, et le conteneur WAV que la leçon 061 a défini), et trois plongées en
profondeur dont la partie 5 relance les mesures avant de changer quoi que ce
soit. La leçon suivante clôt la partie comme la partie 2 s'est close : avec la
table de ce que les frames coûtent réellement, enfin complète.

## Étape de code

Un seul changement pour cette leçon : `main.cpp` devient la démo de clôture —
le cumul de démarrage nomme le monde *et* son son comme une seule exécution, le
cumul de la démo referme le tout (frames mesurées, tampons alimentés, effets
tirés, retournements de la musique), et la `DrawScene` morte est supprimée,
emportant avec elle le seul avertissement du build. Les bibliothèques du
moteur — sprite, blit, police (font), texte, tilemap, tuiles, caméra, et tout
`audio` — ne sont pas touchées ; la démo est l'emboîtement, et l'emboîtement
est fait de ce qui est déjà là. Son état final est étiqueté `lesson-069`.

```diff
diff --git a/src/main.cpp b/src/main.cpp
index a8264b5..455b331 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -1,11 +1,13 @@
-// main.cpp — the engine: one measured frame loop, drawing the world.
+// main.cpp — the engine: one measured frame loop, the world and its sound.
 //
-// Lesson 057: the Part 2 closing demo. Every capability of the software
-// renderer at once — the map drawn through the camera, the sprite moved
-// by polled input and stopped by the map, text laid out over it all —
-// and every phase measured, one record per frame. Nothing is invented
-// here; today the parts fit, and the fit is what the demo shows. The
-// language law of lesson 026 still holds over all of it.
+// Lesson 069: the Part 3 closing demo. Every capability of the engine at
+// once — the Part 2 world drawn through the renderer (the map through the
+// camera, the sprite moved by polled input and stopped by the map, text
+// laid out over it all) beside the Part 3 sound through the mixer (music
+// looping on its channel, effects over it on the pool's, one MixBuffer
+// into one stream) — every phase measured, one record per frame. Nothing
+// is invented here; today the parts fit, and the fit is what the demo
+// shows. The language law of lesson 026 still holds over all of it.
 
 #include <cstdio>
 
@@ -44,19 +46,6 @@ constexpr int CHUNK_FRAMES = AUDIO_RATE / 60;   /* 735 */
    keeps allocation out of the run. */
 static short stream[CHUNK_FRAMES];
 
-/* Lesson 054: the scene, drawn through the camera. The camera's summed
-   offset is applied once, at each draw's origin — the map's and the
-   sprite's. The HUD is not scene and does not pass through here. */
-static void DrawScene(Framebuffer &fb, const TileMap &map,
-                      const TileSheet &sheet, const Sprite &sprite,
-                      int sprite_x, int sprite_y, const Camera &camera)
-{
-    int x = CameraX(camera);
-    int y = CameraY(camera);
-    DrawTileMap(fb, map, sheet, -x, -y);
-    BlitSprite(fb, sprite, sprite_x - x, sprite_y - y);
-}
-
 /* Lesson 066: a loaded sample's facts, printed — the run's byte-level
    check on its two sounds. The peak is the largest frame the sample
    holds, and it is what says how much room the format still has above
@@ -217,11 +206,16 @@ int Run(void)
     int shake_frames = 0; /* lesson 054: the additive hook's demo */
     bool was_blocked = false; /* lesson 056: the mover's state report */
 
-    std::printf("engine: part 2 done — the software renderer draws the world\n");
+    /* The demo's identity: what the run is, named at once — the world
+       and its sound, one measured frame loop. */
+    std::printf("engine: part 3 done — the world draws and the sound plays\n");
     std::printf("engine: world %dx%d cells (%dx%d px), %d kinds; %d glyphs; sprite %dx%d\n",
                 map.width, map.height, map.width * TILE_SIZE,
                 map.height * TILE_SIZE, map.kind_count, FONT_COUNT,
                 sprite.width, sprite.height);
+    std::printf("engine: sound %d-frame music looping on channel %d, %d-frame effect on the pool; one mixer of %d channels\n",
+                music.frame_count, AUDIO_MUSIC_CHANNEL, effect.frame_count,
+                AUDIO_MIXER_CHANNELS);
     std::printf("engine: arrow keys move the sprite, space shakes the camera; close the window to stop\n");
     std::printf("engine: sprite at %d,%d\n", (int)sprite_x, (int)sprite_y);
 
@@ -493,6 +487,11 @@ int Run(void)
                     frame.total * 1e3);
     }
 
+    /* The demo's account: what the run did — the world's frames and the
+       sound's buffers, together — before the cost's table below. */
+    std::printf("engine: demo: %ld frames measured, %d buffers fed, %d effects fired, %d music wraps\n",
+                frame_number, feeds, effect_count, music_wraps);
+
     /* The account as the frame-budget table (lesson 058): the frame
        count, the average, the worst frame — and the render attributed to
        its subsystems, the report Part 5's finale grows. */
```

## Exercices

Deux exercices mixtes. Chacun se termine par sa solution — un diff contre
l'état final de cette leçon, plus une visite guidée — après l'énoncé.

### Exercice 1 — La démo de la partie 3 sur votre machine *(port-to-your-own-machine)*

La démo du livre a été vérifiée sans écran : Xvfb pour la fenêtre, le `null`
d'ALSA pour le son — les pixels calculés et jetés, les échantillons acceptés et
jetés. Lancez la démo de clôture sur votre propre machine et laissez ses deux
moitiés atteindre leurs vrais points d'arrivée : la fenêtre sur votre bureau,
les échantillons vers vos haut-parleurs. Pilotez l'exécution de vos propres
mains — poussez le sprite dans un mur, secouez la caméra, laissez la musique
tourner à travers un retournement — et rapportez le cumul de la démo et la
table du budget de frames à côté de ceux du livre, en nommant la machine avec
les nombres. Répondez ensuite aux deux questions que cette machine ne pouvait
pas trancher : à quoi ressemblent les nombres de la phase audio sur du matériel
dont la soumission peut pousser en retour, et qu'entendez-vous réellement quand
les effets se déclenchent par-dessus le retournement ? Une mesure sans sa
machine est une rumeur ; une démo sans son son est une demi-démo.

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-069/ex1.md)

### Exercice 2 — La table d'acceptation de la partie *(explain-in-prose)*

« Le son terminé » est une affirmation, et une affirmation veut une table.
Faites en sorte que la démo nomme les capacités qu'elle utilise — une ligne par
groupe : dessin, texte, monde, entrée, son, mesure — puis remplissez la table
d'acceptation : pour chaque scénario des spécifications du son de la partie
(les échantillons, le conteneur WAV, la lecture, un canal, le mixage,
l'allocation de canaux, la musique, les effets), la leçon qui l'a construit et
les *preuves* tirées de votre propre exécution (la commande et ce qu'elle a
imprimé). Terminez par la question qui rend la table digne d'être conservée :
quelles lignes cassent en premier quand le moteur change, et comment le
remarqueriez-vous ?

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-069/ex2.md)

---

**Partie :** [Partie 3 — le son](../../index.md) ·
**Précédente :** [Leçon 068 — la musique et les effets ensemble](lesson-068-together.md) ·
**Suivante :** [Leçon 070 — le coût du mixage dans le budget de frames](lesson-070-audio-row.md) ·
**Étiquette de code :** [`lesson-069`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-069)

*Page traduite de la version anglaise `book/lessons/part-3/lesson-069-demo.md`,
révision `f5e9029`.*

<!-- translation-source: book/lessons/part-3/lesson-069-demo.md @ f5e9029 -->
