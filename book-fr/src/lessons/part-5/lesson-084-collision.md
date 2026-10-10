# Leçon 084 — la collision avec les tuiles

{{#include ../../stability-horizon.md}}

## Prose

Le monde défile et la caméra le suit. Mais le héros traverse encore les murs
comme s'ils étaient peints dessus. Cette leçon s'occupe de l'autre moitié du
mouvement : **le jeu résout le mouvement de chaque entité contre la tilemap —
une entité s'arrête sur une tuile solide et glisse le long d'un mur.**

La requête de collision n'est pas nouvelle : la leçon 055 a donné à la tilemap
`TileRectSolid` (ce rectangle est-il sur une tuile solide — ou hors du bord de
la carte, ce qui compte comme solide ?), et la leçon 077 l'a mise au travail
dans le mover, `MoveEntity`. Ce qui est nouveau, c'est que le *jeu* résout son
mouvement à travers elle — la marche est au jeu désormais (`GameWalk`, dans
`game.cpp`), et elle transforme la requête de déplacement de chaque entité
vivante en mouvement contre la carte. Le héros et chaque autre entité se
résolvent de la même façon.

### Un axe à la fois — voilà le glissement

`MoveEntity` est petit, et toute son astuce tient dans l'ordre dans lequel il
travaille :

```cpp
double next_x = entity.x + dx;
if (!TileRectSolid(map, (int)next_x, (int)entity.y, w, h))
    entity.x = next_x;              // the x step, on its own
double next_y = entity.y + dy;
if (!TileRectSolid(map, (int)entity.x, (int)next_y, w, h))
    entity.y = next_y;              // then the y step, on its own
```

Il résout **un axe à la fois**. Prenez le pas en x seul : si l'entité devait
atterrir sur une tuile solide en se déplaçant en x, elle ne se déplace
simplement pas en x — mais cela ne dit rien du y. Prenez ensuite le pas en y
seul, contre le x *d'origine* de l'entité. Si y est libre, l'entité se déplace
en y même si x a été refusé.

Voilà le glissement. Une entité poussée en diagonale contre un mur a un axe
refusé et l'autre libre — elle glisse donc *le long* du mur au lieu de
s'arrêter net. Et une entité poussée dans un coin a les deux axes refusés — elle
s'arrête. S'arrêter et glisser ne sont pas deux comportements ; c'est la même
règle d'un axe à la fois, dans un coin contre le long d'un mur.

### Le héros et chaque entité, résolus de la même façon

La marche (`GameWalk`) est la résolution de mouvement du jeu : chaque entité
vivante, une fois par frame, sa requête `move_x` / `move_y` transformée en `dx`
/ `dy` à travers le mover. Il n'y a pas de code de collision par type — le héros
et un slime qui marche passent par le même `MoveEntity`. D'après une vraie
exécution de l'état final de cette leçon, le héros, conduit en haut à gauche par
une entrée scriptée sous l'affichage sans écran :

```
engine: hero at 312,232
engine: hero at 210,232 (t=3.447)
engine: hero blocked at 206,228 (t=6.477)
```

Lisez la ligne du milieu : le héros est passé de `312` à `210` en x pendant que
son `y` restait à `232` — **il a glissé vers la gauche le long d'un mur**, le
pas en y refusé (la tuile au-dessus était solide) pendant que le pas en x était
libre. Puis `hero blocked at 206,228` : poussé là où les deux axes rencontrent
une tuile solide, il s'est arrêté — il n'est pas entré dans le mur. Arrêt et
glissement, tous deux issus de la règle d'un axe à la fois.

L'entité non-héros de l'exécution — le slime, doté d'une marche (vers le
bas-droite) comme substitut de l'IA que la leçon 089 apporte — se résout à
travers le même `GameWalk` / `MoveEntity` : il glisse le long des murs qu'il
rencontre et s'arrête aux tuiles solides qu'il ne peut pas franchir. La marche
rapporte une visite par entité vivante et par frame, et il n'y a aucune branche
qui traite le héros différemment.

### Ce que cette exécution a vérifié, et ce qu'elle n'a pas vérifié

- **Le héros s'arrête aux tuiles solides** — `hero blocked at 206,228` : poussé
  contre des murs sur les deux axes, il n'est pas entré dedans.
- **Le héros glisse le long des murs** — `312 → 210` à un `y = 232` figé : un
  axe refusé, l'autre libre, du mouvement le long du mur.
- **Une entité se résout de la même façon** — la marche transforme la requête
  de chaque entité en mouvement à travers le même mover ; le slime qui marche
  glisse et s'arrête par le code identique.

Ce que cette leçon ne fait **pas**, c'est donner au mouvement le bon *feel* —
la vitesse du héros saute encore de zéro à plein en un pas, et un héros très
rapide sur une frame très lente pourrait en principe faire un long pas en une
frame. C'est le feel du héros (leçon 085) et la robustesse du mover ; cette
leçon n'est que la résolution : la carte est solide, et le jeu la traite comme
telle.

## Étape de code

Un changement : la résolution de mouvement devient celle du jeu. `src/game.h`
déclare `GameWalk` ; `src/game.cpp` l'implémente — la marche par entité qui
transforme la requête de déplacement de chaque entité vivante en mouvement à
travers le mover (`MoveEntity`), un axe à la fois, le facing suivant où l'entité
va. `src/main.cpp` abandonne sa boucle de marche en ligne et appelle le
`GameWalk` du jeu (toujours chronométré comme la sous-phase `entities` de
l'enregistrement de frame), et donne une marche (vers le bas-droite) à une
entité non-héros à l'apparition — un substitut de l'IA que la leçon 089 apporte,
pour qu'une entité non-héros soit résolue contre la carte ici aussi. Son état
final est étiqueté `lesson-084`.

```diff
diff --git a/src/game.cpp b/src/game.cpp
index a4371f7..0a78d42 100644
--- a/src/game.cpp
+++ b/src/game.cpp
@@ -243,4 +243,32 @@ void GameDrawSprites(const Game &game, Framebuffer &fb,
     }
 }
 
+int GameWalk(EntityStore &store, const TileMap &map, double dt)
+{
+    /* Lesson 084: the walk — every live entity, once per frame, in slot
+       order, its movement resolved against the tilemap. The per-entity
+       work is expressed once here, not per type: the mover (MoveEntity)
+       turns the request into motion one axis at a time, so an entity
+       that meets a solid tile stops on that axis and slides along the
+       wall on the other — and the facing follows where it is going.
+       The hero and every other entity resolve the same way. */
+    int visited = 0;
+    for (int i = 0; i < ENTITY_CAP; ++i) {
+        if (!store.slots[i].live)
+            continue;
+        visited += 1;
+        Entity &e = store.slots[i];
+        MoveEntity(map, e, e.move_x * e.speed * dt, e.move_y * e.speed * dt);
+        if (e.move_x > 0.0)
+            e.facing = 0;
+        else if (e.move_y > 0.0)
+            e.facing = 1;
+        else if (e.move_x < 0.0)
+            e.facing = 2;
+        else if (e.move_y < 0.0)
+            e.facing = 3;
+    }
+    return visited;
+}
+
 } /* namespace engine */
diff --git a/src/game.h b/src/game.h
index 327b088..a031ef7 100644
--- a/src/game.h
+++ b/src/game.h
@@ -100,6 +100,12 @@ void GameDrawMap(const Game &game, Framebuffer &fb, const TileMap &map,
 void GameDrawSprites(const Game &game, Framebuffer &fb,
                      const EntityStore &store);
 
+/* Lesson 084: the walk — the game resolves every live entity's movement
+   against the tilemap. Each entity's movement request becomes motion
+   through the mover (MoveEntity), one axis at a time, so it stops at a
+   solid tile and slides along a wall. Returns the visit count. */
+int GameWalk(EntityStore &store, const TileMap &map, double dt);
+
 } /* namespace engine */
 
 #endif
diff --git a/src/main.cpp b/src/main.cpp
index 60b4442..fda43b0 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -264,6 +264,12 @@ int Run(void)
             ArenaRelease(arena);
             return 1;
         }
+        /* Lesson 084: a non-hero entity walks (down-right) so its
+           movement is resolved against the map like the hero's — a stand
+           in for the AI lesson 089 brings. The walk slides it along
+           walls and stops it at solid tiles. */
+        made.entity->move_x = 1.0;
+        made.entity->move_y = 1.0;
         created += 1;
     }
     std::printf("engine: world: %d entities from the table's rows, live %d of %d\n",
@@ -413,29 +419,12 @@ int Run(void)
 
         double was_x = hero.x, was_y = hero.y;
 
-        /* Lesson 075: the walk — every live entity, once per frame, in
-           slot order. The per-entity work is expressed here, once, and
-           not per type: the entity's step — its movement request becomes
-           motion through the mover, and its facing follows where it is
-           going. Lesson 080: the walk is the game's now, and its work is
-           the world's — no demo scaffolding, no per-kind branches. */
-        int visited = 0;
+        /* Lesson 084: the game resolves its movement against its map —
+           the walk is the game's now (GameWalk, in game.cpp), turning
+           every live entity's request into motion through the mover.
+           The loop times it as the frame record's entity sub-phase. */
         double t_entities = platform::Now();
-        for (int i = 0; i < ENTITY_CAP; ++i) {
-            if (!store.slots[i].live)
-                continue;
-            visited += 1;
-            Entity &e = store.slots[i];
-            MoveEntity(map, e, e.move_x * e.speed * dt, e.move_y * e.speed * dt);
-            if (e.move_x > 0.0)
-                e.facing = 0;
-            else if (e.move_y > 0.0)
-                e.facing = 1;
-            else if (e.move_x < 0.0)
-                e.facing = 2;
-            else if (e.move_y < 0.0)
-                e.facing = 3;
-        }
+        int visited = GameWalk(store, map, dt);
         frame.entities = platform::Now() - t_entities;
         walk_visits += visited;
 
```

## Exercices

Deux défis plus conséquents. Chacun se termine par sa solution — un diff contre
l'état final de cette leçon, plus une visite guidée — après l'énoncé.

### Exercice 1 — Le coin et le mur *(predict-the-output)*

`MoveEntity` résout x d'abord, puis y. Servez-vous-en pour prédire, avant de
lancer quoi que ce soit, ce que fait le héros dans deux cas : (a) conduit en
diagonale contre un mur plat (disons, en haut à gauche contre un mur qui ne
bloque que x) — s'arrête-t-il ou glisse-t-il, et le long de quel axe ? ; (b)
conduit en diagonale dans un coin où les deux axes sont solides — s'arrête-t-il
complètement, ou un axe l'emporte-t-il encore ? Écrivez ensuite une petite sonde
(imprimez les `dx`, `dy` demandés et les `x`, `y` après le déplacement) et
conduisez le héros contre un mur puis dans un coin pour vérifier les deux
prédictions. Quel axe l'emporte dans un coin, et pourquoi l'ordre compte-t-il ?

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-084/ex1.md)

### Exercice 2 — Pas de tunneling *(extend-the-code)*

Un héros rapide sur une frame lente fait un grand pas — `speed × dt`, qui sur
une longue frame peut faire des dizaines de pixels. `MoveEntity` ne vérifie que
la destination de ce pas, donc un pas assez grand pourrait sauter proprement
par-dessus un mur mince (une tuile de large, comme la bordure de la carte) sans
jamais atterrir sur une tuile solide — le héros passerait à travers
(tunneling). Corrigez-le : résolvez un grand déplacement en petits sous-pas
(disons, quelques pixels au plus chacun), en gardant la règle d'un axe à la fois
dans chacun, pour que l'entité s'arrête à la première tuile solide qu'elle
franchirait plutôt que de la sauter. Gardez le glissement en état de marche.
Testez ensuite la limite : augmentez la vitesse du héros dans sa ligne de table
et conduisez-le contre le mur de bordure mince — il devrait s'arrêter, pas
passer à travers.

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-084/ex2.md)

---

**Partie :** [Partie 5 — le jeu](../../index.md) ·
**Précédente :** [Leçon 083 — la tilemap et la caméra](lesson-083-tilemap-camera.md) ·
**Suivante :** [Leçon 085 — le mouvement du héros](lesson-085-hero-movement.md) ·
**Étiquette de code :** [`lesson-084`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-084)

*Page traduite de la version anglaise `book/lessons/part-5/lesson-084-collision.md`,
révision `3e7f026`.*

<!-- translation-source: book/lessons/part-5/lesson-084-collision.md @ 3e7f026 -->
