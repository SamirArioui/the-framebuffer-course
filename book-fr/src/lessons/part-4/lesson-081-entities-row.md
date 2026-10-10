# Leçon 081 — le coût de la tranche dans le budget de frames

{{#include ../../stability-horizon.md}}

## Prose

La tranche tourne, et il reste à la partie une dette à payer : son coût
d'exécution. La leçon 070 avait payé la même dette pour le mixeur — la ligne
audio, mesurée plutôt que devinée — et cette leçon est sa jumelle. Le travail
par entité de la tranche est du vrai travail dans une vraie phase, et le budget
de frames n'en dit encore rien : la ligne de l'update est un seul nombre pour
l'intention, la marche et la caméra réunies. L'idée ici est donc le geste
d'attribution que la leçon 046 a fait pour le render, refait pour l'update :
**la phase update nomme son travail d'entités, et le budget gagne une ligne
mesurée sur les frames qui ont réellement tourné.**

### La ligne est à l'intérieur de la phase

`FrameRecord` gagne un temps nommé, et la discipline est celle qui tient depuis
la leçon 046 : **les temps nommés vivent à l'intérieur d'une phase, jamais à
sa place.**

```cpp
    double entities; /* the walk's per-entity step — the store's
                        entities, moved through the mover */
```

`update` reste la phase — tout entière : la lecture de l'entrée, la marche, le
score, le suivi de caméra, mesurés de son début à sa fin. `entities` dit où, à
l'intérieur, le temps de la marche est passé. La table du budget imprime la
ligne comme elle imprime `sprites`, `text` et `tilemap` sous `render` : en
retrait, partagée, et pas comptée deux fois.

La mesure elle-même tient en deux lignes et zéro réflexion : `platform::Now()`
autour de la marche, exactement comme chaque phase depuis la leçon 036. Aucun
modèle de ce que coûte la marche, aucune constante par entité multipliée par un
compte — le nombre est ce que les frames ont pris. C'est toute la raison d'être
de l'enregistrement de frame dans cette partie.

### Mesuré, pas deviné

L'affirmation « les nombres sont de vraies mesures » est vérifiable comme la
leçon 070 a vérifié sa ligne : **la moyenne de la colonne du journal de frames
reproduit la ligne de la table du budget.** D'après une vraie exécution de
l'état final de cette leçon — la tranche, pilotée par une entrée scriptée,
121 frames :

```
engine: frame budget — 121 frames, avg 1.830 ms, worst 2.492 ms (frame 58)
engine:   subsystem   avg ms    share
engine:   update       0.012       1%
engine:     entities   0.001       0%
engine:   audio        0.000       0%
engine:   render       1.347      74%
engine:     sprites    0.002       0%
engine:     text       0.008       0%
engine:     tilemap    0.933      51%
engine:   present      0.472      26%
engine:   total        1.830     100%
```

et les 121 lignes `frame N:` de la même exécution, moyennées :

```
log averages:  update 0.012  entities 0.001  total 1.830 ms
```

Ligne pour ligne, chiffre pour chiffre : `update 0.012`, `entities 0.001`,
`total 1.830` — la table est l'arithmétique du journal, et le journal est les
frames. Rien dans cette table n'a été estimé.

### Ce que la ligne dit, et ce qu'elle ne dit pas

Les nombres ci-dessus sont le prix honnête de la tranche : deux entités,
parcourues une fois chacune par frame, coûtent un millième de milliseconde —
0.001 ms d'une frame de 1.830 ms. Le coût de la scène est là où il a toujours
été : la marche de la tilemap (0.933 ms) et la présentation (0.472 ms). Un jeu
à deux entités n'est pas encore un jeu d'entités, et la ligne existe pour que,
le jour où il le sera — les écrans de la partie 5 avec des dizaines d'ennemis,
de projectiles et de rafales —, le nombre bouge en public.

Ce que la ligne ne dit *pas*, c'est ce qu'est le reste d'`update`. Les 0.012 ms
d'update moins les 0.001 ms d'entities, ce sont la lecture de l'entrée,
l'intention du héros, le score, le bornage de la caméra et les rapports — le
travail de frame propre au jeu, délibérément *non* attribué aux entités, parce
que rien de tout cela n'est par entité. Cette frontière est la définition de la
ligne : **la marche est le travail des entités** ; ce qui l'entoure est celui
du jeu. Si une leçon future ajoute du comportement par entité (l'IA de la
partie 5), il va à l'intérieur de la marche et à l'intérieur de cette ligne —
exactement la visibilité pour laquelle la ligne existe.

### Ce que cette exécution a vérifié, et ce qu'elle n'a pas vérifié

- **Les nombres rapportés sont de vraies mesures des frames de la tranche.**
  La réconciliation table-contre-journal ci-dessus : 121 frames, la colonne
  `entities` moyennée à `0.001 ms`, `update` à `0.012 ms`, `total` à
  `1.830 ms` — et l'arithmétique propre de la frame se referme toujours
  (`update + audio + render + present = total`).
- **L'attribution est à l'intérieur de la phase.** Les lignes s'additionnent
  au total exactement comme avant l'apparition de la ligne — `entities`
  partage le temps d'`update`, elle ne s'y ajoute pas.
- **La ligne n'est que du travail par entité.** Le chronométrage de la marche
  encadre la boucle et rien d'autre — aucune entrée, aucune caméra, aucun
  rapport.

Ce que cette leçon ne fait **pas**, c'est prédire quoi que ce soit des coûts de
la partie 5. La ligne est un instrument ; quand le jeu contiendra cinquante
entités, la ligne dira ce que coûtent cinquante entités, sur la machine où
elles les coûtent. C'est la discipline que ce cours appelle « la chaîne
d'outils fait partie du programme », et c'est la note sur laquelle la partie 4
se termine.

## Étape de code

Un changement pour cette leçon, d'un nombre à deux : `src/frame.h` gagne
`FrameRecord.entities` et sa somme dans `FrameStats` ; `src/frame.cpp` imprime
la ligne sous `update`, en retrait comme les noms du render ; `src/main.cpp`
chronomètre la marche — `platform::Now()` autour de la boucle — et imprime la
colonne dans le journal de frames à côté du nombre propre de l'update. La
tranche elle-même n'est pas touchée : même jeu, mêmes frames, un fait nommé de
plus. Son état final est étiqueté `lesson-081`.

```diff
diff --git a/src/frame.cpp b/src/frame.cpp
index 6868e33..444b1c0 100644
--- a/src/frame.cpp
+++ b/src/frame.cpp
@@ -19,6 +19,7 @@ void AccountFrame(FrameStats &stats, const FrameRecord &frame)
     stats.sprites_sum += frame.sprites;
     stats.text_sum += frame.text;
     stats.tilemap_sum += frame.tilemap;
+    stats.entities_sum += frame.entities;
     if (frame.total > stats.worst) {
         stats.worst = frame.total;
         stats.worst_number = frame.number;
@@ -49,10 +50,15 @@ void PrintFrameBudget(const FrameStats &stats)
     double sprites = stats.sprites_sum / n * 1e3;
     double text = stats.text_sum / n * 1e3;
     double tilemap = stats.tilemap_sum / n * 1e3;
+    double entities = stats.entities_sum / n * 1e3;
     double present = stats.present_sum / n * 1e3;
     std::printf("engine:   subsystem   avg ms    share\n");
     std::printf("engine:   update      %6.3f      %2.0f%%\n", update,
                 100.0 * update / (avg * 1e3));
+    /* Lesson 081: the update's entity work, named inside the phase it
+       lives in — measured from the frames that ran, like every row. */
+    std::printf("engine:     entities  %6.3f      %2.0f%%\n", entities,
+                100.0 * entities / (avg * 1e3));
     std::printf("engine:   audio       %6.3f      %2.0f%%\n", audio,
                 100.0 * audio / (avg * 1e3));
     std::printf("engine:   render      %6.3f      %2.0f%%\n", render,
diff --git a/src/frame.h b/src/frame.h
index 45d4d88..b05bf68 100644
--- a/src/frame.h
+++ b/src/frame.h
@@ -23,10 +23,14 @@ struct FrameRecord {
     /* Lesson 046: the render phase starts naming what is inside it — one
        field per subsystem, the attribution the frame-budget table
        (lesson 058) grows from. The named times are inside render, never
-       instead of it: render stays the phase, these say where it went. */
+       instead of it: render stays the phase, these say where it went.
+       Lesson 081: update grows the same kind of name — the entity work
+       the walk does, attributed inside the phase it lives in. */
     double sprites; /* sprite draws through the blit */
     double text;    /* lesson 051: text drawing — glyphs through the blit */
     double tilemap; /* lesson 053: the map's walk — tiles through the blit */
+    double entities; /* lesson 081: the walk's per-entity step — the
+                        store's entities, moved through the mover */
 
     /* Lesson 079: the game-time step this frame advanced the simulation
        by — not a duration. Every field above is wall-clock, at any
@@ -47,6 +51,7 @@ struct FrameStats {
     double sprites_sum; /* lesson 046's named sub-phase, summed like the rest */
     double text_sum;
     double tilemap_sum;
+    double entities_sum; /* lesson 081: the update's entity work, summed */
     double worst;      /* the longest frame so far */
     long worst_number; /* and which one it was */
 };
diff --git a/src/main.cpp b/src/main.cpp
index edb88b1..9bc7e46 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -446,6 +446,7 @@ int Run(void)
            going. Lesson 080: the walk is the game's now, and its work is
            the world's — no demo scaffolding, no per-kind branches. */
         int visited = 0;
+        double t_entities = platform::Now();
         for (int i = 0; i < ENTITY_CAP; ++i) {
             if (!store.slots[i].live)
                 continue;
@@ -461,6 +462,7 @@ int Run(void)
             else if (e.move_y < 0.0)
                 e.facing = 3;
         }
+        frame.entities = platform::Now() - t_entities;
         walk_visits += visited;
 
         /* The score, and the hero's own report: where the entity the
@@ -654,8 +656,9 @@ int Run(void)
            phase (lesson 060) joins in the record's own order, and
            lesson 079's step leads it: the game's advance beside the
            machine's durations. */
-        std::printf("frame %ld: step %.3f ms, update %.3f ms, audio %.3f ms, render %.3f ms (sprites %.3f, text %.3f, tilemap %.3f), present %.3f ms, total %.3f ms\n",
+        std::printf("frame %ld: step %.3f ms, update %.3f ms (entities %.3f), audio %.3f ms, render %.3f ms (sprites %.3f, text %.3f, tilemap %.3f), present %.3f ms, total %.3f ms\n",
                     frame.number, frame.step * 1e3, frame.update * 1e3,
+                    frame.entities * 1e3,
                     frame.audio * 1e3,
                     frame.render * 1e3,
                     frame.sprites * 1e3, frame.text * 1e3,
```

## Exercices

Deux exercices mixtes. Chacun se termine par sa solution — un diff contre
l'état final de cette leçon, plus une visite guidée — après l'énoncé.

### Exercice 1 — La ligne, à deux tailles *(measure-the-performance)*

Deux entités coûtent 0.001 ms ; que fait la ligne quand le jeu en contient
plus ? Ajoutez des lignes à `assets/entities.txt` — c'est de la donnée, pas du
code — jusqu'à ce que le magasin contienne 2, 20 et 200 entités (augmentez
`ENTITY_CAP` si votre 200 en a besoin), et mesurez la ligne `entities` à chaque
taille sur votre machine. Le coût de la marche est-il linéaire en le nombre
d'entités ? Réconciliez une des trois exécutions contre son propre journal de
frames comme le fait cette leçon. Répondez ensuite : à vos nombres, combien
d'entités le jeu peut-il parcourir en 0.1 ms — et la marche des entités est-elle
la ligne qui importera un jour ?

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-081/ex1.md)

### Exercice 2 — Ce que la ligne ne dit pas *(explain-in-prose)*

La frontière de la ligne est une définition : la marche est le travail des
entités, et ce qui l'entoure est celui du jeu. Défendez cette frontière dans
vos propres mots : qu'y a-t-il exactement dans `update` en dehors d'`entities`
dans la frame de ce jeu, et pourquoi ces choses-là ne devraient-elles pas être
attribuées aux entités ? Mettez-la ensuite à l'épreuve : nommez une pièce de
travail par entité qui vous tenterait de la mesurer en dehors de la marche
(recherche de chemin ? animation ? déclencheurs de son ?), et dites où son
temps appartient et pourquoi. Enfin — la question de mesure — qu'est-ce qu'une
ligne de 0.001 ms ne peut *pas* vous dire des coûts du jeu, et que mesureriez-vous
à la place pour l'apprendre ?

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-081/ex2.md)

---

**Partie :** [Partie 4 — les services](../../index.md) ·
**Précédente :** [Leçon 080 — la tranche verticale](lesson-080-slice.md) ·
**Suivante :** [Partie 5 — le jeu](../../index.md) ·
**Étiquette de code :** [`lesson-081`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-081)

*Page traduite de la version anglaise `book/lessons/part-4/lesson-081-entities-row.md`,
révision `15b1166`.*

<!-- translation-source: book/lessons/part-4/lesson-081-entities-row.md @ 15b1166 -->
