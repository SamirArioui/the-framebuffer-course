# Leçon 101 — passe 3 : le rapport de budget de frames

{{#include ../../stability-horizon.md}}

## Prose

Le menu en trois passes est épuisé : la passe de mesure a nommé deux
points chauds avec des nombres issus de vraies frames, les deux passes de
correction les ont fait baisser de 43 % et 47 % avec les leviers des
plongées en profondeur, et rien d'autre n'a été touché. Voici la passe 3
et la clôture du cours : **le rapport final de budget de frames** — le
temps par frame attribué à chaque grand sous-système, mesuré sur de
vraies frames du jeu terminé, produit à partir du compte propre de
`frame-accounting` (design D11 : mesuré, jamais modélisé). C'est une page
de nombres. C'est aussi la réponse honnête à la ligne la plus dure du
MVD : *60 fps sur du matériel modeste*.

### Le rapport

Le jeu terminé, joué à fond — dix manches de marche, de tir, de mort et
de redémarrage : **2 583 frames**, dont 2 447 de jeu et 136 d'écran, sur
cette machine. La table du compte, entière et non éditée :

```
engine: frame budget — 2583 frames, avg 1.283 ms, worst 3.124 ms (frame 1101)
engine:   subsystem   avg ms    share
engine:   update       0.013       1%
engine:     entities   0.005       0%
engine:   audio        0.033       3%
engine:   render       0.799      62%
engine:     clear      0.244      19%
engine:     sprites    0.006       0%
engine:     text       0.011       1%
engine:     tilemap    0.538      42%
engine:   present      0.438      34%
engine:   total        1.283     100%
engine:   by state    2447 play frames at 1.313 ms, 136 screen frames at 0.749 ms
engine:   budget      60 fps is 16.667 ms a frame — 0 of 2583 frames over it, worst 3.124 ms (19% of it)
engine:   machine     WSL2, Xvfb :99, no sound hardware (the course's authoring machine)
```

Lisez-la dans l'ordre où elle a grandi. L'**attribution** est le contrat
d'origine du compte (le « au moins la mise à jour du monde, le rendu et
la présentation » de la spécification) : la moyenne de chaque grand
sous-système sur les frames qui ont réellement tourné, les sous-phases
nommées à l'intérieur de leurs phases, et chaque nombre une somme
mesurée — `clear`, `sprites`, `text`, `tilemap` s'additionnent en
`render` sans reste non nommé, parce que la leçon 098 a nommé le dernier
d'entre eux. La ligne **by state** est la leçon du mélange rendue
permanente : une frame de jeu coûte `1.313 ms`, une frame d'écran
`0.749 ms`, et une moyenne des deux n'est ni l'une ni l'autre. La ligne
**budget** vérifie l'affirmation des 60 fps frame par frame — pas « la
moyenne est petite » mais *voici combien de frames ont franchi la ligne* :
zéro. Et la ligne **machine** est la règle du design D12 — une
affirmation de performance porte sa machine — imprimée par le rapport
lui-même, pour que les nombres ne puissent pas voyager sans leur nom.

### Où va la frame de jeu

Une frame de jeu du jeu terminé, `1.313 ms` en moyenne, et les empreintes
des deux passes sont les deux lignes qui ont baissé :

```
                          lesson-098   lesson-099   lesson-100   the finale
  tilemap (map's draw)      0.981        0.559        0.539        0.538
  clear (the frame's)       0.456        0.449        0.239        0.244
  present (the seam's)      0.444        0.434        0.428        0.438
  audio + update            0.045        0.046        0.046        0.046
  sprites + text            0.018        0.016        0.016        0.017
  total                     1.944        1.505        1.269        1.313
```

(la colonne de la clôture est sa propre exécution — les exécutions
oscillent de quelques pour cent, le plancher de bruit mesuré par la leçon
097 ; la *forme* des lignes est l'affirmation.) Les deux lignes nommées
ont baissé de `1.437 → 0.782 ms` ensemble — un tiers de la frame — et ce
qui reste est l'image que le menu a toujours promise : `present`,
l'attente de la couture, est maintenant la plus grosse ligne unique de la
frame. Elle a été mesurée, elle a été nommée, et ce n'était pas au menu
de la corriger.

### La ligne des 60 fps, vérifiée aussi loin que cette machine mesure honnêtement

Le design D12 dit que cette affirmation ne peut pas être prouvée sur la
machine de rédaction, et dit quoi faire à la place : la vérifier aussi
loin que va la mesure honnête, et dire le reste à voix haute. Donc, dans
l'ordre :

**Ce que cette machine a mesuré.** 2 583 vraies frames du jeu terminé :
**0 au-dessus du budget de 16.667 ms** ; la pire frame de l'exécution
`3.124 ms` — 19 % d'une frame de budget. Même la pire frame jamais
enregistrée dans ces leçons (`4.387 ms`, l'exécution de la leçon 098)
fait 26 % du budget — et cela en `-O0`, la compilation la plus lente que
le cours livre. Le coût de la frame n'est pas près de la ligne.

**Ce qu'est cette machine.** WSL2 sous Windows, un affichage Xvfb, pas de
matériel sonore, l'audio mixé dans le silence — et, le fait de forme qui
gouverne tout : cette boucle est **pilotée par les événements**, se
réveillant sur les nouvelles de X ou sur l'échéance de l'alimentation
audio. Sans périphérique audio pour la cadencher, les exécutions de ce
cours étaient cadencées par des secousses de déplacement de fenêtre à
~25 fps. Cette machine ne peut donc **pas démontrer 60 fps** — elle n'a
jamais tourné à 60 — et aucun nombre ci-dessus ne prétend le contraire.
Ce qu'elle démontre, c'est le *coût* de la frame, et c'est de cela que
60 fps est fait.

**Ce qui reste à vérifier, et où.** « Sur du matériel modeste » est une
affirmation sur *votre* machine, pas sur celle-ci — la ligne de perf de
la checklist est donc aiguillée là où le design D12 dit qu'elle l'est :
vers vous. L'exercice 1 envoie le rapport sur votre matériel avec le seul
instrument qui manque à la moyenne — les percentiles des temps de frame —
pour que la ligne des 60 fps soit répondue par *les frames qui manquent*,
sur une machine qui rend réellement à son propre rythme. Quand ce sera
fait, la ligne sera vérifiée jusqu'au bout. D'ici là elle est vérifiée
exactement jusque-là, et cette phrase est le reste, dit à voix haute.

### Le registre : ce que les passes ont dépensé, et ce qu'elles n'ont pas dépensé

Le menu figé a dépensé ses deux correctifs et s'est arrêté. Tout le reste
de ce que la passe de mesure a nommé reste nommé, mesuré et non touché —
le travail futur, au registre (D11) :

- **la copie de la présentation** — `present 0.438 ms` de temps mural par
  frame, `0.004 ms` de CPU : l'attente de la couture sur le serveur X. Un
  present à double tampon ou MIT-SHM est un changement de *couche
  plateforme* — c'est le second OS de la couture qui l'hébergerait.
- **le format des pixels des tuiles** — la copie de la carte déplace
  3 octets à l'entrée et 4 à la sortie ; des pixels vivant dans l'ordre
  du framebuffer au chargement laisseraient le compilateur l'élargir (le
  recensement de la leçon 099 : il décline aujourd'hui). Un changement de
  disposition qui casse la règle de l'unique boucle de copie — la mesure
  doit le demander.
- **le mixage audio à pleine charge** — `audio 0.033 ms` avec la musique
  et une poignée d'effets ; les seize canaux jouant tous ensemble n'ont
  pas été mesurés.
- **l'update au magasin plein** — `entities 0.005 ms` avec une poignée
  d'entités vivantes ; soixante-quatre étincelles n'ont pas été mesurées.
- **l'impression des rapports eux-mêmes** — les sondes et le rapport du
  HUD s'impriment dans `update` et `text` ; la ligne du journal de frames
  s'imprime hors de toute phase mesurée. Une exécution plus silencieuse
  mesure des phases moins chères.

### Ce que cette exécution a vérifié, et ce qu'elle n'a pas vérifié

- **Le rapport attribue le temps par frame à chaque grand sous-système à
  partir de vraies frames du jeu terminé** — 2 583 frames du jeu
  instrumenté joué sur tout son scénario, chaque ligne une somme mesurée,
  le découpage by state lisant les propres enregistrements de l'exécution.
- **La ligne des 60 fps est vérifiée frame par frame** —
  `0 of 2583 frames` au-dessus du budget — **sur une machine nommée**, et
  la moitié improuvable de l'affirmation (« matériel modeste ») est
  nommée comme non prouvée ici et aiguillée vers l'exercice de portage,
  exactement comme D12 le prescrit.

Ce que cette exécution n'a **pas** vérifié, c'est la ligne de perf de la
checklist sur du vrai matériel — et la définition de « terminé » dit ce
que cela fait de ceci : le jeu terminé est terminé aussi loin que cette
machine mesure honnêtement, le reste est dit à voix haute, et le dernier
mot du menu en trois passes n'est pas un nombre mais une discipline :
mesurer, corriger ce qu'on a mesuré, rapporter ce qu'on n'a pas corrigé.

## Étape de code

Un changement : la clôture du rapport. Le compte de `src/frame.h/.cpp`
grandit avec les lignes propres de la clôture — le découpage by state (le
`step` de l'enregistrement dit quelles frames jouaient), le comptage
frame par frame du budget contre `FRAME_BUDGET_MS` (la frame de 60 fps),
et le nom de la machine imprimé avec les nombres (D12) ; `src/main.cpp`
nomme `RUN_MACHINE` — cette machine, dans une chaîne que le rapport ne
peut pas perdre. Les lignes de la table ne sont pas touchées :
l'attribution était déjà mesurée ; c'est ici qu'elle devient le rapport.
Son état final est étiqueté `lesson-101`.

```diff
diff --git a/src/frame.cpp b/src/frame.cpp
index 349af4f..383b424 100644
--- a/src/frame.cpp
+++ b/src/frame.cpp
@@ -21,13 +21,23 @@ void AccountFrame(FrameStats &stats, const FrameRecord &frame)
     stats.tilemap_sum += frame.tilemap;
     stats.clear_sum += frame.clear;
     stats.entities_sum += frame.entities;
+    /* Lesson 101: the by-state split, and the budget line's count. */
+    if (frame.step > 0.0) {
+        stats.play_frames += 1;
+        stats.play_sum += frame.total;
+    } else {
+        stats.screen_frames += 1;
+        stats.screen_sum += frame.total;
+    }
+    if (frame.total * 1e3 > FRAME_BUDGET_MS)
+        stats.over_budget += 1;
     if (frame.total > stats.worst) {
         stats.worst = frame.total;
         stats.worst_number = frame.number;
     }
 }
 
-void PrintFrameBudget(const FrameStats &stats)
+void PrintFrameBudget(const FrameStats &stats, const char *machine)
 {
     if (!stats.frames)
         return;
@@ -79,6 +89,22 @@ void PrintFrameBudget(const FrameStats &stats)
     std::printf("engine:   present     %6.3f      %2.0f%%\n", present,
                 100.0 * present / (avg * 1e3));
     std::printf("engine:   total       %6.3f     100%%\n", avg * 1e3);
+
+    /* Lesson 101: the final report's close — the frames split by what
+       they were doing, the 60 fps line checked frame by frame, and the
+       machine these numbers belong to (D12). */
+    double play = stats.play_frames ? stats.play_sum / (double)stats.play_frames
+                                    : 0.0;
+    double screen =
+        stats.screen_frames ? stats.screen_sum / (double)stats.screen_frames
+                            : 0.0;
+    std::printf("engine:   by state    %ld play frames at %.3f ms, %ld screen frames at %.3f ms\n",
+                stats.play_frames, play * 1e3, stats.screen_frames,
+                screen * 1e3);
+    std::printf("engine:   budget      60 fps is %.3f ms a frame — %ld of %ld frames over it, worst %.3f ms (%.0f%% of it)\n",
+                FRAME_BUDGET_MS, stats.over_budget, stats.frames,
+                stats.worst * 1e3, 100.0 * stats.worst * 1e3 / FRAME_BUDGET_MS);
+    std::printf("engine:   machine     %s\n", machine ? machine : "(unnamed)");
 }
 
 } /* namespace engine */
diff --git a/src/frame.h b/src/frame.h
index 1234c17..7fb1cd6 100644
--- a/src/frame.h
+++ b/src/frame.h
@@ -59,15 +59,33 @@ struct FrameStats {
     double entities_sum; /* lesson 081: the update's entity work, summed */
     double worst;      /* the longest frame so far */
     long worst_number; /* and which one it was */
+
+    /* Lesson 101: the final report's own account — the frames split by
+       what they were doing (the record's `step` says it: a frame that
+       advanced game time was playing, one at zero was showing a
+       screen), and the count that checks the 60 fps line frame by
+       frame. */
+    long play_frames;
+    double play_sum;
+    long screen_frames;
+    double screen_sum;
+    long over_budget; /* frames that spent more than the 60 fps budget */
 };
 
+/* Lesson 101: the 60 fps frame — the budget the finished game is
+   measured against (the MVD's perf line). One sixtieth of a second. */
+constexpr double FRAME_BUDGET_MS = 1000.0 / 60.0;
+
 void AccountFrame(FrameStats &stats, const FrameRecord &frame);
 
 /* Lesson 058: the frame-budget table — the account, attributed per
    subsystem, as the report Part 5's finale grows. Every number in it is
    a measured sum from the frames that actually ran; the shares are of
-   the average frame. */
-void PrintFrameBudget(const FrameStats &stats);
+   the average frame. Lesson 101: it is the final frame-budget report
+   now — the attribution, the by-state split, the 60 fps budget line,
+   and the machine the numbers came from (D12: a performance claim
+   carries its machine). */
+void PrintFrameBudget(const FrameStats &stats, const char *machine);
 
 } /* namespace engine */
 
diff --git a/src/main.cpp b/src/main.cpp
index 112049a..5ef73e3 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -29,6 +29,16 @@
 
 namespace engine {
 
+/* Lesson 101: the machine these measurements belong to (D12 — a
+   performance claim carries its machine). The report prints it with
+   its numbers; a run on different hardware names different hardware.
+   What this name means for the numbers: a paced headless run (the
+   loop is event-driven; the pacing is window-move jiggles at ~25 fps),
+   a `-O0` build, the audio mixed in silence (no sound device), and the
+   display's copy through the X server. */
+constexpr const char *RUN_MACHINE =
+    "WSL2, Xvfb :99, no sound hardware (the course's authoring machine)";
+
 int Run(void)
 {
     platform::WindowResult opened =
@@ -321,7 +331,7 @@ int Run(void)
        (the frame account's own table) and the arena's. */
     ReportEnd(world.sound, feed, world.store, hero, walk_visits,
               frame_number);
-    PrintFrameBudget(stats);
+    PrintFrameBudget(stats, RUN_MACHINE);
     std::printf("engine: arena: %zu of %zu bytes used\n", world.arena.used,
                 world.arena.memory.size);
 
```

## Exercices

Les deux défis de la clôture — la vérification que cette machine ne peut
pas faire, et le seul nombre que le rapport cache encore. Chacun se
termine par sa solution — un diff contre l'état final de cette leçon,
plus une visite guidée — après l'énoncé.

### Exercice 1 — La ligne des 60 fps sur votre machine *(port-to-your-own-machine)*

Cette machine ne peut pas démontrer 60 fps ; la vôtre le peut. Lancez le
jeu terminé sur votre matériel — une fenêtre sur votre bureau, un
périphérique sonore si vous en avez un, joué à fond — et répondez à la
ligne de perf du MVD avec des frames mesurées. La moyenne n'est pas
l'instrument pour cela : étendez le compte avec les **percentiles** des
temps de frame (la médiane, le 95e, le 99e — un histogramme fixe dans la
mémoire propre du compte, aucune allocation), et rapportez le rapport de
votre machine : la table, les percentiles, la ligne de budget, et le nom
de votre machine là où est le nôtre. Dites ensuite la phrase dont la
checklist a besoin — *ma machine tient 60 fps* ou non — avec les frames
qui la manquent comptées.

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-101/ex1.md)

### Exercice 2 — La pire frame, prédite *(predict-the-output)*

Le rapport nomme le numéro de la pire frame et son coût
(`worst 3.124 ms (frame 1101)`) mais pas sa forme. Avant de lancer quoi
que ce soit, prédisez-la : quel sous-système mange la pire frame d'une
exécution jouée, et à peu près quelle part — le dessin de la carte,
le clear, l'attente de la couture, l'apparition d'une vague dans
l'update ? Faites ensuite attribuer la pire frame au rapport (il se
souvient déjà laquelle c'était — apprenez au compte à se souvenir de *ce*
que c'était) et lancez le même scénario de dix manches : votre prédiction
était-elle juste, dans les nombres du rapport lui-même ? Et répondez au
contrefactuel dans vos mots : si la pire frame est celle de la couture,
qu'est-ce que cela dit de qui possède le correctif ?

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-101/ex2.md)

---

**Partie :** [Partie 5 — le jeu](../../index.md) ·
**Précédente :** [Leçon 100 — passe 2b : corriger le clear](lesson-100-clear.md) ·
**Suivante :** [Leçon 102 — la rétrospective : notre moteur face aux moteurs réels](lesson-102-retrospective.md) ·
**Étiquette de code :** [`lesson-101`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-101)

*Page traduite de la version anglaise `book/lessons/part-5/lesson-101-frame-budget.md`,
révision `4ef60c6`.*

<!-- translation-source: book/lessons/part-5/lesson-101-frame-budget.md @ 4ef60c6 -->
