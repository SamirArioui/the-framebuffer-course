# Leçon 086 — la rétroaction et l'animation

{{#include ../../stability-horizon.md}}

## Prose

Le héros a du poids désormais, mais il glisse encore sur la carte comme une
statue et rien dans le jeu ne *réagit*. Cette leçon lui donne deux choses sur
lesquelles le travail de ressenti des leçons 092-093 s'appuiera : **le héros
s'anime quand il marche, et les deux hooks de rétroaction — screenshake et
hitstop — se déclenchent et se reposent tout seuls.**

### Le cycle de marche

L'art du héros est une **planche de sprites** désormais (`assets/hero.ppm`) :
deux frames 16×16 côte à côte — une foulée, et la même foulée un pas plus loin.
L'animation, c'est l'indice de frame qui avance pendant que le héros se déplace :

```cpp
if (want_x != 0.0 || want_y != 0.0) {
    hero.frame_t += dt;
    if (hero.frame_t >= ANIM_STEP) {
        hero.frame_t -= ANIM_STEP;
        hero.frame = (hero.frame + 1) % count;   /* the sheet's frames */
    }
} else {
    hero.frame = 0;                              /* at rest, frame 0 */
}
```

Une frame toutes les `ANIM_STEP`, avec retour au début à la fin de la planche —
un cycle de marche. Au repos, il tient la frame 0. Deux détails le gardent
honnête : une entité entre en collision en **une frame** (large de
`ANIM_FRAME_W`), pas en la planche entière — ce qu'elle dessine est ce contre
quoi elle entre en collision —, et le dessin parcourt la frame `hero.frame` de
la planche (`BlitSpriteFrame`) plutôt que l'image entière. D'une vraie
exécution, la frame qui avance pendant que le héros marche :

```
engine: hero frame 1 (t=3.348)
engine: hero frame 0 (t=7.863)
```

La frame quitte 0, passe à 1 et revient à 0 — le cycle qui avance pendant que
le héros marche et se repose quand il s'arrête.

### Les deux hooks de rétroaction

Un **screenshake** et un **hitstop** — les deux premiers des quatre effets de la
boîte à outils du juice (les leçons 092-093 ajoutent la rafale et l'easing et,
plus important, décident *quand* les déclencher depuis les événements du jeu).
Ici, ce sont les mécanismes, chacun une chose qui **se déclenche puis se
repose** :

- **Screenshake** déplace le décalage additif de la caméra (le hook de juice de
  la leçon 054) tant qu'il dure, et le repose à **exactement zéro**.
- **Hitstop** fait chuter l'échelle du temps de jeu à une fraction et la ramène
  à la **pleine vitesse à sa propre échéance de temps mural** — l'horloge de la
  leçon 078 : ce qui doit *finir* pendant que le jeu est arrêté ne peut pas
  tourner au temps de jeu.

L'échelle est celle de l'état (le jeu tourne, le reste tient) multipliée par le
facteur du hitstop — un bouton, deux pilotes, multipliés. D'une vraie exécution,
les deux hooks déclenchés une fois et reposés tout seuls :

```
engine: feel: shake fired (6 px, 0.5s), hitstop fired (0.25x, 0.4s)
engine: feel: hitstop rested — full speed again
engine: feel: shake rested at 0,0
```

Déclenchés, puis — sans que rien ne les annule — au repos. Le hitstop est revenu
à la pleine vitesse quand son échéance est passée ; la secousse s'est stabilisée
au décalage qu'elle avait promis : `0,0`, exactement.

### Ce que cette exécution a vérifié, et ce qu'elle n'a pas vérifié

- **L'animation avance** — la frame du cycle de marche quitte 0 et progresse
  pendant que le héros se déplace, et se repose à 0 quand il s'arrête.
- **Les hooks se déclenchent et se reposent** — la secousse se repose à
  exactement `0,0` ; le hitstop revient à la pleine vitesse à sa propre échéance.

Ce que cette leçon ne fait **pas**, c'est décider *quand* déclencher les hooks.
Une démonstration sur le temps mural les déclenche une fois ici ; la boîte à
outils (leçons 092-093) les déclenchera depuis les événements du jeu — un coup
touche, une rafale part — ce qui est la règle « la rétroaction commence avec
l'événement ». Elle n'apporte pas non plus la rafale ni l'easing : ce sont les
deux autres effets de la boîte à outils, et ils sont pour bientôt.

## Étape de code

Un changement : l'animation et la rétroaction. `src/feel.h` et `src/feel.cpp`
sont nouveaux — les deux hooks (`FeelShake`, `FeelHitstop`) et leur vie de
déclenchement-et-repos (`FeelUpdate` sur l'horloge murale, `FeelTimeScale` pour
l'échelle). `assets/hero.ppm` est une nouvelle planche de sprites (le cycle de
marche du héros), et la ligne du héros la nomme. `src/entity.h/.cpp` portent la
frame (et entrent en collision en une frame, pas en la planche) ;
`src/hero.h/.cpp` font avancer le cycle de marche ; `src/blit.h/.cpp` dessinent
une frame d'une planche ; `src/game.cpp` dessine la frame de l'entité et remet
le décalage additif de la caméra à la rétroaction (les hooks le possèdent
désormais) ; `src/main.cpp` déclenche la démonstration et replie le hitstop dans
l'échelle du temps de jeu. Son état final est étiqueté `lesson-086`.

```diff
diff --git a/assets/entities.txt b/assets/entities.txt
index 0683f17..f0635f1 100644
--- a/assets/entities.txt
+++ b/assets/entities.txt
@@ -1,3 +1,3 @@
 name x y facing speed health sprite
-hero 312 232 0 240 3 assets/sprite.ppm
+hero 312 232 0 240 3 assets/hero.ppm
 slime 400 320 2 96 1 assets/sprite.ppm
diff --git a/src/blit.cpp b/src/blit.cpp
index 1c52f0d..0d417f4 100644
--- a/src/blit.cpp
+++ b/src/blit.cpp
@@ -35,4 +35,31 @@ void BlitSprite(Framebuffer &fb, const Sprite &s, int x, int y)
     }
 }
 
+void BlitSpriteFrame(Framebuffer &fb, const Sprite &s, int src_x,
+                     int frame_w, int x, int y)
+{
+    /* BlitSprite, scoped to one frame_w-wide column of the sheet at
+       source x src_x: the source pixel's column is src_x + (i - x). */
+    int left = x < 0 ? 0 : x;
+    int top = y < 0 ? 0 : y;
+    int right = x + frame_w < fb.width ? x + frame_w : fb.width;
+    int bottom = y + s.height < fb.height ? y + s.height : fb.height;
+
+    for (int j = top; j < bottom; ++j) {
+        for (int i = left; i < right; ++i) {
+            const unsigned char *src =
+                &s.pixels[(((size_t)(j - y) * s.width) +
+                           (src_x + (i - x))) * 3];
+            if (src[0] == s.key_r && src[1] == s.key_g && src[2] == s.key_b)
+                continue; /* the transparent color writes nothing */
+            unsigned char *dst =
+                &fb.pixels[(((size_t)j * fb.width) + i) * 4];
+            dst[0] = src[2]; /* blue */
+            dst[1] = src[1]; /* green */
+            dst[2] = src[0]; /* red */
+            dst[3] = 0;
+        }
+    }
+}
+
 } /* namespace engine */
diff --git a/src/blit.h b/src/blit.h
index 259ddf3..3832e8b 100644
--- a/src/blit.h
+++ b/src/blit.h
@@ -20,6 +20,12 @@ namespace engine {
    lesson 015's fold at rectangle scale, never a wrap into other pixels. */
 void BlitSprite(Framebuffer &fb, const Sprite &sprite, int x, int y);
 
+/* Lesson 086: one frame of a sprite sheet — the `frame_w`-wide column of
+   the sheet starting at source x `src_x` — drawn at (x, y) exactly like
+   BlitSprite. A walk cycle is one sheet, and this draws one step. */
+void BlitSpriteFrame(Framebuffer &fb, const Sprite &sprite, int src_x,
+                     int frame_w, int x, int y);
+
 } /* namespace engine */
 
 #endif
diff --git a/src/entity.cpp b/src/entity.cpp
index d3c7d3b..b45cc1f 100644
--- a/src/entity.cpp
+++ b/src/entity.cpp
@@ -60,15 +60,17 @@ void MoveEntity(const TileMap &map, Entity &entity, double dx, double dy)
 {
     /* One axis at a time: a wall blocks the movement into it and the
        movement along it still works — the slide is this shape, not a
-       special case. The rectangle the map is asked about is the
-       entity's art: what it draws is what it collides as. */
+       special case. The rectangle the map is asked about is one frame of
+       the entity's art (lesson 086: an animated kind's sheet is several
+       frames wide, but what it draws — and so what it collides as — is
+       one ANIM_FRAME_W-wide frame). */
     double next_x = entity.x + dx;
     if (!TileRectSolid(map, (int)next_x, (int)entity.y,
-                       entity.sprite->width, entity.sprite->height))
+                       ANIM_FRAME_W, entity.sprite->height))
         entity.x = next_x;
     double next_y = entity.y + dy;
     if (!TileRectSolid(map, (int)entity.x, (int)next_y,
-                       entity.sprite->width, entity.sprite->height))
+                       ANIM_FRAME_W, entity.sprite->height))
         entity.y = next_y;
 }
 
diff --git a/src/entity.h b/src/entity.h
index c0ec0e7..560296f 100644
--- a/src/entity.h
+++ b/src/entity.h
@@ -26,7 +26,11 @@ struct Entity {
     int facing;                /* 0 right, 1 down, 2 left, 3 up */
     int speed;                 /* world pixels per second */
     int health;                /* points */
-    const Sprite *sprite;      /* the art it draws, from its row */
+    const Sprite *sprite;      /* the art it draws, from its row — for an
+                                  animated kind, its sprite sheet */
+    int frame;                 /* lesson 086: which frame of the sheet is
+                                  showing — the walk cycle advances it */
+    double frame_t;            /* and how long this frame has shown */
     double move_x, move_y;     /* lesson 076: this frame's movement
                                   request — the game sets it (the
                                   player's input for the hero, Part 5's
@@ -40,6 +44,11 @@ struct Entity {
    answered from the definition alone. */
 Entity EntityFromDef(const EntityDef &def);
 
+/* Lesson 086: every frame of a sprite sheet is this wide — a walk cycle
+   is a sheet of ANIM_FRAME_W-wide frames. An entity collides as one
+   frame (what it draws), not as the whole sheet. */
+constexpr int ANIM_FRAME_W = 16;
+
 /* Lesson 074: the store's capacity — a decision, made here and named in
    the closing review. Sixty-four live entities: the hero, the enemy
    types, and a screenful of projectiles. What the game does when it is
diff --git a/src/feel.cpp b/src/feel.cpp
new file mode 100644
index 0000000..2651e0c
--- /dev/null
+++ b/src/feel.cpp
@@ -0,0 +1,71 @@
+// feel.cpp — the feedback hooks: fire, decay, rest.
+//
+// Lesson 086: each hook fires, runs down its own wall-time, and returns
+// exactly to rest. Nothing here decides *when* to fire — that is the
+// juice toolkit's job (lessons 092-093), reading the game's events.
+
+#include "feel.h"
+
+#include <cstdio>
+
+namespace engine {
+
+void FeelInit(Feedback &feel)
+{
+    feel.shake = 0.0;
+    feel.shake_mag = 0.0;
+    feel.hitstop = 0.0;
+    feel.hitstop_k = 0.0;
+}
+
+void FeelShake(Feedback &feel, double magnitude, double seconds)
+{
+    feel.shake = seconds;
+    feel.shake_mag = magnitude;
+}
+
+void FeelHitstop(Feedback &feel, double fraction, double seconds)
+{
+    feel.hitstop = seconds;
+    feel.hitstop_k = fraction;
+}
+
+double FeelTimeScale(const Feedback &feel)
+{
+    /* At rest the factor is full speed; during a hitstop it is the
+       fraction the hitstop was fired at. */
+    return feel.hitstop > 0.0 ? feel.hitstop_k : GAMETIME_FULL;
+}
+
+void FeelUpdate(Feedback &feel, double wall_dt, Camera &camera)
+{
+    /* The hitstop runs on its own wall-time and returns to full speed
+       when its deadline passes — the game need not remember to undo it. */
+    if (feel.hitstop > 0.0) {
+        feel.hitstop -= wall_dt;
+        if (feel.hitstop < 0.0) {
+            feel.hitstop = 0.0;
+            std::printf("engine: feel: hitstop rested — full speed again\n");
+        }
+    }
+
+    /* The screenshake drives the camera's additive offset — the juice
+       hook lesson 054 defined. While it lasts the offset alternates;
+       when it ends the offset rests at exactly zero. */
+    if (feel.shake > 0.0) {
+        feel.shake -= wall_dt;
+        if (feel.shake <= 0.0) {
+            feel.shake = 0.0;
+            camera.add_x = 0;
+            camera.add_y = 0;
+            std::printf("engine: feel: shake rested at %d,%d\n", camera.add_x,
+                        camera.add_y);
+        } else {
+            camera.add_x = ((int)(feel.shake * 40.0) & 1) ? (int)feel.shake_mag
+                                                          : -(int)feel.shake_mag;
+            camera.add_y = 0;
+        }
+    }
+}
+
+} /* namespace engine */
diff --git a/src/feel.h b/src/feel.h
new file mode 100644
index 0000000..d8706e2
--- /dev/null
+++ b/src/feel.h
@@ -0,0 +1,48 @@
+// feel.h — the feedback hooks the juice toolkit will drive.
+//
+// Lesson 086: two hooks — a screenshake and a hitstop — each a thing
+// that fires and then rests. These are the hooks the juice toolkit
+// (lessons 092-093) will drive from the game's events: here they are the
+// mechanisms, each with its own fire-and-rest life, and nothing yet says
+// when to fire them. A hook at rest costs nothing and changes nothing.
+#ifndef FEEL_H
+#define FEEL_H
+
+#include "camera.h"
+#include "gametime.h"
+
+namespace engine {
+
+/* The feedback state: what is still firing. Every field is at rest at
+   zero — a hook that has fired and finished leaves itself exactly here. */
+struct Feedback {
+    double shake;     /* seconds of screenshake left; 0 = at rest */
+    double shake_mag; /* the shake's offset while it lasts, pixels */
+    double hitstop;   /* seconds of hitstop left; 0 = full speed */
+    double hitstop_k; /* the fraction of game time during the hitstop */
+};
+
+void FeelInit(Feedback &feel);
+
+/* Fire a screenshake: the camera's additive offset moves for `seconds`,
+   then returns to rest — exactly zero. */
+void FeelShake(Feedback &feel, double magnitude, double seconds);
+
+/* Fire a hitstop: the game-time scale drops to `fraction` and returns to
+   full speed on its own wall-time deadline (lesson 078's clock: what
+   must end while the game is stopped cannot run on game time). */
+void FeelHitstop(Feedback &feel, double fraction, double seconds);
+
+/* The game-time factor the hitstop applies right now — a fraction during
+   a hitstop, full speed at rest. The state's own scale multiplies this,
+   so a pause still freezes and a hitstop only slows play. */
+double FeelTimeScale(const Feedback &feel);
+
+/* Advance the feedback once a frame on the wall clock: each hook decays
+   toward rest and drives what it owns — the shake, the camera's additive
+   offset (resting at exactly zero). */
+void FeelUpdate(Feedback &feel, double wall_dt, Camera &camera);
+
+} /* namespace engine */
+
+#endif
diff --git a/src/game.cpp b/src/game.cpp
index 4db9368..967f072 100644
--- a/src/game.cpp
+++ b/src/game.cpp
@@ -207,8 +207,8 @@ void GameFollow(Game &game, const Entity &hero, const TileMap &map)
         game.camera.base_y = base_y;
         std::printf("engine: camera base %d,%d\n", base_x, base_y);
     }
-    game.camera.add_x = 0;
-    game.camera.add_y = 0;
+    /* The camera's additive offset is the juice hook — lesson 086's
+       feedback (FeelUpdate) drives it now, and rests it at zero. */
 }
 
 void GameDrawMap(const Game &game, Framebuffer &fb, const TileMap &map,
@@ -228,8 +228,9 @@ void GameDrawSprites(const Game &game, Framebuffer &fb,
         if (!store.slots[i].live)
             continue;
         const Entity &e = store.slots[i];
-        BlitSprite(fb, *e.sprite, (int)e.x - CameraX(game.camera),
-                   (int)e.y - CameraY(game.camera));
+        BlitSpriteFrame(fb, *e.sprite, e.frame * ANIM_FRAME_W, ANIM_FRAME_W,
+                        (int)e.x - CameraX(game.camera),
+                        (int)e.y - CameraY(game.camera));
     }
 }
 
diff --git a/src/hero.cpp b/src/hero.cpp
index 7c119ae..a2798ab 100644
--- a/src/hero.cpp
+++ b/src/hero.cpp
@@ -42,6 +42,23 @@ void HeroMove(Entity &hero, platform::Window *window, double dt)
         k = 1.0;
     hero.move_x += (intent_x - hero.move_x) * k;
     hero.move_y += (intent_y - hero.move_y) * k;
+
+    /* Lesson 086: the walk cycle — the frame advances while the hero
+       steps, one frame per ANIM_STEP, and holds at frame 0 at rest. The
+       sheet's frame count is its width over one frame's width. */
+    if (want_x != 0.0 || want_y != 0.0) {
+        hero.frame_t += dt;
+        if (hero.frame_t >= ANIM_STEP) {
+            hero.frame_t -= ANIM_STEP;
+            int count = hero.sprite->width / ANIM_FRAME_W;
+            if (count < 1)
+                count = 1;
+            hero.frame = (hero.frame + 1) % count;
+        }
+    } else {
+        hero.frame = 0;
+        hero.frame_t = 0.0;
+    }
 }
 
 } /* namespace engine */
diff --git a/src/hero.h b/src/hero.h
index cd658ff..c602349 100644
--- a/src/hero.h
+++ b/src/hero.h
@@ -28,6 +28,9 @@ constexpr double HERO_TIME = 0.12; /* seconds to close on the intent */
    ground at the straight-line speed, not sqrt(2) times it. */
 constexpr double HERO_DIAG = 0.70710678;
 
+/* Lesson 086: how long one frame of the walk cycle shows. */
+constexpr double ANIM_STEP = 0.12;
+
 /* The hero's movement, once per frame of play. The held direction is the
    intent, normalized so the diagonal is no faster than straight; the
    hero's velocity eases toward that intent (accel) and toward rest
diff --git a/src/main.cpp b/src/main.cpp
index 96436f6..e1993f9 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -15,6 +15,7 @@
 #include "audio.h"
 #include "blit.h"
 #include "entity.h"
+#include "feel.h"
 #include "font.h"
 #include "framebuffer.h"
 #include "frame.h"
@@ -248,6 +249,14 @@ int Run(void)
     Game game;
     GameInit(game, hero.health);
 
+    /* Lesson 086: the feedback hooks — a screenshake and a hitstop, both
+       at rest. The juice toolkit (lessons 092-093) will fire these from
+       the game's events; here a demonstration fires both once so their
+       fire-and-rest life is visible. */
+    Feedback feel;
+    FeelInit(feel);
+    bool feel_demo = false;
+
     /* Lesson 080: the vertical slice — the game's shape, and nothing
        else. The hero is the row the game asks for by name (it is the
        one the player controls); the world's other kinds come from the
@@ -326,6 +335,7 @@ int Run(void)
     GameTime game_time = { GAMETIME_FULL }; /* lesson 078: the scale — set by the state now */
     bool was_blocked = false; /* lesson 077: the mover's state report */
     int was_vx = 0, was_vy = 0; /* lesson 085: the hero's velocity, as it eases */
+    int was_frame = 0;          /* lesson 086: the hero's walk-cycle frame */
 
     /* The slice's identity: what the run is, named at once — L0*, the
        gate this part closes on. Every service it uses was finished
@@ -409,7 +419,25 @@ int Run(void)
            screen. The hero's movement request is written here (play's
            arrows) and left at rest in every other state. */
         GameInput(game, opened.window, hero, wall_dt);
-        GameTimeSetScale(game_time, GameScale(game));
+
+        /* Lesson 086: the feedback hooks run on their own wall-time —
+           each fires, decays, and rests. The demonstration fires both
+           once, in play, so their fire-and-rest life is visible; the
+           juice toolkit (lessons 092-093) will fire them from the game's
+           events instead of this script. */
+        if (!feel_demo && game.state == GAME_PLAY &&
+            platform::Now() - started >= 3.0) {
+            feel_demo = true;
+            FeelShake(feel, 6.0, 0.5);
+            FeelHitstop(feel, 0.25, 0.4);
+            std::printf("engine: feel: shake fired (6 px, 0.5s), hitstop fired (0.25x, 0.4s)\n");
+        }
+        FeelUpdate(feel, wall_dt, game.camera);
+
+        /* The game-time scale is the state's (play runs, the rest hold)
+           times the hitstop's factor (a fraction during a hitstop, full
+           at rest) — one knob, two drivers, multiplied. */
+        GameTimeSetScale(game_time, GameScale(game) * FeelTimeScale(feel));
 
         /* Lesson 078: the update advances by game time — the wall
            clock's step, scaled. Everything the simulation does with dt
@@ -469,6 +497,14 @@ int Run(void)
             }
         }
 
+        /* Lesson 086: the walk cycle — the frame advances while the hero
+           steps, and this is the measurement of it advancing. */
+        if (hero.frame != was_frame) {
+            std::printf("engine: hero frame %d (t=%.3f)\n", hero.frame,
+                        platform::Now() - started);
+            was_frame = hero.frame;
+        }
+
         /* Lesson 083: the game's world-view — the camera's base follows
            the hero, clamped to the map's bounds, and its additive offset
            rests at exactly zero. The game owns the camera now (GameFollow,
```

## Exercices

Deux défis plus vastes. Chacun se termine par sa solution — un diff contre
l'état final de cette leçon, plus une visite guidée — après l'énoncé.

### Exercice 1 — La rétroaction commence avec l'événement *(extend-the-code)*

Les hooks se déclenchent ici sur un script de temps mural, mais la règle du
game feel est qu'un effet de ressenti commence **dans la frame où son événement
déclencheur se produit**. Faites commencer la rétroaction avec l'événement :
quand le héros prend un coup (le substitut de combat sur la touche espace),
déclenchez un court hitstop et un petit screenshake — le coup *atterrit* avec du
poids. Gardez le déclenchement-et-repos propre aux hooks (ils doivent toujours
revenir à la pleine vitesse et à `0,0` tout seuls) ; le changement porte sur *ce
qui les déclenche*. Puis lancez-le et appuyez sur espace : le coup se lit-il
comme un seul moment — la secousse et le ralentissement démarrant sur la frame
du coup ?

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-086/ex1.md)

### Exercice 2 — Pourquoi le temps mural *(explain-in-prose)*

Les deux hooks s'écoulent sur leur propre **temps mural**, pas sur le temps de
jeu. Défendez ce choix dans vos propres mots. Concrètement : un hitstop règle
l'échelle du temps de jeu à une fraction — si le compte à rebours *propre* du
hitstop tournait au temps de jeu, que se passerait-il quand l'échelle est à une
fraction (le hitstop finirait-il un jour) ? Et que se passerait-il pour un
screenshake si le jeu était mis en pause au milieu de la secousse et que sa
décroissance tournait au temps de jeu ? Répondez ensuite à la question de
mesure : l'exécution ci-dessus montre `hitstop rested — full speed again` et
`shake rested at 0,0` — que prouvent ces deux lignes sur l'horloge qu'ont
utilisée les hooks ?

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-086/ex2.md)

---

**Partie :** [Partie 5 — le jeu](../../index.md) ·
**Précédente :** [Leçon 085 — le mouvement du héros](lesson-085-hero-movement.md) ·
**Suivante :** [Leçon 087 — les projectiles et les deux armes](lesson-087-projectiles-weapons.md) ·
**Étiquette de code :** [`lesson-086`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-086)

*Page traduite de la version anglaise `book/lessons/part-5/lesson-086-feedback-animation.md`,
révision `3e7f026`.*

<!-- translation-source: book/lessons/part-5/lesson-086-feedback-animation.md @ 3e7f026 -->
