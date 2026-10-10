# Leçon 085 — le mouvement du héros

{{#include ../../stability-horizon.md}}

## Prose

Le héros glisse le long des murs et s'arrête aux tuiles désormais, mais il se
déplace encore comme un curseur — il est à pleine vitesse à l'instant où vous
appuyez sur une touche, et au repos à l'instant où vous relâchez. Cette leçon
lui donne du poids : **le héros est une vitesse qui converge en douceur vers là
où vous voulez aller, et la diagonale n'est pas plus rapide que la course
droite.**

### L'intention est une direction, pas une vitesse

Les touches maintenues donnent une direction — l'une des huit. Mais prises
brutes, une diagonale (les deux flèches) vaut `(1, 1)`, et faire tourner la
marche là-dessus déplace le héros de `speed` sur *chaque* axe — `speed × √2` au
sol, soit environ 41 % plus rapide qu'un déplacement droit. L'intention est donc
d'abord **normalisée** :

```cpp
if (intent_x != 0.0 && intent_y != 0.0) {
    intent_x *= HERO_DIAG;   /* 1 / sqrt(2) */
    intent_y *= HERO_DIAG;
}
```

Une diagonale devient `(1/√2, 1/√2)`, si bien que le héros couvre le terrain à
la vitesse en ligne droite, quelle que soit la direction où il court. (Une
normalisation générale diviserait par la longueur ; pour huit directions, la
longueur ne vaut jamais que 1 ou √2, donc une seule multiplication suffit.)

### La vitesse converge en douceur — c'est cela, le poids

Le héros porte une vitesse (sa requête de déplacement, que la marche transforme
en mouvement) et la fait converger en douceur vers l'intention :

```cpp
double k = dt / HERO_TIME;         /* the ease, per frame */
if (k > 1.0) k = 1.0;
hero.move_x += (intent_x - hero.move_x) * k;
hero.move_y += (intent_y - hero.move_y) * k;
```

Quand vous braquez, la vitesse se rapproche de l'intention — **accélération**.
Quand vous relâchez, l'intention est nulle, donc la vitesse se rapproche du
repos — **décélération**. Quand vous inversez la direction, la vitesse
*traverse* l'easing d'une direction à l'autre au lieu de sauter d'un coup — le
virage se ressent. `HERO_TIME` est le ressenti : le temps que le héros met à
atteindre (ou à quitter) la pleine vitesse. Et l'easing est mis à l'échelle par
`dt`, donc il est le même à n'importe quelle fréquence de frames — une courbe
régulière vers l'intention, pas un saut fixe par frame.

D'une vraie exécution pilotée par une entrée scriptée, la vitesse du héros
telle qu'elle est pilotée — une direction maintenue depuis le repos, puis
relâchée, puis pilotée en diagonale :

```
engine: hero velocity 240,0 (t=3.348)     <- full speed straight (from rest)
engine: hero velocity 0,0 (t=5.864)       <- released, eased to rest
engine: hero velocity 240,0 (t=3.348)     <- and up to speed again
engine: hero velocity 231,20 (t=3.362)    <- mid-ease, turning to the diagonal
engine: hero velocity 169,169 (t=5.377)   <- the diagonal, settled
```

Deux faits sont dans ces nombres. Le héros **part du repos et y revient en
douceur** — la vitesse quitte `0` et revient à `0` au lieu de sauter directement
à `240` (la ligne `231,20` est un seul pas de l'easing, saisi en plein virage).
Et **la diagonale couvre le terrain à la vitesse en ligne droite** : `169,169` a
une norme de `√(169² + 169²) ≈ 239` — la même vitesse au sol que `240,0`, et non
`√2` fois celle-ci. C'est la normalisation, mesurée.

### Ce que cette exécution a vérifié, et ce qu'elle n'a pas vérifié

- **La diagonale est à la vitesse en ligne droite** — la vitesse diagonale
  stabilisée `169,169` a une norme de ≈ `240`, égale au `240,0` droit.
- **Le héros accélère depuis le repos et décélère jusqu'au repos** — la vitesse
  quitte `0` et revient à `0`, en douceur (`231,20` est un pas de la courbe),
  jamais un saut à la pleine vitesse.

Ce que cette leçon ne fait **pas**, c'est régler le ressenti. `HERO_TIME` est un
premier nombre, et « combien de poids » est un jugement que le jeu fera plus
tard. Une réserve honnête sur la mesure ci-dessus : la boucle sans écran de la
machine de l'auteur est pilotée par les événements et tourne à environ une frame
par seconde, donc `dt` est grand et l'easing s'achève souvent *dans* une seule
frame — la « montée progressive sur plusieurs frames » est ce que vous voyez
aux cadences interactives (60 fps, où `dt/HERO_TIME` étale la même courbe sur
~7 frames). L'easing est mis à l'échelle par `dt` et indépendant de la fréquence
de frames dans les deux cas ; il est simplement échantillonné plus finement à
60 fps. Sur votre propre bureau, regardez le rapport de vitesse pendant que vous
appuyez et relâchez — la courbe est le propos.

## Étape de code

Un changement : le mouvement du héros devient celui du héros. `src/hero.h` et
`src/hero.cpp` sont nouveaux — `HeroMove` lit la direction maintenue, la
normalise (la diagonale à la vitesse en ligne droite) et fait converger en
douceur la vitesse du héros vers elle (accélération) ou vers le repos
(décélération). `src/game.cpp` n'écrit plus la requête de déplacement du héros
depuis les touches brutes (c'est celle de `HeroMove` désormais) et restaure le
héros au repos quand une nouvelle partie commence ; `src/main.cpp` appelle
`HeroMove` à chaque frame de jeu (avant la marche) et rapporte la vitesse du
héros telle qu'elle converge. La marche transforme toujours la vitesse lissée en
mouvement contre la carte — inchangée depuis la leçon 084. Son état final est
étiqueté `lesson-085`.

```diff
diff --git a/src/game.cpp b/src/game.cpp
index 0a78d42..4db9368 100644
--- a/src/game.cpp
+++ b/src/game.cpp
@@ -69,36 +69,26 @@ void GameInit(Game &game, int hero_health_full)
 void GameInput(Game &game, platform::Window *window, Entity &hero,
                double wall_dt)
 {
-    /* Play's movement is the hero's own request; every other state leaves
-       the hero at rest, so the walk moves nothing and the simulation
-       stands still. */
-    hero.move_x = 0.0;
-    hero.move_y = 0.0;
-
     switch (game.state) {
     case GAME_TITLE:
         /* The title screen accepts one thing: the start key. */
         if (platform::KeyPressed(window, platform::KEY_ENTER)) {
-            /* A fresh game restores the hero's health and the game's
-               waves — the row's facts, not remembered state. */
+            /* A fresh game restores the hero's health, the game's waves,
+               and the hero's rest — the row's facts, not remembered
+               state. */
             hero.health = game.hero_health_full;
             game.waves_remaining = GAME_WAVES;
             game.play_clock = 0.0;
+            hero.move_x = 0.0;
+            hero.move_y = 0.0;
             Transition(game, GAME_PLAY, "the player started");
         }
         break;
 
     case GAME_PLAY: {
-        /* Play's input: polled movement state, written to the hero's
-           request. The walk turns it into motion (lesson 076). */
-        if (platform::KeyDown(window, platform::KEY_LEFT))
-            hero.move_x -= 1.0;
-        if (platform::KeyDown(window, platform::KEY_RIGHT))
-            hero.move_x += 1.0;
-        if (platform::KeyDown(window, platform::KEY_UP))
-            hero.move_y -= 1.0;
-        if (platform::KeyDown(window, platform::KEY_DOWN))
-            hero.move_y += 1.0;
+        /* Play's movement is the hero's own (HeroMove, lesson 085) — the
+           held direction read and eased into motion there. What is left
+           here is the play state's other input and the named conditions. */
 
         /* The stand-in for combat: SPACE is a hit on the hero (lesson 087
            makes real hits land). The named condition below reads the
diff --git a/src/hero.cpp b/src/hero.cpp
new file mode 100644
index 0000000..7c119ae
--- /dev/null
+++ b/src/hero.cpp
@@ -0,0 +1,47 @@
+// hero.cpp — the hero's movement: intent in, eased motion out.
+//
+// Lesson 085: the whole of the hero's feel is here — the diagonal
+// normalized to the straight-line speed, and the velocity eased toward
+// the intent (or toward rest) so the hero reads as a thing with weight.
+
+#include "hero.h"
+
+namespace engine {
+
+void HeroMove(Entity &hero, platform::Window *window, double dt)
+{
+    /* The player's intent: the held direction, from polled input state
+       (lesson 032) — one step per frame, no events. */
+    double want_x = 0.0, want_y = 0.0;
+    if (platform::KeyDown(window, platform::KEY_LEFT))
+        want_x -= 1.0;
+    if (platform::KeyDown(window, platform::KEY_RIGHT))
+        want_x += 1.0;
+    if (platform::KeyDown(window, platform::KEY_UP))
+        want_y -= 1.0;
+    if (platform::KeyDown(window, platform::KEY_DOWN))
+        want_y += 1.0;
+
+    /* The intent is a direction. Normalized — a diagonal is scaled by
+       1/sqrt(2) — so the hero covers ground at the straight-line speed
+       whichever of the eight directions it runs in. */
+    double intent_x = want_x, intent_y = want_y;
+    if (intent_x != 0.0 && intent_y != 0.0) {
+        intent_x *= HERO_DIAG;
+        intent_y *= HERO_DIAG;
+    }
+
+    /* The ease: the velocity closes on the intent by dt/HERO_TIME each
+       frame — toward the intent when the player steers (acceleration),
+       toward rest when they let go (deceleration). A turn passes through
+       the ease instead of snapping to full speed the other way. The
+       hero's movement request carries the eased velocity; the walk turns
+       it into motion (move x speed = the velocity). */
+    double k = dt / HERO_TIME;
+    if (k > 1.0)
+        k = 1.0;
+    hero.move_x += (intent_x - hero.move_x) * k;
+    hero.move_y += (intent_y - hero.move_y) * k;
+}
+
+} /* namespace engine */
diff --git a/src/hero.h b/src/hero.h
new file mode 100644
index 0000000..cd658ff
--- /dev/null
+++ b/src/hero.h
@@ -0,0 +1,41 @@
+// hero.h — the hero's movement: the player's intent, eased into motion.
+//
+// Lesson 085: the hero is not a position that jumps to a new spot each
+// frame. It is a thing with weight — a velocity that eases toward where
+// the player means to go (acceleration) and toward rest (deceleration),
+// never a step change. And the intent is a *direction*, normalized, so
+// the diagonal is no faster than the straight run.
+//
+// This is the hero's own behavior (design D2), in its own file beside
+// the services and the game machine: the input becomes motion here, and
+// the walk turns that motion into steps against the map.
+#ifndef HERO_H
+#define HERO_H
+
+#include "entity.h"
+#include "platform.h"
+
+namespace engine {
+
+/* The hero's accel/decel time constant — the feel: roughly how long it
+   takes to ease from rest to full speed (or back). Lesson 085 keeps it
+   here as the hero's own fact; when the table format grows named
+   columns (lesson 087) the feel becomes data, like the hero's speed
+   already is. */
+constexpr double HERO_TIME = 0.12; /* seconds to close on the intent */
+
+/* 1 / sqrt(2): a diagonal intent is scaled by this so the hero covers
+   ground at the straight-line speed, not sqrt(2) times it. */
+constexpr double HERO_DIAG = 0.70710678;
+
+/* The hero's movement, once per frame of play. The held direction is the
+   intent, normalized so the diagonal is no faster than straight; the
+   hero's velocity eases toward that intent (accel) and toward rest
+   (decel) — a turn passes through the ease rather than snapping. The
+   result is left in the hero's own movement request, which the walk
+   turns into motion against the map. */
+void HeroMove(Entity &hero, platform::Window *window, double dt);
+
+} /* namespace engine */
+
+#endif
diff --git a/src/main.cpp b/src/main.cpp
index fda43b0..96436f6 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -20,6 +20,7 @@
 #include "frame.h"
 #include "game.h"
 #include "gametime.h"
+#include "hero.h"
 #include "platform.h"
 #include "sprite.h"
 #include "table.h"
@@ -324,6 +325,7 @@ int Run(void)
     double distance = 0.0; /* the score: the world the hero has walked */
     GameTime game_time = { GAMETIME_FULL }; /* lesson 078: the scale — set by the state now */
     bool was_blocked = false; /* lesson 077: the mover's state report */
+    int was_vx = 0, was_vy = 0; /* lesson 085: the hero's velocity, as it eases */
 
     /* The slice's identity: what the run is, named at once — L0*, the
        gate this part closes on. Every service it uses was finished
@@ -419,6 +421,12 @@ int Run(void)
 
         double was_x = hero.x, was_y = hero.y;
 
+        /* Lesson 085: the hero's movement — the held direction eased into
+           motion (accel/decel, the diagonal at the straight-line speed).
+           Only in play; the walk turns the eased velocity into steps. */
+        if (game.state == GAME_PLAY)
+            HeroMove(hero, opened.window, dt);
+
         /* Lesson 084: the game resolves its movement against its map —
            the walk is the game's now (GameWalk, in game.cpp), turning
            every live entity's request into motion through the mover.
@@ -447,6 +455,20 @@ int Run(void)
             std::printf("engine: hero at %d,%d (t=%.3f)\n", (int)hero.x,
                         (int)hero.y, platform::Now() - started);
 
+        /* Lesson 085: the hero's velocity, as it eases — the accel (the
+           speed rising over frames) and the decel (falling to rest) are
+           what the player feels, and this is the measurement of it. */
+        {
+            int vx = (int)(hero.move_x * hero.speed);
+            int vy = (int)(hero.move_y * hero.speed);
+            if (vx != was_vx || vy != was_vy) {
+                std::printf("engine: hero velocity %d,%d (t=%.3f)\n", vx, vy,
+                            platform::Now() - started);
+                was_vx = vx;
+                was_vy = vy;
+            }
+        }
+
         /* Lesson 083: the game's world-view — the camera's base follows
            the hero, clamped to the map's bounds, and its additive offset
            rests at exactly zero. The game owns the camera now (GameFollow,
```

## Exercices

Deux défis plus vastes. Chacun se termine par sa solution — un diff contre
l'état final de cette leçon, plus une visite guidée — après l'énoncé.

### Exercice 1 — La courbe, prédite *(predict-the-output)*

L'easing est de l'arithmétique, donc la trajectoire est prévisible. Depuis le
repos (`move = 0`), maintenir une direction donne `move += (1 − move) ×
dt/HERO_TIME` à chaque frame. À 60 fps stable (`dt = 1/60`) avec `HERO_TIME =
0.12`, calculez la vitesse du héros (en fraction de la pleine vitesse) après 1,
2 et 3 frames — atteint-elle la pleine vitesse en ces trois frames ? Doublez
ensuite `HERO_TIME` à `0.24` et prédisez de nouveau. Écrivez une petite sonde
(imprimez `hero.move_x` à chaque frame) et, sur une machine qui tourne à 60 fps,
vérifiez vos six nombres contre l'exécution. Laquelle atteint la pleine vitesse
plus tôt, et de combien de frames ?

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-085/ex1.md)

### Exercice 2 — Plus lourd à arrêter *(extend-the-code)*

Un seul `HERO_TIME` rend le départ et l'arrêt symétriques. Les vrais héros
montent souvent en vitesse vite mais s'arrêtent en roue libre — accélérer semble
net, s'arrêter semble pesant. Séparez la constante en deux : `HERO_ACCEL` et
`HERO_DECEL`, et utilisez la bonne selon que le héros accélère (l'intention le
pousse plus vite) ou ralentit (l'intention est nulle ou opposée). Gardez la
normalisation de la diagonale et l'indépendance à la fréquence de frames. Puis
lancez-le : le héros atteint-il la pleine vitesse nettement plus tôt qu'il ne
s'arrête ?

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-085/ex2.md)

---

**Partie :** [Partie 5 — le jeu](../../index.md) ·
**Précédente :** [Leçon 084 — la collision avec les tuiles](lesson-084-collision.md) ·
**Suivante :** [Leçon 086 — la rétroaction et l'animation](lesson-086-feedback-animation.md) ·
**Étiquette de code :** [`lesson-085`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-085)

*Page traduite de la version anglaise `book/lessons/part-5/lesson-085-hero-movement.md`,
révision `3e7f026`.*

<!-- translation-source: book/lessons/part-5/lesson-085-hero-movement.md @ 3e7f026 -->
