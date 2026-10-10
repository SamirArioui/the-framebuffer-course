# Leçon 092 — hitstop et screenshake

{{#include ../../stability-horizon.md}}

## Prose

La leçon 086 a construit deux hooks et les a laissés en attente : un hitstop qui
fait chuter l'échelle du temps de jeu à une fraction et la ramène tout seul à
son échéance en temps mural, et un screenshake qui déplace le décalage additif
de la caméra et le repose exactement à zéro. Elle a déclenché les deux une fois
depuis un script en temps mural, pour que leur vie de déclenchement-et-repos
soit visible — et a dit clairement que le script était un substitut. Cette leçon
est ce pour quoi les hooks ont été construits : **les événements du jeu lui-même
les déclenchent**, et une règle organise tout ce qui suit.

> **La rétroaction commence avec l'événement.** Un effet de ressenti commence
> dans la frame où son événement déclencheur se produit.

Pas sur une horloge, pas un temps plus tard, pas « quand ça arrange » : le coup
et son poids sont un seul moment, parce que le joueur doit lire la cause et
l'effet comme un seul moment. Le reste de cette leçon est cette règle rendue
concrète — les événements, les poids, et l'ordre des frames qui garde l'effet à
l'intérieur de la frame de l'événement.

### Les événements : un coup atterrit, une mort tombe

La boîte à outils répond à deux événements dans cette leçon, tous deux se
produisant déjà dans le vol du combat (`CombatFly`) : **un coup atterrit** (la
branche du vol où le coup atterrit — les points de vie de la cible y chutent des
dégâts du tir) et **une mort tombe** (la même branche, où une cible atteint zéro
point de vie). Le coup est un hitstop court et une petite secousse — un quart du
temps de jeu pendant 0.15 s, une secousse de 5 px pendant 0.25 s. La mort, ce
sont les deux mêmes hooks, plus lourds — un quart pendant 0.30 s, une secousse
de 10 px pendant 0.50 s. Un coup fatal répond comme les deux événements à la
fois, et les poids de la mort l'emportent, parce que déclencher de nouveau,
c'est simplement la boîte à outils qui dit *celui-ci est plus lourd*.

Voici la règle visible dans une exécution. Le vrai effectif, le héros immobile
pendant que les deux bats de la vague 1 se rapprochent et tirent — deux bolts
atterrissent dans une seule frame, et la boîte à outils se déclenche à côté de
chaque ligne de coup :

```
engine: fire: bat -> bolt (damage 1, range 160)
engine: hit: bolt hits hero — damage 1, health 3 -> 2
engine: feel: hitstop fired (0.25x, 0.15s)
engine: feel: shake fired (5 px, 0.25s)
engine: shot bolt retired — hit hero
engine: hit: bolt hits hero — damage 1, health 2 -> 1
engine: feel: hitstop fired (0.25x, 0.15s)
engine: feel: shake fired (5 px, 0.25s)
engine: shot bolt retired — hit hero
frame 102: step 44.293 ms, …
```

Chaque ligne `fired` se trouve entre le compte rendu de la frame 101 et celui de
la frame 102 — les événements et leurs effets sont à l'intérieur du compte rendu
d'**une seule frame**, celle de la frame 102. C'est la règle, mesurable : aucune
ligne `frame` ne sépare un coup de la rétroaction qu'il a causée.

Et trois secondes plus tard, le troisième bolt porte le coup fatal au héros — un
coup et une mort dans une seule frame :

```
engine: hit: bolt hits hero — damage 1, health 1 -> 0
engine: feel: hitstop fired (0.25x, 0.15s)
engine: feel: shake fired (5 px, 0.25s)
engine: shot bolt retired — hit hero
engine: feel: hitstop fired (0.25x, 0.30s)
engine: feel: shake fired (10 px, 0.50s)
engine: hit: bolt hits hero — damage 1, health 0 -> 0
…
engine: state play -> death (the hero's health reached zero)
```

(Bord honnête, mesuré et non caché : le deuxième trait de la rafale porte sur un
héros déjà à zéro et répond de nouveau avec de la rétroaction. La règle des
coups demande « est-il vivant », l'acteur du jeu n'est jamais retiré — le
cadavre encaisse donc un coup de plus avant que la machine à états ne lise la
condition de défaite à la frame suivante. Une frame de double rétroaction ;
l'événement reçoit tout de même sa réponse dans sa propre frame.)

Les mêmes événements se déclenchent quand c'est le héros qui tue — un effectif
jetable d'un seul type fragile et immobile aux pieds du héros, le bolt à bout
portant du blaster (les nombres sont ceux de l'effectif jetable) :

```
engine: fire: hero -> bolt (damage 1, range 160)
engine: hit: bolt hits bag — damage 1, health 1 -> 0
engine: feel: hitstop fired (0.25x, 0.15s)
engine: feel: shake fired (5 px, 0.25s)
engine: shot bolt retired — hit bag
engine: bag retired — zero health
engine: feel: hitstop fired (0.25x, 0.30s)
engine: feel: shake fired (10 px, 0.50s)
```

Une frame, un coup fatal, quatre déclenchements — les poids du coup, puis ceux
de la mort par-dessus.

### L'ordre des frames que la règle exige

Déclencher dans la frame de l'événement ne suffit pas en soi ; la frame doit
aussi *montrer* l'effet. Le passage par frame de la boîte à outils (`FeelUpdate`,
la décroissance-et-pilotage de la leçon 086) a donc changé de place : il tournait
avant l'avancée du monde, et il tourne maintenant **après les événements de la
frame et avant que la frame ne soit dessinée** — après la marche qui l'a
déclenché, à côté du suivi de caméra, avant le render. Lancez-le avant la marche
et la secousse du coup atteindrait la caméra une frame trop tard — l'image serait
en retard sur l'événement, exactement ce que la règle interdit. Lancez-le après
le dessin et l'effet manquerait entièrement l'image de l'événement.

Une limite honnête, visible dans le même extrait : le `step` de la frame 102 est
`44.293 ms` — pleine vitesse. Le step est l'*avancée* de la frame, déjà prise
avant la marche qui trouve le coup ; un pas déjà dépensé ne peut pas être réduit
rétroactivement. Ce que la frame de l'événement porte bel et bien, c'est le
déclenchement lui-même (ci-dessus), la secousse dans son image (le décalage est
piloté avant le dessin), et le facteur du hitstop déjà abaissé — le pas suivant
est celui qui est ralenti.

### Le hitstop se termine tout seul

L'échéance du hitstop est en temps mural — la règle de la leçon 078 : tout ce qui
doit *se terminer* pendant que le jeu est arrêté ne peut pas tourner au temps de
jeu, et tout l'intérêt d'un hitstop est de se terminer. L'échelle chute au quart
et revient sans que le jeu n'intervienne, et le journal de frames le mesure :

```
frame 103: step 10.779 ms, …
frame 104: step 11.001 ms, …
engine: feel: hitstop rested — full speed again
frame 105: step 10.808 ms, …
frame 106: step 43.493 ms, …
```

Trois frames à `10.8 ms` contre les `43 ms` cadencés de l'exécution — le quart
auquel le hitstop s'est déclenché — puis `43.493 ms` de nouveau : pleine vitesse,
sans l'aide de personne. (La frame 105 est encore lente bien que le repos soit
imprimé avant elle : son step a été calculé à son début, et le repos retombe
dans son compte rendu.) Mesuré contre l'horloge murale de l'exécution, le
déclenchement à `t=32.265` et le repos à `t=32.389` sont à 124 ms d'écart pour
les 0.15 s demandées — l'échéance honorée à la granularité de la frame, à une
frame près, sur les frames d'environ 44 ms de cette exécution ; à 60 fps, la
même granularité fait moins de 17 ms.

### La secousse se repose exactement à zéro

La secousse pilote le décalage additif de la caméra — le hook que la leçon 054 a
laissé dans la caméra — et la règle qui compte est là où elle *finit* :
exactement zéro, pour que la vue du monde ne reste jamais décalée d'un pixel. La
ligne de repos le dit dans chaque exécution : `engine: feel: shake rested at
0,0`. Et entre le déclenchement et le repos, le décalage bouge réellement — une
sonde jetable qui imprime le décalage qu'utilise le dessin (dite comme telle :
une sonde, pas l'exécution livrée) le montre alternant tant que la secousse
dure :

```
engine: probe: camera add 10,0
engine: probe: camera add 10,0
engine: probe: camera add -10,0
engine: probe: camera add -10,0
…
engine: feel: shake rested at 0,0
```

`10` est la magnitude propre de la mort — la secousse plus lourde du coup fatal,
en pixels, sur le décalage de la caméra — et le retour au repos est le `0,0` du
hook lui-même, exact.

### Ce que cette exécution a vérifié, et ce qu'elle n'a pas vérifié

- **La rétroaction commence avec l'événement** — chaque ligne `fired` se trouve
  à l'intérieur du compte rendu de la frame de l'événement, sans ligne `frame`
  entre un coup et sa rétroaction ; le décalage de la secousse est piloté avant
  que la frame ne se dessine.
- **Le hitstop ralentit la simulation à une fraction et revient à pleine vitesse
  tout seul à son échéance en temps mural** — `step 10.8 ms` contre les `43 ms`
  cadencés pendant trois frames, puis `43.5 ms` de nouveau, la ligne de repos
  entre les deux ; l'échéance honorée à une frame près de ses 0.15 s (124 ms de
  temps mural mesurées sur des frames d'environ 44 ms).
- **Le décalage de la secousse se repose exactement à zéro** — `shake rested at
  0,0`, la sonde montrant le décalage bouger de ±10 px tant que l'effet dure.

Ce que cette exécution n'a **pas** vérifié, c'est si les poids *se ressentent*
bien — un arrêt d'un quart de seconde et une secousse de 5 px sont des nombres ;
savoir si un coup atterrit avec le bon poids est un jugement que cette exécution
sans écran ne peut pas rendre. Ce jugement est celui du joueur, et c'est la
seule chose du game feel que la mesure ne peut pas trancher. Ce que l'exécution
ne peut pas montrer non plus, c'est l'image secouée elle-même (il n'y a pas
d'écran à regarder ici) — elle vérifie le décalage auquel l'image est dessinée,
ce qui est la même chose par construction.

## Étape de code

Un changement : la boîte à outils se déclenche depuis les événements. Le vol de
`src/combat.cpp` déclenche `FeelHitstop`/`FeelShake` aux lignes de coup et de
mort — la frame propre de l'événement, les poids propres de l'événement — et
`CombatFly` (avec `GameWalk` qui le transmet) gagne `Feedback &feel` pour
déclencher à travers. `src/main.cpp` perd le script de démonstration en temps
mural et déplace le passage de la boîte à outils (`FeelUpdate`) après les
événements de la frame et avant son dessin ; les hooks de `src/feel.cpp`
rapportent leur propre déclenchement à côté des lignes de l'événement,
l'impression de la démonstration mourant avec la démonstration. Les hooks
eux-mêmes — les mécanismes de déclenchement-et-repos de la leçon 086 — sont
intacts. Son état final est étiqueté `lesson-092`.

```diff
diff --git a/src/combat.cpp b/src/combat.cpp
index f894460..a10ffb7 100644
--- a/src/combat.cpp
+++ b/src/combat.cpp
@@ -5,6 +5,9 @@
 // row names; the flight is the projectile's speed over game time; the
 // life is its row's range. Nothing here knows which weapon or which
 // projectile exists — the tables know that.
+//
+// Lesson 092: the hit and the death are the juice toolkit's events, and
+// the feedback hooks fire at the lines where they happen.
 
 #include "combat.h"
 
@@ -113,7 +116,7 @@ bool CombatFire(EntityStore &store, const EntityTable &shots,
 }
 
 void CombatFly(const TileMap &map, EntityStore &store, const Entity &hero,
-               Entity &shot, double dt)
+               Entity &shot, Feedback &feel, double dt)
 {
     /* The flight, in game time: the shot's speed over dt, sub-stepped
        through the mover so each sub-step is small. What is checked at
@@ -149,6 +152,15 @@ void CombatFly(const TileMap &map, EntityStore &store, const Entity &hero,
             std::printf("engine: hit: %s hits %s — damage %d, health %d -> %d\n",
                         shot.name, target->name, shot.damage, was,
                         target->health);
+
+            /* Lesson 092: feedback starts with the event. The hit is one
+               of the toolkit's triggers, and the toolkit answers here —
+               in the hit's own frame, on this very line of the flight —
+               a short hitstop and a small shake. Not on a clock, not a
+               frame later: the hit and its weight are one moment. */
+            FeelHitstop(feel, 0.25, 0.15);
+            FeelShake(feel, 5.0, 0.25);
+
             std::printf("engine: shot %s retired — hit %s\n", shot.name,
                         target->name);
             EntityRetire(store, shot);
@@ -161,6 +173,14 @@ void CombatFly(const TileMap &map, EntityStore &store, const Entity &hero,
                             target->name);
                 EntityRetire(store, *target);
             }
+            if (target->health == 0) {
+                /* Lesson 092: a death is the toolkit's heavier event —
+                   the same two hooks, weighted for it, fired in the same
+                   frame as the hit that made it. A killing blow answers
+                   as both, and the death's weights win. */
+                FeelHitstop(feel, 0.25, 0.30);
+                FeelShake(feel, 10.0, 0.50);
+            }
             return;
         }
 
diff --git a/src/combat.h b/src/combat.h
index 77df2ba..fddb88a 100644
--- a/src/combat.h
+++ b/src/combat.h
@@ -16,6 +16,7 @@
 #define COMBAT_H
 
 #include "entity.h"
+#include "feel.h"
 #include "table.h"
 
 namespace engine {
@@ -56,9 +57,11 @@ bool CombatFire(EntityStore &store, const EntityTable &shots,
    health by the shot's damage and retires the shot; a zero-health target
    is retired too — the hero excepted, whose zero health is the game's
    defeat condition (the state machine reads it, the game's actor is not
-   retired out from under the game). */
+   retired out from under the game). Lesson 092: the hit and the death
+   are the toolkit's events — the feedback hooks fire here, in the
+   event's own frame, through `feel`. */
 void CombatFly(const TileMap &map, EntityStore &store, const Entity &hero,
-               Entity &shot, double dt);
+               Entity &shot, Feedback &feel, double dt);
 
 /* Lesson 090: the enemy attack, once per frame of game time. An armed
    entity — one whose row names a projectile kind — fires it at the
diff --git a/src/feel.cpp b/src/feel.cpp
index 2651e0c..76eb89a 100644
--- a/src/feel.cpp
+++ b/src/feel.cpp
@@ -1,8 +1,10 @@
 // feel.cpp — the feedback hooks: fire, decay, rest.
 //
 // Lesson 086: each hook fires, runs down its own wall-time, and returns
-// exactly to rest. Nothing here decides *when* to fire — that is the
-// juice toolkit's job (lessons 092-093), reading the game's events.
+// exactly to rest. Lesson 092: the game's own events fire them — a hit
+// lands, a death falls — in the event's own frame, and each hook says
+// when it fires beside the event's own line. The weights are the
+// event's, passed in from where the event happens.
 
 #include "feel.h"
 
@@ -22,12 +24,16 @@ void FeelShake(Feedback &feel, double magnitude, double seconds)
 {
     feel.shake = seconds;
     feel.shake_mag = magnitude;
+    std::printf("engine: feel: shake fired (%.0f px, %.2fs)\n", magnitude,
+                seconds);
 }
 
 void FeelHitstop(Feedback &feel, double fraction, double seconds)
 {
     feel.hitstop = seconds;
     feel.hitstop_k = fraction;
+    std::printf("engine: feel: hitstop fired (%.2fx, %.2fs)\n", fraction,
+                seconds);
 }
 
 double FeelTimeScale(const Feedback &feel)
diff --git a/src/feel.h b/src/feel.h
index d8706e2..5827080 100644
--- a/src/feel.h
+++ b/src/feel.h
@@ -1,10 +1,12 @@
-// feel.h — the feedback hooks the juice toolkit will drive.
+// feel.h — the feedback hooks the juice toolkit drives.
 //
 // Lesson 086: two hooks — a screenshake and a hitstop — each a thing
-// that fires and then rests. These are the hooks the juice toolkit
-// (lessons 092-093) will drive from the game's events: here they are the
-// mechanisms, each with its own fire-and-rest life, and nothing yet says
-// when to fire them. A hook at rest costs nothing and changes nothing.
+// that fires and then rests. Lesson 092: the toolkit fires them from the
+// game's own events — a hit lands, a death falls — in the event's own
+// frame, so the player reads cause and effect as one moment. The hooks
+// are the mechanisms, each with its own fire-and-rest life; the events
+// decide when and how heavily. A hook at rest costs nothing and changes
+// nothing.
 #ifndef FEEL_H
 #define FEEL_H
 
diff --git a/src/game.cpp b/src/game.cpp
index a2f87fe..3da68a4 100644
--- a/src/game.cpp
+++ b/src/game.cpp
@@ -218,7 +218,7 @@ void GameDrawSprites(const Game &game, Framebuffer &fb,
 }
 
 int GameWalk(EntityStore &store, const TileMap &map, const Entity &hero,
-             const EntityTable &shots, double dt)
+             const EntityTable &shots, Feedback &feel, double dt)
 {
     /* Lesson 084: the walk — every live entity, once per frame, in slot
        order, its movement resolved against the tilemap. The per-entity
@@ -237,7 +237,11 @@ int GameWalk(EntityStore &store, const TileMap &map, const Entity &hero,
        entity's movement request the way the player's input writes the
        hero's. One branch on the behavior, per-entity work expressed
        once: the boss (lesson 090) is one more value here, not one more
-       shape. */
+       shape.
+
+       Lesson 092: the flight's events — a hit lands, a death falls —
+       fire the feedback hooks in their own frame (the toolkit is handed
+       along through `feel`). */
     int visited = 0;
     for (int i = 0; i < ENTITY_CAP; ++i) {
         if (!store.slots[i].live)
@@ -246,7 +250,7 @@ int GameWalk(EntityStore &store, const TileMap &map, const Entity &hero,
         Entity &e = store.slots[i];
         switch (e.behavior) {
         case BEHAVIOR_FLY:
-            CombatFly(map, store, hero, e, dt);
+            CombatFly(map, store, hero, e, feel, dt);
             continue; /* the flight moves itself, through the mover */
         case BEHAVIOR_CHASE:
             AiChase(e, hero);
diff --git a/src/game.h b/src/game.h
index e171716..30dcc3e 100644
--- a/src/game.h
+++ b/src/game.h
@@ -22,6 +22,7 @@
 
 #include "camera.h"
 #include "entity.h"
+#include "feel.h"
 #include "font.h"
 #include "framebuffer.h"
 #include "gametime.h"
@@ -111,10 +112,12 @@ void GameDrawSprites(const Game &game, Framebuffer &fb,
    walls, at its range, at what it hits) instead of the request. Lesson
    089-090: the enemy behaviors and the boss's pattern write the request
    the way the player's input writes the hero's, and an armed entity
-   attacks at its row's rate. The hero is handed along for the combat's
-   rules to know the game's actor by. Returns the visit count. */
+   attacks at its row's rate. Lesson 092: the combat's events (a hit, a
+   death) fire the feedback hooks through `feel`, in their own frame.
+   The hero is handed along for the combat's rules to know the game's
+   actor by. Returns the visit count. */
 int GameWalk(EntityStore &store, const TileMap &map, const Entity &hero,
-             const EntityTable &shots, double dt);
+             const EntityTable &shots, Feedback &feel, double dt);
 
 /* Lesson 091: the waves, once per frame of play. A fresh fight clears
    the last one from the store; a wave spawns its composition from the
diff --git a/src/main.cpp b/src/main.cpp
index c460681..a115e30 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -309,12 +309,12 @@ int Run(void)
     GameInit(game, hero.health);
 
     /* Lesson 086: the feedback hooks — a screenshake and a hitstop, both
-       at rest. The juice toolkit (lessons 092-093) will fire these from
-       the game's events; here a demonstration fires both once so their
-       fire-and-rest life is visible. */
+       at rest. Lesson 092: the juice toolkit fires them from the game's
+       own events now — a hit lands, a death falls — in the event's own
+       frame (the walk's flight, in game.cpp/combat.cpp); the wall-time
+       demonstration that used to fire them here is gone. */
     Feedback feel;
     FeelInit(feel);
-    bool feel_demo = false;
 
     /* Lesson 080: the vertical slice — the game's shape, and nothing
        else. The hero is the row the game asks for by name (it is the
@@ -492,20 +492,6 @@ int Run(void)
            arrows) and left at rest in every other state. */
         GameInput(game, opened.window, hero, wall_dt);
 
-        /* Lesson 086: the feedback hooks run on their own wall-time —
-           each fires, decays, and rests. The demonstration fires both
-           once, in play, so their fire-and-rest life is visible; the
-           juice toolkit (lessons 092-093) will fire them from the game's
-           events instead of this script. */
-        if (!feel_demo && game.state == GAME_PLAY &&
-            platform::Now() - started >= 3.0) {
-            feel_demo = true;
-            FeelShake(feel, 6.0, 0.5);
-            FeelHitstop(feel, 0.25, 0.4);
-            std::printf("engine: feel: shake fired (6 px, 0.5s), hitstop fired (0.25x, 0.4s)\n");
-        }
-        FeelUpdate(feel, wall_dt, game.camera);
-
         /* The game-time scale is the state's (play runs, the rest hold)
            times the hitstop's factor (a fraction during a hitstop, full
            at rest) — one knob, two drivers, multiplied. */
@@ -547,7 +533,7 @@ int Run(void)
             GameWaves(game, store, foes);
 
         double t_entities = platform::Now();
-        int visited = GameWalk(store, map, hero, shots, dt);
+        int visited = GameWalk(store, map, hero, shots, feel, dt);
         frame.entities = platform::Now() - t_entities;
         walk_visits += visited;
 
@@ -618,6 +604,15 @@ int Run(void)
            in game.cpp); the loop keeps no camera of its own. */
         GameFollow(game, hero, map);
 
+        /* Lesson 086: the feedback hooks run on their own wall-time —
+           each fires, decays, and rests. Lesson 092 moved the run to
+           here, after the frame's events and before the frame is drawn:
+           the toolkit settles whatever the walk fired this frame, so a
+           hit's shake is already in the hit's own frame's picture. Each
+           hook returns to rest on its own — the hitstop to full speed,
+           the shake's additive offset to exactly zero. */
+        FeelUpdate(feel, wall_dt, game.camera);
+
         frame.update = platform::Now() - t0;
 
         /* Lesson 060: the audio step — the loop feeds the device the next
```

## Exercices

Deux défis plus conséquents. Chacun se termine par sa solution — un diff contre
l'état final de cette leçon, plus une visite guidée — après l'énoncé.

### Exercice 1 — La secousse qui s'amortit *(extend-the-code)*

Le décalage du screenshake oscille à pleine magnitude et se coupe à zéro à
l'instant où sa vie se termine — et la coupure se voit : le monde revient d'un
coup au lieu de s'immobiliser. Faites **s'amortir** la secousse — sa magnitude
décroît sur toute la vie de la secousse, pour que la caméra revienne en douceur
au repos, et le décalage se repose toujours à **exactement zéro** à son échéance
(cette règle est celle qui doit tenir ; la secousse reste un seul effet,
screenshake, pas un nouvel effet). L'état `Feedback` peut grandir de ce dont il
a besoin pour diviser. Montrez ensuite l'amortissement dans une exécution —
l'exécution doit rapporter le décalage tant que la secousse se déclenche (les
hooks rapportent déjà leur déclenchement et leur repos ; l'entre-deux est à vous
d'imprimer) — et citez la descente jusqu'au repos.

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-092/ex1.md)

### Exercice 2 — La pause qui rencontre le hitstop *(predict-the-output)*

L'échelle du temps de jeu est l'échelle de l'état multipliée par le facteur du
hitstop — un bouton, deux pilotes, multipliés. Avant de lancer quoi que ce soit,
prédisez ce qui se passe quand le joueur met en pause **pendant qu'un hitstop
tourne encore** : ce que montre la colonne `step` du journal de frames avant,
pendant et après la pause ; où tombe la ligne `hitstop rested` par rapport à la
pause ; et si le jeu reprend à pleine vitesse ou avec le hitstop encore suspendu
au-dessus de lui. Écrivez la prédiction. Puis lancez-le — mettez en pause juste
après qu'un coup a atterri, avec une sonde à côté de l'exécution qui imprime les
deux facteurs et le temps mural restant du hitstop à chaque frame de la fenêtre —
et comparez à votre prédiction, en expliquant le produit que votre sonde a
mesuré.

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-092/ex2.md)

---

**Partie :** [Partie 5 — le jeu](../../index.md) ·
**Précédente :** [Leçon 091 — vagues](lesson-091-waves.md) ·
**Suivante :** [Leçon 093 — rafales de particules et easing](lesson-093-bursts-easing.md) ·
**Étiquette de code :** [`lesson-092`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-092)

*Page traduite de la version anglaise `book/lessons/part-5/lesson-092-hitstop-shake.md`,
révision `6f25fd1`.*

<!-- translation-source: book/lessons/part-5/lesson-092-hitstop-shake.md @ 6f25fd1 -->
