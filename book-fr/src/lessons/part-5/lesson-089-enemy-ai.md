# Leçon 089 — l'IA ennemie

{{#include ../../stability-horizon.md}}

## Prose

L'effectif se tient au `x` et au `y` de sa ligne et ne fait rien. Cette leçon
lui donne une intention : **poursuivre, garder la distance, fuir** — les trois
comportements autour desquels les types d'ennemis ont été conçus, chacun une
petite fonction qui écrit la requête de déplacement d'une entité. Les requêtes
passent par le mover comme celles de toute entité ; le travail par entité de
la marche gagne une branche sur le comportement que la ligne porte déjà.
Aucun update par type, aucune machinerie d'entité — la forme du design D6,
exactement.

### Un comportement écrit une requête

L'entrée du joueur écrit la requête de déplacement du héros ; un comportement
écrit celle d'un ennemi, de la même façon — une direction parmi les huit que
le mouvement connaît, mise à l'échelle de la vitesse propre de l'entité quand
la marche la déplace. Trois fonctions, chacune autour d'un seul nombre : la
distance entre l'entité et le héros.

- **Poursuivre** pointe vers le héros, chaque frame. La distance se réduit.
- **Fuir** pointe à l'opposé, chaque frame. La distance grandit.
- **Garder la distance** pointe vers le héros quand il est trop loin, à
  l'opposé quand il est trop près, et se repose entre les deux — une bande
  autour d'`AI_KEEP` (160 px, le fait du comportement, pas d'un type) pour
  qu'un gardien à sa distance se tienne immobile au lieu de tressaillir de
  part et d'autre de la ligne.

```cpp
double dx = hero.x - e.x, dy = hero.y - e.y;
double d2 = dx * dx + dy * dy;          /* the distance, squared */
if (d2 > far * far)        AiChase(e, hero);
else if (d2 < near * near) AiFlee(e, hero);
else { e.move_x = 0.0; e.move_y = 0.0; }   /* at its distance */
```

La distance est comparée au carré — aucune racine carrée ne tourne à
l'exécution nulle part dans les comportements (l'arithmétique du rapport est
la seule exception, et c'est un rapport). Et la direction est le **point
cardinal** du delta, les mêmes huit directions que la visée du héros : le
monde se déplace en huit voies, et les ennemis y courent aussi.

### Une branche, le travail par entité exprimé une fois

Le travail par entité de la marche branche sur le **comportement de l'entité —
le fait que sa ligne porte** — et rien d'autre :

```cpp
switch (e.behavior) {
case BEHAVIOR_FLY:   CombatFly(map, store, hero, e, dt); continue;
case BEHAVIOR_CHASE: AiChase(e, hero); break;
case BEHAVIOR_KEEP:  AiKeep(e, hero); break;
case BEHAVIOR_FLEE:  AiFlee(e, hero); break;
default: break;  /* `none` stands; `boss` is lesson 090's pattern */
}
```

Chaque comportement retombe ensuite sur le même appel `MoveEntity` que le
héros utilise — l'intention devient mouvement seulement là où la carte le
permet. Le bat, le wisp, le spitter et le golem n'existent pas du tout dans ce
code ; ce sont quatre valeurs d'un seul switch. Le boss, à la leçon suivante,
est une cinquième valeur — composée de ces trois mêmes comportements plus un
planning — pas une cinquième forme.

### Les trois comportements, mesurés

Une exécution avec le héros immobile déplace les trois types à la fois. Le
rapport échantillonne chaque entité pendant qu'elle parcourt une tuile, sa
distance au héros à côté — le nombre autour duquel chaque comportement
tourne :

```
engine: bat at 389,72 — 177 px of the hero (t=1.504)
engine: bat at 368,93 — 149 px of the hero (t=1.692)
engine: bat at 353,112 — 126 px of the hero (t=1.865)
engine: bat at 349,137 — 101 px of the hero (t=2.081)
engine: bat at 329,156 — 77 px of the hero (t=2.253)
engine: bat at 310,176 — 55 px of the hero (t=2.426)
engine: wisp at 640,463 — 401 px of the hero (t=1.504)
engine: wisp at 659,479 — 426 px of the hero (t=1.736)
engine: wisp at 685,479 — 447 px of the hero (t=2.038)
engine: wisp at 710,479 — 469 px of the hero (t=2.340)
engine: wisp at 736,479 — 491 px of the hero (t=2.642)
engine: spitter at 222,297 — 111 px of the hero (t=1.504)
engine: spitter at 203,316 — 137 px of the hero (t=1.779)
```

La **poursuite** se réduit : 177 → 149 → 126 → 101 → 77 → 55, le bat se
rapprochant du héros le long de sa voie diagonale. La **fuite** grandit :
401 → 426 → 447 → 469 → 491 — et regardez le `y` du wisp : épinglé à `479`,
`479`, `479` tandis que son `x` glisse vers la droite. C'est le **mover** au
travail : le wisp a manqué de monde, le mur a refusé le pas, et le mouvement
le long du mur a quand même eu lieu — la même règle de glissement que le héros
utilise depuis la leçon 084. Chaque comportement passe par `MoveEntity` ; la
carte a le dernier mot.

Le troisième, **garder la distance**, est le plus discret. Son premier rapport
le montre déjà en train de corriger : le spitter se tenait à 111 px — trop
près pour son 160 — et a reculé. Là où il s'est installé, d'après le bilan de
clôture de l'exécution :

```
engine: world: slime ends at 400,320 — 124 px of the hero
engine: world: bat ends at 316,176 — 55 px of the hero
engine: world: wisp ends at 736,480 — 491 px of the hero
engine: world: spitter ends at 191,328 — 154 px of the hero
engine: world: golem ends at 384,96 — 153 px of the hero
```

`154 px` — à l'intérieur de la bande que le comportement tient (152 à 168), et
il s'y maintient. Le golem est à `153 px` par pure coïncidence de l'endroit où
se tient sa ligne : son comportement est `boss`, et le schéma du boss est
celui de la leçon suivante. Le slime est à la position de sa ligne parce que
sa ligne dit `none` et que la marche de substitution de la leçon 084 a disparu
maintenant que les vrais comportements existent.

### Ce que cette exécution a vérifié, et ce qu'elle n'a pas vérifié

- **Chaque comportement déplace son entité à travers le mover** — la distance
  de la poursuite qui se réduit (177 → 55), celle de la fuite qui grandit
  (401 → 491) et se tient au mur à cause du mover (son `y` fixe, son `x` qui
  glisse), le gardien se stabilisant à 154 px de son 160 et s'y maintenant.
- **Le travail par entité est exprimé une fois** — l'unique switch de la
  marche sur le comportement de la ligne ; les types apparaissent dans les
  rapports de l'exécution et nulle part dans le code.

Ce que cette exécution n'a **pas** vérifié, c'est quoi que ce soit qui
*attaque* : les comportements écrivent du mouvement et rien d'autre — les
armes des lignes d'ennemis (`damage`, `rate`, `fires`) sont encore seulement
portées. Le boss non plus ne fait rien d'autre que se tenir là. La leçon 090
assemble le boss à partir de ces trois comportements plus un planning à lui,
et c'est là que le monde commence à tirer en retour.

## Étape de code

Un changement : les comportements. `src/ai.h` et `src/ai.cpp` sont nouveaux —
trois petites fonctions qui écrivent la requête de déplacement d'une entité
(`AiChase`, `AiKeep`, `AiFlee`) et la distance de garde que le comportement
tient. La marche de `src/game.cpp` gagne l'unique branche sur le comportement
de la ligne (le `fly` du projectile y était déjà) ; `src/main.cpp` abandonne
la marche de substitution de la leçon 084 (les comportements sont réels
maintenant) et rapporte le mouvement du monde au fil qu'il se produit — chaque
entité pendant qu'elle parcourt une tuile, sa distance au héros à côté — et où
elle a fini. Son état final est étiqueté `lesson-089`.

```diff
diff --git a/src/ai.cpp b/src/ai.cpp
new file mode 100644
index 0000000..1ed2dc4
--- /dev/null
+++ b/src/ai.cpp
@@ -0,0 +1,48 @@
+// ai.cpp — the enemy behaviors, one request at a time.
+//
+// Lesson 089: a behavior is a small function of the entity, the hero,
+// and what the entity's row already carried — nothing else. Each writes
+// the entity's movement request; the walk turns every request into
+// motion through the mover, exactly as it does for the hero. No
+// behavior knows about kinds, rows, or the store.
+
+#include "ai.h"
+
+#include "combat.h" /* CombatAim — the same eight compass points */
+
+namespace engine {
+
+void AiChase(Entity &e, const Entity &hero)
+{
+    /* The request is the compass point at the hero — the same eight
+       directions the player's arrows write for the hero, so the chaser
+       covers ground at its own speed whichever way it runs. */
+    CombatAim(hero.x - e.x, hero.y - e.y, e.move_x, e.move_y);
+}
+
+void AiKeep(Entity &e, const Entity &hero)
+{
+    /* The distance is squared throughout — no square root runs at run
+       time. Beyond the band the keeper closes; inside it, it backs
+       away; within it, it stands at its distance. */
+    double dx = hero.x - e.x, dy = hero.y - e.y;
+    double d2 = dx * dx + dy * dy;
+    double near = AI_KEEP - AI_KEEP_BAND;
+    double far = AI_KEEP + AI_KEEP_BAND;
+    if (d2 > far * far)
+        AiChase(e, hero);
+    else if (d2 < near * near)
+        AiFlee(e, hero);
+    else {
+        e.move_x = 0.0;
+        e.move_y = 0.0;
+    }
+}
+
+void AiFlee(Entity &e, const Entity &hero)
+{
+    /* Away — the compass point of the reverse delta. */
+    CombatAim(e.x - hero.x, e.y - hero.y, e.move_x, e.move_y);
+}
+
+} /* namespace engine */
diff --git a/src/ai.h b/src/ai.h
new file mode 100644
index 0000000..a2b1ee7
--- /dev/null
+++ b/src/ai.h
@@ -0,0 +1,42 @@
+// ai.h — the enemy behaviors: chase, keep-distance, flee.
+//
+// Lesson 089: three small functions, each writing one entity's movement
+// request the way the player's input writes the hero's — a direction of
+// the eight the movement knows — and every behavior moves through the
+// mover (MoveEntity) like every entity does. The walk branches once on
+// the entity's behavior, the fact its row carries (design D6):
+// per-entity work is expressed once, not per type. The boss (lesson
+// 090) composes these same three with a schedule of its own — its own
+// pattern, never its own movement machinery.
+//
+// This is the game layer's AI file pair, beside the services (D2).
+#ifndef AI_H
+#define AI_H
+
+#include "entity.h"
+
+namespace engine {
+
+/* Lesson 089: the distance the keep-distance behavior keeps, in world
+   pixels — the behavior's fact, not any kind's: every kind that keeps,
+   keeps this far. */
+constexpr double AI_KEEP = 160.0;
+
+/* The band inside which "close enough" holds — a keeper at its
+   distance stands instead of twitching across the line. */
+constexpr double AI_KEEP_BAND = 8.0;
+
+/* Chase: the request points at the hero, every frame. */
+void AiChase(Entity &e, const Entity &hero);
+
+/* Keep-distance: the request points at the hero when it is too far,
+   away when it is too close, and is rest at the distance — the ranged
+   kind's habit: near enough to shoot, far enough to live. */
+void AiKeep(Entity &e, const Entity &hero);
+
+/* Flee: the request points away from the hero, every frame. */
+void AiFlee(Entity &e, const Entity &hero);
+
+} /* namespace engine */
+
+#endif
diff --git a/src/game.cpp b/src/game.cpp
index 33101e3..11276a1 100644
--- a/src/game.cpp
+++ b/src/game.cpp
@@ -12,6 +12,7 @@
 #include <cstdio>
 
 #include "blit.h"
+#include "ai.h"
 #include "combat.h"
 #include "text.h"
 #include "tilemap.h"
@@ -239,17 +240,38 @@ int GameWalk(EntityStore &store, const TileMap &map, const Entity &hero,
        Lesson 087: the per-entity work branches on the entity's behavior
        — the fact its row carries. A projectile flies: its own
        sub-stepped flight through the same mover, retiring at walls, at
-       its range's end, and at the entity it hit. Every other behavior
-       leaves the request for the mover below. */
+       its range's end, and at the entity it hit.
+
+       Lesson 089: the rest of the branch is the enemy behaviors —
+       chase, keep-distance, flee — each a small function writing this
+       entity's movement request the way the player's input writes the
+       hero's. One branch on the behavior, per-entity work expressed
+       once: the boss (lesson 090) is one more value here, not one more
+       shape. */
     int visited = 0;
     for (int i = 0; i < ENTITY_CAP; ++i) {
         if (!store.slots[i].live)
             continue;
         visited += 1;
         Entity &e = store.slots[i];
-        if (e.behavior == BEHAVIOR_FLY) {
+        switch (e.behavior) {
+        case BEHAVIOR_FLY:
             CombatFly(map, store, hero, e, dt);
-            continue;
+            continue; /* the flight moves itself, through the mover */
+        case BEHAVIOR_CHASE:
+            AiChase(e, hero);
+            break;
+        case BEHAVIOR_KEEP:
+            AiKeep(e, hero);
+            break;
+        case BEHAVIOR_FLEE:
+            AiFlee(e, hero);
+            break;
+        default:
+            /* `none` stands where it stands — the request is its row's
+               (lesson 084's stand-in walk writes one) — and `boss` is
+               lesson 090's pattern, composed of these same behaviors. */
+            break;
         }
         MoveEntity(map, e, e.move_x * e.speed * dt, e.move_y * e.speed * dt);
         if (e.move_x > 0.0)
diff --git a/src/main.cpp b/src/main.cpp
index 6a3320a..4c5a16f 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -9,6 +9,7 @@
 // is invented here; today the parts fit, and the fit is what the demo
 // shows. The language law of lesson 026 still holds over all of it.
 
+#include <cmath>
 #include <cstdio>
 
 #include "arena.h"
@@ -347,12 +348,10 @@ int Run(void)
             ArenaRelease(arena);
             return 1;
         }
-        /* Lesson 084: a non-hero entity walks (down-right) so its
-           movement is resolved against the map like the hero's — a stand
-           in for the AI lesson 089 brings. The walk slides it along
-           walls and stops it at solid tiles. */
-        made.entity->move_x = 1.0;
-        made.entity->move_y = 1.0;
+        /* Lesson 084: a non-hero entity walked (down-right) here — a
+           stand-in for the AI. Lesson 089 replaced it: the behaviors
+           are real now, and the world's kinds move the ways their rows
+           say (the slime's row says `none`, so it stands). */
         if (!foe)
             foe = made.entity;
         created += 1;
@@ -441,6 +440,8 @@ int Run(void)
     bool was_blocked = false; /* lesson 077: the mover's state report */
     int was_vx = 0, was_vy = 0; /* lesson 085: the hero's velocity, as it eases */
     int was_frame = 0;          /* lesson 086: the hero's walk-cycle frame */
+    double seen_x[ENTITY_CAP] = {}, seen_y[ENTITY_CAP] = {}; /* lesson 089:
+                                  where each entity was last reported */
 
     /* The slice's identity: what the run is, named at once — L0*, the
        gate this part closes on. Every service it uses was finished
@@ -587,6 +588,26 @@ int Run(void)
         frame.entities = platform::Now() - t_entities;
         walk_visits += visited;
 
+        /* Lesson 089: the world's motion, as the behaviors produce it —
+           every non-hero entity reported as it travels about a tile, its
+           distance to the hero beside it (the number all three behaviors
+           are about: chase shrinks it, flee grows it, keep holds it). */
+        for (int i = 0; i < ENTITY_CAP; ++i) {
+            Entity &e = store.slots[i];
+            if (!e.live || &e == &hero)
+                continue;
+            double dx = e.x - seen_x[i], dy = e.y - seen_y[i];
+            if (dx * dx + dy * dy < 24.0 * 24.0)
+                continue;
+            seen_x[i] = e.x;
+            seen_y[i] = e.y;
+            double to_x = hero.x - e.x, to_y = hero.y - e.y;
+            std::printf("engine: %s at %d,%d — %d px of the hero (t=%.3f)\n",
+                        e.name, (int)e.x, (int)e.y,
+                        (int)std::sqrt(to_x * to_x + to_y * to_y),
+                        platform::Now() - started);
+        }
+
         /* The score, and the hero's own report: where the entity the
            game moves has got to. */
         distance += (hero.x > was_x ? hero.x - was_x : was_x - hero.x) +
@@ -790,6 +811,21 @@ int Run(void)
     std::printf("engine: demo: %ld frames measured, %d buffers fed, %d effects fired, %d music wraps\n",
                 frame_number, feeds, effect_count, music_wraps);
 
+    /* Lesson 089: where the behaviors left the world — every live
+       entity's position and its distance to the hero, the number all
+       three behaviors are about (chase shrinks it, flee grows it, keep
+       holds it). The report above samples a moving world; this one
+       states where it ended. */
+    for (int i = 0; i < ENTITY_CAP; ++i) {
+        Entity &e = store.slots[i];
+        if (!e.live || &e == &hero)
+            continue;
+        double to_x = hero.x - e.x, to_y = hero.y - e.y;
+        std::printf("engine: world: %s ends at %d,%d — %d px of the hero\n",
+                    e.name, (int)e.x, (int)e.y,
+                    (int)std::sqrt(to_x * to_x + to_y * to_y));
+    }
+
     /* Lesson 075: the walk's account — one visit per live entity per
        frame, and nothing else. */
     std::printf("engine: walk: %ld visits over %ld frames — one per live entity per frame\n",
```

## Exercices

Deux défis plus grands. Chacun se termine par sa solution — un diff contre
l'état final de cette leçon plus une visite guidée — après l'énoncé.

### Exercice 1 — La distance du gardien est de la donnée *(extend-the-code)*

`AI_KEEP` est un seul nombre pour tous les gardiens — mais « les attributs par
type sont de la donnée » a un cousin : le réglage d'un comportement devrait
vivre là où vit le réglage. Faites de la distance de garde **un fait de
ligne** : faites grandir le format comme la leçon 087 l'a grandi (nommé,
additif, avec défaut — les fichiers qui nomment la colonne la déclarent, les
fichiers qui ne la nomment pas gardent le défaut), portez-la comme toute autre
valeur, et faites utiliser à `AiKeep` le nombre propre de l'entité. Puis
lancez deux gardiens à deux distances dans un même monde et citez les lignes
du bilan de clôture. Qu'est-ce que le défaut achète à un fichier qui reste
silencieux ?

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-089/ex1.md)

### Exercice 2 — L'escalier du poursuivant *(predict-the-output)*

Les comportements écrivent des **points cardinaux** — les huit directions —
jamais une direction entre elles. Donc la trajectoire d'un poursuivant est un
escalier. Prédisez la trajectoire du bat précisément : partant du `(560, 72)`
de sa ligne avec le héros immobile à `(312, 232)`, quelle suite de
déplacements `AiChase` demande-t-il, quand la trajectoire cesse-t-elle d'être
diagonale, et à quoi ressemble le dernier segment ? Dites ensuite ce qui
arrive quand un mur se dresse en travers de la diagonale : quel axe le mover
résout-il en premier, et à quoi ressemble l'escalier pendant que le bat
glisse ? Lancez-le et comparez à la trajectoire citée par la leçon.

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-089/ex2.md)

---

**Partie :** [Partie 5 — le jeu](../../index.md) ·
**Précédente :** [Leçon 088 — les tables d'archétypes des ennemis](lesson-088-enemy-tables.md) ·
**Suivante :** [Leçon 090 — le boss](lesson-090-boss.md) ·
**Étiquette de code :** [`lesson-089`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-089)

*Page traduite de la version anglaise `book/lessons/part-5/lesson-089-enemy-ai.md`,
révision `d371511`.*

<!-- translation-source: book/lessons/part-5/lesson-089-enemy-ai.md @ d371511 -->
