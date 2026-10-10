# Leçon 077 — le mover sur une entité

{{#include ../../stability-horizon.md}}

## Prose

La leçon 076 s'est terminée sur un héros qui bouge et un monde qui ne répond
pas : le héros scripté traversait les murs et sortait du bord de la carte, et
la leçon le disait tout haut. Un mouvement sans l'avis du monde, c'est la
moitié d'un modèle de déplacement. L'idée de cette leçon est donc l'habitude
que la leçon 056 a commencée, enfin appliquée là où elle appartient : **le
mover sur une entité — le mouvement n'a lieu que là où la carte l'autorise**,
une fonction par laquelle passe le déplacement de chaque entité, pour que le
héros s'arrête aux murs et glisse le long de ceux-ci.

### Un axe à la fois

```cpp
void MoveEntity(const TileMap &map, Entity &entity, double dx, double dy);
```

Tout le mover tient en quatre lignes qui en donnent la forme :

```cpp
double next_x = entity.x + dx;
if (!TileRectSolid(map, (int)next_x, (int)entity.y,
                   entity.sprite->width, entity.sprite->height))
    entity.x = next_x;
double next_y = entity.y + dy;
if (!TileRectSolid(map, (int)entity.x, (int)next_y,
                   entity.sprite->width, entity.sprite->height))
    entity.y = next_y;
```

La requête arrive sous la forme d'un pas en pixels ; chaque axe est essayé
séparément ; un axe dont le pas mettrait l'entité dans une tuile solide n'a
simplement pas lieu. C'est l'habitude du mover de la leçon 056, inchangée — ce
qui est nouveau, c'est qu'elle est *une seule fonction* désormais, qui prend
une entité, et que c'est elle que la marche appelle pour chaque entité. Chaque
entité se résout contre les mêmes requêtes ; le héros n'a pas de mover privé.

Pourquoi « un axe à la fois » mérite sa propre phrase : c'est ce qui fait
fonctionner le **glissement**. Un joueur qui pousse un héros en diagonale
contre un mur s'attend à avancer *le long* du mur, pas à s'arrêter net — et
avec les axes essayés séparément, c'est exactement ce qui se passe : l'axe
vers le mur est refusé, l'axe le long de celui-ci aboutit. Un mover qui
testerait toute la diagonale d'un coup refuserait les deux et le héros
resterait collé à chaque coin de mur.

La requête que pose le mover est `TileRectSolid` — la requête de rectangle de
la leçon 055, répondue depuis les données de la carte elle-même (la solidité
des types), jamais depuis le code de dessin. Le rectangle est l'**art** de
l'entité : la largeur et la hauteur de son sprite à sa position. C'est
délibéré et simple — ce qu'une entité dessine est ce contre quoi elle entre en
collision — et c'est le même rectangle contre lequel l'ancien sprite de démo
était testé. La politique de bord de la carte vient avec (la leçon 055 l'a
définie) : un rectangle hors de la carte compte comme solide, donc le bord du
monde bloque comme un mur et la requête ne lit jamais au-delà des cellules de
la carte.

### Le pas, de bout en bout

Le travail par entité de la marche tient désormais en un appel :

```cpp
MoveEntity(map, e, e.move_x * e.speed * dt, e.move_y * e.speed * dt);
```

lisez-le de droite à gauche : la requête de la frame × la vitesse de la ligne
× le `dt` de la frame forment le pas que l'entité *veut* ; le mover livre le
pas que le monde *autorise*. Rien d'autre dans le moteur ne déplace une entité
— le jeu écrit la requête, la marche appelle le mover, la carte a le dernier
mot.

### Ce que cette exécution a vérifié, et ce qu'elle n'a pas vérifié

De vraies exécutions de l'état final de cette leçon sur cette machine, pilotées
par une entrée scriptée :

- **Le héros s'arrête aux tuiles solides.** En descendant vers le bas de la
  carte : `engine: hero at 706,480 (t=6.366)` puis
  `engine: hero blocked at 706,480 (t=6.406)` — la requête continuait
  d'arriver et la position tenait. 480 + 16 atteint la ligne de bordure, qui
  est un type `#` — solide dans les données de la carte elle-même — et le pas
  qui y entrerait n'a pas lieu. (Le rapport d'état du mover est revenu avec le
  mover : `blocked` / `unblocked` aux transitions, comme celui de la leçon
  056.)
- **Le mouvement le long du mur, lui, fonctionne toujours.** Le glissement de
  la même exécution : `hero at 648,477`, `hero at 653,477`, `hero at
  658,477`, puis `hero blocked at 658,477` (les requêtes Bas, refusées à la
  ligne du bas) et `hero at 663,477`, `hero at 668,477` (les requêtes Droite,
  qui aboutissent) : x avance pendant que y tient. Le héros a glissé le long
  du mur au lieu d'y rester collé — la forme à un axe, visible en deux
  colonnes d'un rapport.
- **Le mouvement est libre là où rien n'est solide.** Les longues courses
  entre les deux : `hero at 419,232` … `hero at 706,232`, un pas par frame
  appuyée, rien de consulté sinon les cellules ouvertes de la carte.

Ce que cette leçon ne change **pas**, c'est quoi que ce soit à la carte : les
requêtes sont utilisées, pas modifiées (la ligne des capacités modifiées de la
proposition de ce changement). Et rien ici n'est du comportement *de jeu* — le
mover est de la géométrie. Ce qu'un héros fait quand il heurte un mur (tourner,
glisser, se faire mal) appartient au jeu ; qu'il ne puisse pas traverser le mur
appartient au mover.

## Étape de code

Un seul changement pour cette leçon, de l'arithmétique à la résolution :
`src/entity.h` / `src/entity.cpp` accueillent `MoveEntity` — l'habitude du
mover de la leçon 056 en une fonction par laquelle passe le déplacement de
chaque entité, un axe à la fois contre les requêtes de collision de la carte.
`src/main.cpp` fait appeler cette fonction par le pas par entité de la marche
(l'arithmétique nue qu'elle remplace tient en une ligne) et ramène le rapport
d'état du mover — `blocked` / `unblocked` aux transitions — désormais à propos
de l'entité du héros. Le magasin, la table, la caméra, le son et les mesures
de la boucle sont intacts. Son état final est étiqueté `lesson-077`.

```diff
diff --git a/src/entity.cpp b/src/entity.cpp
index 04ff1f3..d3c7d3b 100644
--- a/src/entity.cpp
+++ b/src/entity.cpp
@@ -56,4 +56,20 @@ void EntityRetire(EntityStore &store, Entity &entity)
     store.live -= 1;
 }
 
+void MoveEntity(const TileMap &map, Entity &entity, double dx, double dy)
+{
+    /* One axis at a time: a wall blocks the movement into it and the
+       movement along it still works — the slide is this shape, not a
+       special case. The rectangle the map is asked about is the
+       entity's art: what it draws is what it collides as. */
+    double next_x = entity.x + dx;
+    if (!TileRectSolid(map, (int)next_x, (int)entity.y,
+                       entity.sprite->width, entity.sprite->height))
+        entity.x = next_x;
+    double next_y = entity.y + dy;
+    if (!TileRectSolid(map, (int)entity.x, (int)next_y,
+                       entity.sprite->width, entity.sprite->height))
+        entity.y = next_y;
+}
+
 } /* namespace engine */
diff --git a/src/entity.h b/src/entity.h
index bcda6b1..c0ec0e7 100644
--- a/src/entity.h
+++ b/src/entity.h
@@ -11,6 +11,7 @@
 
 #include "sprite.h"
 #include "table.h"
+#include "tilemap.h"
 
 namespace engine {
 
@@ -78,6 +79,15 @@ EntityResult EntityCreate(EntityStore &store, const EntityDef &def);
    been used. Retiring an entity that is already gone is nothing. */
 void EntityRetire(EntityStore &store, Entity &entity);
 
+/* Lesson 077: the mover, applied to entities — the habit lesson 056
+   started, as one function every entity's motion goes through. Intent
+   becomes motion only where the map allows it: the move that would put
+   the entity in a solid tile does not happen, and the movement along the
+   wall still does (one axis at a time, which is what makes the slide
+   work). Empty space is free: the entity arrives at the requested
+   position. */
+void MoveEntity(const TileMap &map, Entity &entity, double dx, double dy);
+
 /* Lesson 075: the walk — the shape the game's per-entity work takes:
 
      for (int i = 0; i < ENTITY_CAP; ++i) {
diff --git a/src/main.cpp b/src/main.cpp
index 2bdcaa7..dbc83e7 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -305,6 +305,7 @@ int Run(void)
     double last = started;
     double distance = 0.0; /* the score: the world the hero has walked */
     int shake_frames = 0; /* lesson 054: the additive hook's demo */
+    bool was_blocked = false; /* lesson 077: the mover's state report */
 
     /* The demo's identity: what the run is, named at once — the hero,
        an entity the game moves, over the world the map draws. */
@@ -415,8 +416,10 @@ int Run(void)
                 continue;
             }
             Entity &e = store.slots[i];
-            e.x += e.move_x * e.speed * dt;
-            e.y += e.move_y * e.speed * dt;
+
+            /* Lesson 077: the mover, on the entity — the request
+               becomes motion only where the map allows it. */
+            MoveEntity(map, e, e.move_x * e.speed * dt, e.move_y * e.speed * dt);
             if (e.move_x > 0.0)
                 e.facing = 0;
             else if (e.move_y > 0.0)
@@ -450,6 +453,17 @@ int Run(void)
            game moves has got to. */
         distance += (hero.x > was_x ? hero.x - was_x : was_x - hero.x) +
                     (hero.y > was_y ? hero.y - was_y : was_y - hero.y);
+
+        /* Lesson 077: the mover's state report, on transitions — the
+           hero moving, or pushed against something that will not move. */
+        bool blocked = (hero.move_x != 0.0 || hero.move_y != 0.0) &&
+                       hero.x == was_x && hero.y == was_y;
+        if (blocked != was_blocked) {
+            std::printf("engine: hero %s at %d,%d (t=%.3f)\n",
+                        blocked ? "blocked" : "unblocked", (int)hero.x,
+                        (int)hero.y, platform::Now() - started);
+            was_blocked = blocked;
+        }
         if ((int)hero.x != (int)was_x || (int)hero.y != (int)was_y)
             std::printf("engine: hero at %d,%d (t=%.3f)\n", (int)hero.x,
                         (int)hero.y, platform::Now() - started);
```

## Exercices

Deux exercices mixtes. Chacun se termine par sa solution — un diff contre
l'état final de cette leçon, plus une visite guidée — après l'énoncé.

### Exercice 1 — Le coin, prédit *(predict-the-output)*

Le mover essaie un axe à la fois, et l'ordre est x d'abord. Placez le héros
dans un coin : des tuiles solides à sa droite et en dessous, de l'espace
ouvert en haut et à gauche — et une frame dont la requête est **Bas-Droite**
avec un pas qui le déplacerait de 20 pixels sur chaque axe. Avant de lancer
quoi que ce soit, notez par écrit la position du héros après cette frame et
lequel des deux composants a bougé. Notez ensuite les deux mêmes réponses
pour une frame dont la requête est **Bas-Gauche** dans le même coin, et pour
**Haut-Droite**. Lancez les trois (une sonde jetable qui appelle `MoveEntity`
directement est la plus propre) et réconciliez — puis dites en une phrase ce
que l'ordre x d'abord signifie pour un héros qui glisse autour d'un coin.

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-077/ex1.md)

### Exercice 2 — Quel mur a dit non *(extend-the-code)*

`blocked` dit que le héros n'a pas bougé ; il ne dit pas quel axe le monde a
refusé. Faites *rapporter* sa résolution au mover — un résultat typé disant
quels composants de la requête ont abouti et lesquels ont été refusés — et
faites utiliser ce résultat par le rapport d'état de l'exécution, pour qu'un
héros poussé vers la gauche contre un mur et un héros poussé vers le bas
contre le sol se lisent différemment. Vérifiez avec une entrée scriptée contre
un mur d'un seul côté. Pourquoi une réponse typée vaut-elle mieux ici que la
comparaison de positions qu'utilise l'exécution aujourd'hui ?

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-077/ex2.md)

---

**Partie :** [Partie 4 — les services](../../index.md) ·
**Précédente :** [Leçon 076 — le héros comme entité](lesson-076-hero.md) ·
**Suivante :** [Leçon 078 — l'échelle du temps de jeu](lesson-078-game-time.md) ·
**Étiquette de code :** [`lesson-077`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-077)

*Page traduite de la version anglaise `book/lessons/part-4/lesson-077-mover.md`,
révision `c8d0b9c`.*

<!-- translation-source: book/lessons/part-4/lesson-077-mover.md @ c8d0b9c -->
