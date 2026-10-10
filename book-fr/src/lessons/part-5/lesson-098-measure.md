# Leçon 098 — passe 1 : mesurer

{{#include ../../stability-horizon.md}}

## Prose

Le jeu est terminé ; l'optimisation commence ; et le menu figé (`target-game`,
design D11) dit exactement comment elle commence — **mesurer**. Pas « regarder
le code et deviner ce qui est lent ». Pas « optimiser les parties qui semblent
lourdes ». Instrumentez, profilez, et *nommez* le top 2 des points chauds —
avec des nombres de vraies frames — parce que les deux leçons suivantes
corrigent exactement ces deux-là et rien d'autre. Tout ce que cette passe ne
nomme pas est consigné comme travail futur et laissé tranquille.

### Les instruments, et les limites honnêtes de cette machine

Deux instruments, et seulement deux, parce que cette machine n'en a que deux.
**Le bilan des frames** — `FrameRecord`, `FrameStats`, `PrintFrameBudget`
(`frame.h`, `frame.cpp`) — mesure les phases de chaque frame sur l'horloge de
la plateforme et garde les sommes courantes ; c'est l'instrument intégré et la
source dont grandit le rapport final. Et **`gprof` via un build `-pg`** — le
profileur au niveau des fonctions, celui à côté duquel l'outillage des plongées
a été construit pour lire.

Ce que cette machine n'a **pas**, nommé pour que la mesure ne fasse jamais
semblant du contraire : `perf` (et `perf_event_paranoid` vaut `2`, donc il
serait de toute façon restreint même installé), `valgrind`/callgrind, `ltrace`,
`strace`. Ce qu'elle est : WSL2, Xvfb sur `:99`, pas de matériel son — donc
l'exécution mixe son audio en silence et la copie de l'affichage passe par le
serveur X (design D12 : chaque nombre de cette leçon porte le nom de sa
machine). Et un fait de forme qui gouverne le scénario : la boucle est **pilotée
par les événements** — elle se réveille sur des nouvelles X ou sur l'échéance
de l'alimentation audio — donc une exécution mesure autant de frames que le
cadencement lui en donne. Les exécutions ci-dessous sont cadencées à ~25 fps en
émettant en flux des événements de déplacement de fenêtre alternés (`xdotool
windowmove`), comme les lots précédents l'ont fait.

### L'unique lacune de l'instrument, comblée d'abord

Avant le profilage, le bilan lui-même reçoit son instrument. Le journal de
frames attribue les sous-phases du render — sprites, text, tilemap — mais le
*premier* travail du render, le clear du framebuffer, n'était jamais nommé :
c'était le résidu innommé entre la ligne `render` et la somme de ses
sous-phases. Une passe de mesure qui laisse un coût innommé ne peut le
reprocher à personne. L'étape de code le nomme donc : `FrameRecord` accueille
`clear`, la boucle chronomètre le clear là où il tourne, et la table du bilan
et le journal de frames portent la ligne. L'exécution dit ce qu'elle a toujours
dit, plus un nom :

```
engine: frame budget — 1551 frames, avg 1.857 ms, worst 3.137 ms (frame 646)
engine:   subsystem   avg ms    share
engine:   update       0.013       1%
engine:     entities   0.005       0%
engine:   audio        0.032       2%
engine:   render       1.370      74%
engine:     clear      0.458      25%
engine:     sprites    0.006       0%
engine:     text       0.012       1%
engine:     tilemap    0.895      48%
engine:   present      0.442      24%
engine:   total        1.857     100%
```

### L'exécution de mesure

Le scénario, c'est le jeu qu'on joue, et jouer n'est pas une ligne droite : le
héros meurt, et un jeu mort est un écran de titre. L'exécution de mesure pilote
donc du vrai jeu à dessein — `Return` vers le jeu, puis six segments de dix
secondes de marche et de tir (droite, gauche, droite, …), chaque segment suivi
de deux `Return` qui redémarrent le jeu si le héros est mort et ne font *rien*
s'il ne l'est pas (le jeu ignore `Return` ; mort → titre → jeu demande deux
pressions). 1 551 frames à ~25 fps sur une soixantaine de secondes, une mort,
cinq redémarrages.

Et le mélange des états compte, donc l'exécution est lue deux fois — une fois
entière, et une fois découpée selon le champ `step` de l'enregistrement (une
frame qui a fait avancer le temps de jeu jouait ; une frame à `step 0.000`
montrait un écran). Le découpage, calculé depuis les lignes du journal de
frames lui-même — 1 415 frames de jeu et 136 frames de panneau :

```
play  (1415 frames): update 0.013  entities 0.005  audio 0.031
                      render 1.455  clear 0.456  sprites 0.006
                      text 0.012  tilemap 0.981  present 0.444  total 1.944
panel ( 136 frames): update 0.008  entities 0.006  audio 0.037
                      render 0.488  clear 0.473  sprites 0.000
                      text 0.015  tilemap 0.000  present 0.427  total 0.961
```

Lisez cette paire une fois et la leçon du mélange est apprise : le `clear` coûte
pareil à chaque frame (chaque frame commence par peindre 307 200 pixels), le
`tilemap` seulement sur les frames de jeu, et le `0.895` de la table sur
l'exécution entière n'est que le mélange pondéré de `0.981` et `0.000`. Les
moyennes répondent à « combien a coûté cette exécution », pas « combien coûte
le jeu » — c'est pourquoi les points chauds sont nommés depuis les frames de
jeu.

### Le profil

Le build `-pg`, le même scénario, dix segments : 2 583 frames, le bilan des
frames lisant `avg 1.834 ms (tilemap 0.835, clear 0.443, present 0.433)` depuis
l'exécution même sur laquelle le profil a été écrit. `gmon.out` à la fermeture ;
`gprof build-pg/game gmon.out` — 3,53 secondes de CPU échantillonnées en grains
de 0,01 seconde :

```
  %   cumulative   self              self     total
 time   seconds   seconds    calls   s/call   s/call  name
 59.77      2.11     2.11  3501190     0.00     0.00  engine::BlitSprite(engine::Framebuffer&, engine::Sprite const&, int, int)
 37.68      3.44     1.33     2584     0.00     0.00  engine::ClearBuffer(engine::Framebuffer&, unsigned char, unsigned char, unsigned char)
  1.13      3.48     0.04     2204     0.00     0.00  engine::DrawTileMap(engine::Framebuffer&, engine::TileMap const&, engine::TileSheet const&, int, int)
  0.57      3.50     0.02    14745     0.00     0.00  engine::BlitSpriteFrame(engine::Framebuffer&, engine::Sprite const&, int, int, int, int)
  0.28      3.51     0.01 29905680     0.00     0.00  engine::(anonymous namespace)::ChannelFrame(engine::Channel&)
  0.28      3.52     0.01     2584     0.00     0.00  platform::Present(platform::Window*, unsigned char const*, int, int)
  0.28      3.53     0.01     2543     0.00     0.00  engine::MixBuffer(engine::Mixer&, short*, int)
```

Deux instruments, un verdict — et les comptes d'appels disent exactement à qui
appartient le coût de la première ligne : `BlitSprite` a été appelé 3 501 190
fois, et `DrawTileMap` parcourt 2 204 cartes × 1 536 cellules = 3 385 344
d'entre elles — **96,7 %**. Les 115 846 restantes sont les glyphes (exactement
le compte d'appels de `FontGlyph`) qui empruntent la même boucle, 3 % de ses
appels.

### Le top 2 des points chauds, nommé

**Point chaud n° 1 — le dessin de la carte.** La marche de `DrawTileMap`, un
`BlitSprite` par cellule. Sur une frame de jeu, c'est `tilemap 0.981 ms` sur
`1.944 ms` — **la moitié de la frame** ; dans le profil plat, `BlitSprite` +
`DrawTileMap` font **2,15 s des 3,53 s** de CPU échantillonné (60,9 %). C'est
le coût de dessiner 1 536 tuiles, chacune à travers la boucle par pixel du blit
de sprites.

**Point chaud n° 2 — le clear de la frame.** `ClearBuffer`, un appel par frame,
à chaque frame. C'est `clear 0.456 ms` d'une frame de jeu (23 %) et `0.473 ms`
d'une frame de panneau — **49 % de chaque frame qui ne dessine aucune carte** ;
dans le profil plat c'est la deuxième ligne, **1,33 s (37,7 %)**. C'est le coût
de peindre 307 200 pixels quatre octets à la fois.

Les deux points chauds sont des noms, pas des humeurs — les deux leçons
suivantes corrigent exactement ces deux-là (la copie de `BlitSprite` telle que
la carte la parcourt en 099, `ClearBuffer` en 100) et rien d'autre.

### Nommé, et pas celui du menu : la couture, et le reste

Un coût mesuré est plus gros que la part du point chaud n° 2 dans une frame de
jeu et n'est *pas* au menu : `present`, `0.444 ms` par frame (23 %). Regardez sa
ligne de profil — `platform::Present`, `0.01 s` de CPU en 2 583 frames, 0,28 %.
Les deux nombres sont vrais : le temps mural du present est de l'**attente**,
pas du calcul — le processus remet des pixels au serveur X et attend que la
copie soit prise. C'est le coût de la couture (`XPutImage` de la couche
plateforme), pas du travail du moteur que les leviers des plongées peuvent
raccourcir, et une présentation à double tampon ou MIT-SHM est un changement de
*couche plateforme* que le menu figé ne fait pas. Cela va sur la liste des
travaux futurs, nommé et mesuré :

- **la copie de la présentation** — `present` 0.444 ms/frame mural, 0.004
  ms/frame CPU : l'attente de la couture sur le serveur X (un second OS
  implémente ce fichier ; un present plus rapide est son changement).
- **le mixage audio à pleine charge** — `audio 0.031 ms` avec la musique et
  une poignée d'effets ; un déluge sur les 16 canaux n'est pas mesuré.
- **le travail par entité de l'update à magasin plein** — `entities 0.005 ms`
  avec une poignée d'entités vivantes ; un magasin de 64 emplacements plein de
  sparks n'est pas mesuré.
- **l'impression des rapports elle-même** — les sondes et le rapport du HUD
  s'impriment dans `update` et `text`, et la ligne du journal de frames
  s'imprime en dehors de toute phase ; une exécution plus silencieuse mesurerait
  des phases moins chères.

### Ce que cette exécution a vérifié, et ce qu'elle n'a pas vérifié

- **Le top 2 est nommé avec des nombres de vraies frames** — 1 415 frames de
  jeu du build instrumenté et 2 583 frames du build profilé, toutes deux sur
  cette machine (WSL2, Xvfb `:99`, pas de matériel son), le bilan des frames et
  `gprof` d'accord sur les noms *et* l'ordre.
- **La lacune de l'instrument est comblée** — la ligne `clear` est mesurée
  (`0.458 ms` sur l'exécution mélangée), et les nombres de la table sur
  l'exécution entière s'additionnent maintenant : `clear + sprites + text +
  tilemap` font le render, sans reste innommé.

Ce que cette exécution n'a **pas** vérifié : les nombres du build de
*livraison*. Chaque mesure ici est celle du build du cours — `-O0`, et le build
du profil ajoute la comptabilité de `-pg` par-dessus — donc les millisecondes
absolues sont celles de ce build, pas de `-O3` (la leçon 049 a chiffré cet
écart : un changement de plusieurs fois). Les *noms* des points chauds sont ce
sur quoi le menu se fixe, et les leviers des plongées — copier moins, copier
plus rapproché, copier plus large — sont exactement ce qui s'applique encore à
n'importe quel niveau d'optimisation. Le profil n'est pas finement résolu non
plus : 353 échantillons en grains de 0,01 seconde placent chaque ligne à un ou
deux pour cent près, et une exécution plus longue les raffermirait. Et rien ici
n'est une preuve de 60 fps : une exécution cadencée sans écran à ~25 fps est un
banc de mesure, pas le matériel modeste de la checklist — cette affirmation est
à la clôture de la vérifier, aussi loin que cette machine peut honnêtement le
faire.

## Étape de code

Un seul changement : la lacune de l'instrument. `src/frame.h` accueille dans
`FrameRecord` et `FrameStats` un nom de plus — `clear`, le clear du framebuffer,
le premier travail du render — et `AccountFrame` et `PrintFrameBudget` de
`src/frame.cpp` le somment et l'impriment comme première des sous-phases du
render. La boucle chronomètre le clear là où il tourne déjà et la liste
`render` du journal de frames gagne le champ, en premier, là où il se produit
dans la frame. Rien d'autre ne bouge : la mesure est le changement. Son état
final est étiqueté `lesson-098`.

```diff
diff --git a/src/frame.cpp b/src/frame.cpp
index 444b1c0..349af4f 100644
--- a/src/frame.cpp
+++ b/src/frame.cpp
@@ -19,6 +19,7 @@ void AccountFrame(FrameStats &stats, const FrameRecord &frame)
     stats.sprites_sum += frame.sprites;
     stats.text_sum += frame.text;
     stats.tilemap_sum += frame.tilemap;
+    stats.clear_sum += frame.clear;
     stats.entities_sum += frame.entities;
     if (frame.total > stats.worst) {
         stats.worst = frame.total;
@@ -47,6 +48,7 @@ void PrintFrameBudget(const FrameStats &stats)
     double update = stats.update_sum / n * 1e3;
     double audio = stats.audio_sum / n * 1e3;
     double render = stats.render_sum / n * 1e3;
+    double clear = stats.clear_sum / n * 1e3;
     double sprites = stats.sprites_sum / n * 1e3;
     double text = stats.text_sum / n * 1e3;
     double tilemap = stats.tilemap_sum / n * 1e3;
@@ -63,6 +65,11 @@ void PrintFrameBudget(const FrameStats &stats)
                 100.0 * audio / (avg * 1e3));
     std::printf("engine:   render      %6.3f      %2.0f%%\n", render,
                 100.0 * render / (avg * 1e3));
+    /* Lesson 098: the measure pass's instrument — the clear, named at
+       last. It was always inside render; until now it was the unnamed
+       remainder between render's row and its named sub-phases' sum. */
+    std::printf("engine:     clear     %6.3f      %2.0f%%\n", clear,
+                100.0 * clear / (avg * 1e3));
     std::printf("engine:     sprites   %6.3f      %2.0f%%\n", sprites,
                 100.0 * sprites / (avg * 1e3));
     std::printf("engine:     text      %6.3f      %2.0f%%\n", text,
diff --git a/src/frame.h b/src/frame.h
index b05bf68..1234c17 100644
--- a/src/frame.h
+++ b/src/frame.h
@@ -25,7 +25,11 @@ struct FrameRecord {
        (lesson 058) grows from. The named times are inside render, never
        instead of it: render stays the phase, these say where it went.
        Lesson 081: update grows the same kind of name — the entity work
-       the walk does, attributed inside the phase it lives in. */
+       the walk does, attributed inside the phase it lives in.
+       Lesson 098: the measure pass names the one piece of the frame no
+       row carried — the clear, the render's first work, until now the
+       unnamed remainder of the render row. */
+    double clear;    /* the framebuffer's clear — one color, every pixel */
     double sprites; /* sprite draws through the blit */
     double text;    /* lesson 051: text drawing — glyphs through the blit */
     double tilemap; /* lesson 053: the map's walk — tiles through the blit */
@@ -51,6 +55,7 @@ struct FrameStats {
     double sprites_sum; /* lesson 046's named sub-phase, summed like the rest */
     double text_sum;
     double tilemap_sum;
+    double clear_sum; /* lesson 098: the clear's row, summed like the rest */
     double entities_sum; /* lesson 081: the update's entity work, summed */
     double worst;      /* the longest frame so far */
     long worst_number; /* and which one it was */
diff --git a/src/main.cpp b/src/main.cpp
index f48d76a..112049a 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -240,7 +240,10 @@ int Run(void)
         /* Render: the current state's screen, and only that one (lesson
            082). The backdrop is the state's own — the world's blue in
            play, the panel's darker blue on the panel screens — cleared
-           once here, in the render phase, before the named sub-phases. */
+           once here, in the render phase, before the named sub-phases.
+           Lesson 098: the clear is a named sub-phase now — the measure
+           pass's instrument, and the account's row. */
+        double t_clear = platform::Now();
         if (game.state == GAME_PLAY) {
             ClearBuffer(*world.fb, 32, 32, 64);
         } else {
@@ -251,6 +254,7 @@ int Run(void)
             GameScreenColor(game, screen_r, screen_g, screen_b);
             ClearBuffer(*world.fb, screen_r, screen_g, screen_b);
         }
+        frame.clear = platform::Now() - t_clear;
         if (game.state == GAME_PLAY) {
             /* Lesson 083: the game draws its own world — the scrolling
                map and the live entities, through the game's camera. The
@@ -300,12 +304,14 @@ int Run(void)
            fields, one per subsystem, as the parts name them. The audio
            phase (lesson 060) joins in the record's own order, and
            lesson 079's step leads it: the game's advance beside the
-           machine's durations. */
-        std::printf("frame %ld: step %.3f ms, update %.3f ms (entities %.3f), audio %.3f ms, render %.3f ms (sprites %.3f, text %.3f, tilemap %.3f), present %.3f ms, total %.3f ms\n",
+           machine's durations. Lesson 098: the clear's field joins the
+           render's list, first — where it happens in the frame. */
+        std::printf("frame %ld: step %.3f ms, update %.3f ms (entities %.3f), audio %.3f ms, render %.3f ms (clear %.3f, sprites %.3f, text %.3f, tilemap %.3f), present %.3f ms, total %.3f ms\n",
                     frame.number, frame.step * 1e3, frame.update * 1e3,
                     frame.entities * 1e3,
                     frame.audio * 1e3,
                     frame.render * 1e3,
+                    frame.clear * 1e3,
                     frame.sprites * 1e3, frame.text * 1e3,
                     frame.tilemap * 1e3, frame.present * 1e3,
                     frame.total * 1e3);
```

## Exercices

Deux défis sur la mesure elle-même — un sur cette machine, un sur la vôtre (le
design D12 route la vérification sur matériel réel vers vous). Chacun se
termine par sa solution — un diff contre l'état final de cette leçon, plus une
visite guidée — après l'énoncé.

### Exercice 1 — Ce que le profil ne peut pas voir *(measure-the-performance)*

Le bilan des frames dit que le present coûte `0.444 ms` par frame ; le profil
plat dit que `platform::Present` a utilisé `0.01 s` de CPU sur 2 583 frames —
environ `0.004 ms` chacune. Les deux sont vrais, et l'écart est la couture qui
attend le serveur X plutôt que le processus qui calcule. Faites de la
distinction une ligne du rapport de l'exécution elle-même : une sonde qui
imprime le travail moteur de l'exécution par frame à côté de son attente de
couture par frame. Répondez ensuite avec les nombres de votre exécution :
combien de temps mural par frame le profileur ne voit-il pas, et qu'est-ce que
cela fait aux parts des points chauds — les pourcentages du profil plat
décrivent-ils la frame, ou le CPU ?

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-098/ex1.md)

### Exercice 2 — La passe de mesure sur votre machine *(port-to-your-own-machine)*

Chaque nombre de cette leçon porte sa machine — WSL2, Xvfb, pas de matériel
son, un build `-pg` en `-O0`. Faites la même passe de mesure sur la vôtre :
construisez le build `-pg`, jouez au jeu sur votre propre affichage
(périphérique son et tout), collectez le bilan des frames et le profil plat de
la même exécution, et nommez le top 2 des points chauds de **votre** machine à
côté de celui-ci. Expliquez ensuite les différences que vous avez réellement
mesurées — l'ordre a-t-il tenu (le dessin de la carte d'abord, le clear
ensuite), et sinon, que fait votre machine différemment : la copie du present,
l'alimentation du périphérique son, la largeur du CPU ? Rapportez les premières
lignes de votre profil comme une fiche à poser à côté de celle du livre.

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-098/ex2.md)

---

**Partie :** [Partie 5 — le jeu](../../index.md) ·
**Précédente :** [Leçon 097 — payer la dette](lesson-097-debt.md) ·
**Suivante :** [Leçon 099 — passe 2a : corriger le dessin de la carte](lesson-099-map-draw.md) ·
**Étiquette de code :** [`lesson-098`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-098)

*Page traduite de la version anglaise `book/lessons/part-5/lesson-098-measure.md`,
révision `17073ed`.*

<!-- translation-source: book/lessons/part-5/lesson-098-measure.md @ 17073ed -->
