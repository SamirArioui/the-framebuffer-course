# Leçon 080 — la tranche verticale

{{#include ../../stability-horizon.md}}

## Prose

C'est la leçon vers laquelle la partie 4 marche depuis le début, et elle est
délibérément petite : **un héros parcourt la tilemap, la caméra le suit, en
n'utilisant que des services terminés.** Pas une fonctionnalité — une *porte*.
Le MVD la nomme L0\*, et sa règle est toute la leçon : l'exécution n'introduit
aucun comportement nouveau, et si elle n'est pas triviale, un service manque et
la partie 4 n'est pas terminée. Chaque partie jusqu'ici a prouvé son propre
sous-système. Celle-ci est la preuve qu'ils se composent.

### Ce qu'est le jeu

L'exécution est la forme du jeu. Lisez une de ses frames et chaque ligne
renvoie à une leçon déjà terminée :

| La frame fait | Le service | Depuis |
| -------------- | ----------- | ----- |
| le monde, ce sont les lignes de la table, une entité par définition | la table d'archétypes, `EntityCreate` | 071-074 |
| l'intention du joueur est l'état d'entrée par scrutation | les touches de la couture | 032 |
| chaque entité vivante est parcourue une fois, dans l'ordre des emplacements | le magasin | 075 |
| la requête de chaque entité devient du mouvement à travers le mover | `MoveEntity` | 056, 077 |
| le pas vaut `request × speed × dt` | les champs de l'entité, le temps de jeu | 076-078 |
| la base de la caméra suit le héros, bornée à la carte | la caméra, les limites de la carte | 052-054 |
| chaque entité est dessinée à sa position à travers la caméra | le blit, la marche de dessin | 045, 076 |
| l'enregistrement mesure les phases et le pas | l'enregistrement de frame | 036, 079 |

Le démarrage raconte la même histoire : la table se charge (complète ou
nommée), l'art des définitions se charge et leur est remis, le jeu demande son
héros par son nom et fait apparaître le monde depuis les mêmes lignes, et le
bouton est sur lecture. Deux entités dans le magasin — le héros et le type
d'ennemi que le fichier nomme — et un jeu qui grandirait par *lignes*, pas par
code.

### Ce que la tranche invente : rien

L'étape de code de cette leçon est surtout faite de suppressions. L'échafaudage
de démo dont les leçons précédentes avaient besoin — le script de création qui
remplissait le magasin pour tester le refus, la vérification de mise à mort qui
rendait le retrait visible, le script de réutilisation qui prouvait l'ordre des
emplacements — n'est pas celui du jeu, et la tranche le retire. Ce qui reste,
c'est le travail par entité de la marche tel que le jeu le porte : le pas de
l'entité, résolu contre le monde. Aucune branche par type, aucun cas particulier
pour le héros (l'intention du joueur est écrite dans la requête du héros avant
la marche, exactement comme l'IA de la partie 5 écrira dans celles de ses
entités), et aucune nouvelle fonction du moteur.

C'est le test de la porte, et il vaut la peine d'être précis sur ce que
« trivial » veut dire ici. La *composition* de la tranche est triviale : la
frame est la même que celle laissée par la leçon 077, avec le même mover, la
même marche, la même caméra. Si l'étape de code d'aujourd'hui avait eu besoin
d'une nouvelle fonction du moteur — une fonction auxiliaire de « suivi de
caméra », un chemin pour « faire apparaître le héros », un cas particulier de
mouvement — cette fonction serait le service manquant, et la leçon l'aurait
nommée au lieu de la livrer. La revue de clôture le consigne : la tranche n'a
**rien** inventé, et ce qu'elle a retiré, c'est l'échafaudage de la démo (voir
`plan/part4-review.md`).

### Ce que cette exécution a vérifié, et ce qu'elle n'a pas vérifié

D'après une vraie exécution de l'état final de cette leçon sur cette machine —
l'affichage sans écran, l'entrée scriptée, et l'arithmétique vérifiée contre
les rapports de l'exécution elle-même :

- **La tranche tourne.** `engine: part 4 done — the vertical slice: a hero
  walks the tilemap, the camera follows` — et le monde duquel elle part est
  celui de la table : `engine: world: 2 entities from the table's rows, live
  2 of 64`.
- **La position du héros et la base de la caméra se réconcilient avec les
  limites de la carte.** Chaque rapport `camera base` de l'exécution a été
  vérifié contre la position du héros à cette frame par le bornage que la
  leçon 054 définit — `clamp(hero + art/2 − frame/2, 0, map·TILE − frame)` —
  et chacun s'accorde. Les nombres de l'exécution elle-même montrent les deux
  moitiés de la règle : `camera base 120,0`, `123,0` tandis que le héros marche
  vers la droite (la vue qui suit), et `128,0` — la limite de défilement de la
  carte elle-même, `48·16 − 640` — où le bornage retient la vue pendant que le
  héros continue de marcher. La colonne y est bornée à `32` (`32·16 − 480`) de
  la même façon.
- **Chaque entité est parcourue et dessinée une fois par frame.** Le compte :
  `engine: walk: 372 visits over 186 frames` — 2 entités × 186 frames, au
  chiffre près.
- **Le temps de jeu est dans la frame.** Le pas passe par `GameTimeStep` et le
  script de la démo tourne le bouton (les trois réglages de la leçon 078) — la
  tranche compose le service ; elle ne le réimplémente pas.

Ce que cette leçon ne vérifie **pas**, c'est si la tranche *se sent* bien —
cela n'est pas vérifiable sans écran, et la conception le route là où il
appartient : vos mains, sur votre machine (l'exercice ci-dessous). Ce qui *est*
vérifiable, c'est que le héros marche, que la caméra suit, que les nombres se
réconcilient et que rien de nouveau n'a été inventé.

## Étape de code

Un changement pour cette leçon, de la démo au jeu : `src/main.cpp` compose les
services terminés en la tranche. Le monde apparaît depuis les lignes de la
table — le héros par son nom, les autres types une entité par ligne — et le
travail par entité de la marche est le pas de l'entité, la vérification de
mise à mort et les scripts de durée de vie de la démo retirés avec
l'échafaudage qu'ils étaient. La ligne d'identité nomme la porte que cette
leçon ferme. Les fichiers du moteur ne sont pas touchés — `entity.*`,
`table.*`, `gametime.*`, `frame.*` et la couture plateforme restent exactement
tels que les leçons précédentes les ont laissés. Son état final est étiqueté
`lesson-080`.

```diff
diff --git a/src/main.cpp b/src/main.cpp
index 1904f32..edb88b1 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -239,24 +239,27 @@ int Run(void)
                 hero.name, hero.x, hero.y, hero.facing, hero.speed, hero.health,
                 hero.sprite->width, hero.sprite->height);
 
-    /* Lesson 074: the creation policy — the first free slot, and never
-       a live entity's. Lesson 075: the demo keeps a small world — eight
-       entities, three of them killed before the first walk — so the
-       store has free slots and the report can tell a freed slot from one
-       that has never been used. */
-    int created = 0;
-    while (store.live < 8) {
-        EntityResult made =
-            EntityCreate(store, table.rows[store.live % table.count]);
-        if (made.error != ENTITY_OK)
-            break;
+    /* Lesson 080: the vertical slice — the game's shape, and nothing
+       else. The hero is the row the game asks for by name (it is the
+       one the player controls); the world's other kinds come from the
+       same table, one entity per row. A new row is a new entity; the
+       run has no per-kind code to grow. */
+    int created = 1;
+    for (int i = 0; i < table.count; ++i) {
+        if (&table.rows[i] == hero_def.def)
+            continue;
+        EntityResult made = EntityCreate(store, table.rows[i]);
+        if (made.error != ENTITY_OK) {
+            std::fprintf(stderr, "engine: the store refused %s\n",
+                         table.rows[i].name);
+            platform::CloseWindow(opened.window);
+            ArenaRelease(arena);
+            return 1;
+        }
         created += 1;
     }
-    store.slots[2].health = 0;
-    store.slots[4].health = 0;
-    store.slots[6].health = 0;
-    std::printf("engine: store: live %d of %d — the hero and %d from the script; slots 2, 4, 6 killed\n",
-                store.live, ENTITY_CAP, created);
+    std::printf("engine: world: %d entities from the table's rows, live %d of %d\n",
+                created, store.live, ENTITY_CAP);
 
     /* The lookup's typed failure, checked on purpose: a definition the
        table does not hold is a value — never an entity with assumed
@@ -310,9 +313,10 @@ int Run(void)
     int scale_phase = 0;  /* lesson 078: the demo's script, by wall seconds */
     bool was_blocked = false; /* lesson 077: the mover's state report */
 
-    /* The demo's identity: what the run is, named at once — the hero,
-       an entity the game moves, over the world the map draws. */
-    std::printf("engine: the hero, as an entity — a row the game moves, a camera that follows\n");
+    /* The slice's identity: what the run is, named at once — L0*, the
+       gate this part closes on. Every service it uses was finished
+       before this lesson; the lesson is the fit. */
+    std::printf("engine: part 4 done — the vertical slice: a hero walks the tilemap, the camera follows\n");
     std::printf("engine: world %dx%d cells (%dx%d px), %d kinds; %d glyphs; hero %dx%d\n",
                 map.width, map.height, map.width * TILE_SIZE,
                 map.height * TILE_SIZE, map.kind_count, FONT_COUNT,
@@ -437,26 +441,16 @@ int Run(void)
 
         /* Lesson 075: the walk — every live entity, once per frame, in
            slot order. The per-entity work is expressed here, once, and
-           not per type; lesson 076 makes it the entity's step — its
-           movement request becomes motion, and its facing follows where
-           it is going. An entity retired in passing is not visited again
-           and no other is skipped — the slots do not move under the
-           walk. */
+           not per type: the entity's step — its movement request becomes
+           motion through the mover, and its facing follows where it is
+           going. Lesson 080: the walk is the game's now, and its work is
+           the world's — no demo scaffolding, no per-kind branches. */
         int visited = 0;
         for (int i = 0; i < ENTITY_CAP; ++i) {
             if (!store.slots[i].live)
                 continue;
             visited += 1;
-            if (store.slots[i].health <= 0) {
-                std::printf("engine: walk (frame %ld): retiring slot %d in passing\n",
-                            frame.number, i);
-                EntityRetire(store, store.slots[i]);
-                continue;
-            }
             Entity &e = store.slots[i];
-
-            /* Lesson 077: the mover, on the entity — the request
-               becomes motion only where the map allows it. */
             MoveEntity(map, e, e.move_x * e.speed * dt, e.move_y * e.speed * dt);
             if (e.move_x > 0.0)
                 e.facing = 0;
@@ -468,24 +462,6 @@ int Run(void)
                 e.facing = 3;
         }
         walk_visits += visited;
-        if (frame_number == 1)
-            std::printf("engine: walk (frame 1): visited %d live entities, once each, in slot order; live %d of %d\n",
-                        visited, store.live, ENTITY_CAP);
-
-        /* Lesson 075: the reuse — three requests once the walk has
-           freed three slots. Each lands in a freed slot, before any
-           slot that has never been used. */
-        if (frame_number == 2) {
-            for (int k = 0; k < 3; ++k) {
-                EntityResult made =
-                    EntityCreate(store, table.rows[k % table.count]);
-                if (made.error != ENTITY_OK)
-                    break;
-                std::printf("engine: store: created in slot %d (freed before never-used)\n",
-                            (int)(made.entity - store.slots));
-            }
-            std::printf("engine: store: live %d of %d\n", store.live, ENTITY_CAP);
-        }
 
         /* The score, and the hero's own report: where the entity the
            game moves has got to. */
```

## Exercices

Deux exercices mixtes. Chacun se termine par sa solution — un diff contre
l'état final de cette leçon, plus une visite guidée — après l'énoncé.

### Exercice 1 — La caméra aux bords de la carte *(predict-the-output)*

La base de la caméra vaut `clamp(hero + art/2 − frame/2, 0, map·TILE −
frame)` — et cette carte défile de 128 pixels horizontalement et de 32
verticalement. Avant de lancer quoi que ce soit, notez la base pour chacune de
ces positions du héros : `0,0` ; `400,232` ; `734,480` ; `312,232` ; et
`734,232`. Lesquelles de vos réponses sont le bornage qui parle et lesquelles
la vue qui suit ? Lancez ensuite une sonde jetable qui place le héros à chaque
position (et déplace la caméra comme le fait l'update) et réconciliez.

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-080/ex1.md)

### Exercice 2 — Réponse à la porte *(explain-in-prose)*

La question de la porte est : « la tranche a-t-elle inventé quoi que ce
soit ? » Répondez-y dans vos propres mots, dans deux directions. Premièrement :
nommez chaque ligne de la frame de la tranche qui n'est *pas* un service
terminé — et si vous n'en trouvez aucune, dites pourquoi c'est une preuve
plutôt qu'une politesse. Deuxièmement : le contrefactuel — si la tranche
*avait* eu besoin d'une nouvelle pièce de code moteur pour s'emboîter (une
fonction auxiliaire, un cas particulier, un nouveau champ), que vous dirait-elle
sur le service qu'elle toucherait, et que feriez-vous à ce sujet avant que la
partie 5 ne commence ? Pour rendre la première moitié vérifiable, faites nommer
par l'exécution les services qu'elle compose — une ligne par groupe, comme la
démo de la leçon 069 nommait ses capacités — et remplissez la liste
d'acceptation en dessous.

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-080/ex2.md)

---

**Partie :** [Partie 4 — les services](../../index.md) ·
**Précédente :** [Leçon 079 — la mesure n'est pas mise à l'échelle](lesson-079-wall-clock.md) ·
**Suivante :** [Leçon 081 — le coût de la tranche dans le budget de frames](lesson-081-entities-row.md) ·
**Étiquette de code :** [`lesson-080`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-080)

*Page traduite de la version anglaise `book/lessons/part-4/lesson-080-slice.md`,
révision `7d3e8c3`.*

<!-- translation-source: book/lessons/part-4/lesson-080-slice.md @ 7d3e8c3 -->
