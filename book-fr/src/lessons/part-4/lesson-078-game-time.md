# Leçon 078 — l'échelle du temps de jeu

{{#include ../../stability-horizon.md}}

## Prose

Chaque pas que ce jeu a fait depuis la leçon 035 a été multiplié par une seule
chose : l'horloge murale. `dt` est ce que l'horloge de la plateforme dit s'être
écoulé, et la simulation avance d'exactement cela. C'est honnête et c'est aussi
inflexible — parce que les deux effets que veut tout jeu de ce genre sont *la
même requête* : « le monde devrait ralentir ». Un écran de pause, c'est le
monde à vitesse nulle. Le hitstop — le juice de la partie 5, cette demi-seconde
de quasi-immobilité quand un coup touche — c'est le monde à une fraction. Et
pour l'instant, il n'y a rien à régler. L'idée de cette leçon est donc le hook
que la partie 5 actionnera : **l'échelle du temps de jeu** — un nombre que le
jeu règle, de l'arrêt à la pleine vitesse, et le pas de la simulation est le
pas de l'horloge murale mis à l'échelle.

### Un nombre, trois réglages

```cpp
struct GameTime {
    double scale;
};

void GameTimeSetScale(GameTime &time, double scale);
double GameTimeStep(const GameTime &time, double wall_dt);
```

Le bouton est un seul `double`. `GameTimeSetScale` est la façon dont le jeu le
tourne — et il va de `0.0` à `GAMETIME_FULL` (1.0), pas plus loin : une valeur
hors de cette plage est bornée, donc « plus rapide que le jeu » et « le temps à
l'envers » ne sont pas des accidents qu'un appelant peut avoir. Les trois
réglages que le jeu fait sont le même appel :

| Réglage | Échelle | Le pas |
| ------- | ------- | ------ |
| pause | 0 | aucun — la simulation reste immobile |
| hitstop | une fraction (la démo utilise 0.25) | le pas mural, divisé par deux puis encore par deux |
| jeu | `GAMETIME_FULL` | le pas mural, inchangé |

L'échelle est *une valeur que le jeu possède*. Rien dans le moteur ne la règle ;
le script de la démo le fait, exactement comme le feront l'écran de pause de la
partie 5 (L1) et son hitstop (L11). Et la valeur en vigueur pour une frame est
celle que le pas de cette frame reflète — réglez l'échelle en milieu de frame
et le pas de cette frame la porte, parce que le pas est calculé une fois, là où
l'update commence.

### Le pas est la seule chose qu'elle atteint

La première arithmétique de l'update tient désormais en deux lignes :

```cpp
double wall_dt = now - last;
double dt = GameTimeStep(game_time, wall_dt);
```

`dt` — le nombre par lequel la marche multiplie la vitesse du héros — est le
temps de jeu. À l'échelle 0, il vaut 0 et le héros ne bouge pas ; à 0.25, il
vaut un quart du pas mural et le héros rampe ; à pleine vitesse, il vaut le pas
mural et rien du jeu d'aujourd'hui ne change. L'échelle atteint ce nombre et
**rien d'autre**. L'horloge de la plateforme reste celle qui mesure, comme la
leçon 035 le lui a appris ; l'enregistrement de frame continue de mesurer les
durées de l'horloge murale ; l'étape audio alimente toujours le périphérique à
l'horizon du tampon (le son ne ralentit pas quand le jeu se met en pause — un
jeu en pause garde sa musique). Cette dernière moitié est tout le sujet de la
leçon 079 ; l'affirmation de cette leçon est seulement que le pas est là où
l'échelle atterrit.

Pourquoi une échelle et non trois mécanismes : une pause qui serait « l'update
ne tourne pas » laisserait le jeu incapable de lire sa propre entrée (comment
*reprendre* ?), et un hitstop qui serait « une seconde horloge, plus lente »
ferait deux horloges à démêler dans une boucle dont la leçon 035 a soutenu
qu'elle devait en avoir exactement une. Un seul bouton, tourné vers différentes
positions, est la pause, le hitstop et le jeu.

### Ce que cette exécution a vérifié, et ce qu'elle n'a pas vérifié

D'une vraie exécution de l'état final de cette leçon sur cette machine, pilotée
par une entrée scriptée pendant que le script de la démo tournait le bouton à
trois, cinq et sept secondes :

- **Le pas se met à l'échelle à 0, à une fraction et à pleine vitesse.**
  L'exécution nomme chaque transition avec le pas qui en sort :
  `engine: game-time: scale 0.25 (hitstop) — step 3.807 ms of a 15.230 ms wall step`
  — 15.230 × 0.25 = 3.807, au chiffre près ;
  `engine: game-time: scale 0.00 (pause) — step 0.000 ms of a 1018.469 ms wall step`
  — un pas mural d'une seconde pleine (la boucle a dormi entre les nouvelles
  d'entrée) et un pas de jeu nul ;
  `engine: game-time: scale 1.00 (play) — step 15.122 ms of a 15.122 ms wall step`
  — le pas mural, entier.
- **Le monde lui obéit.** Les rapports de position du héros montrent l'échelle
  à l'œuvre : en jeu, un pas de 3-4 pixels par frame appuyée (`hero at
  705,232` … `709,232`) ; pendant le hitstop, un *pixel* par frame (`hero at
  728,232`, `729`, `730`, `731` … — les mêmes appuis, un quart du mouvement) ;
  pendant la pause, aucun mouvement du tout entre t 5.0 et t 7.0, et
  l'exécution a continué de dessiner et de rapporter tout du long.

Ce que cette leçon ne vérifie **pas**, c'est l'enregistrement de frame —
l'affirmation que la mesure n'est pas mise à l'échelle. C'est la leçon 079, et
c'est le genre d'affirmation qui mérite sa propre page : la différence entre
« le jeu est en pause » et « la machine est au repos » est exactement ce que
l'enregistrement de frame doit continuer de montrer.

## Étape de code

Un seul changement pour cette leçon, de l'horloge murale au temps de jeu :
`src/gametime.h` / `src/gametime.cpp` accueillent `GameTime`, le bornage de
l'échelle et `GameTimeStep` — le pas de l'horloge murale, mis à l'échelle.
`src/main.cpp` accueille l'exécution autour : le bouton dans la comptabilité de
l'exécution (en jeu), le script de la démo qui le tourne à travers hitstop,
pause et jeu à trois, cinq et sept secondes, et le pas de l'update calculé via
`GameTimeStep` là où `dt` était la différence brute d'horloge. La marche, le
mover, le magasin, le son et l'enregistrement de frame sont intacts — ce qui
est exactement le but. Son état final est étiqueté `lesson-078`.

```diff
diff --git a/src/gametime.cpp b/src/gametime.cpp
new file mode 100644
index 0000000..75bff29
--- /dev/null
+++ b/src/gametime.cpp
@@ -0,0 +1,24 @@
+// gametime.cpp — the scale, and the step it makes.
+//
+// Lesson 078: two functions, one idea. The scale is a value the game
+// sets; the step is the wall clock's, multiplied.
+
+#include "gametime.h"
+
+namespace engine {
+
+void GameTimeSetScale(GameTime &time, double scale)
+{
+    if (scale < 0.0)
+        scale = 0.0;
+    if (scale > GAMETIME_FULL)
+        scale = GAMETIME_FULL;
+    time.scale = scale;
+}
+
+double GameTimeStep(const GameTime &time, double wall_dt)
+{
+    return wall_dt * time.scale;
+}
+
+} /* namespace engine */
diff --git a/src/gametime.h b/src/gametime.h
new file mode 100644
index 0000000..2948440
--- /dev/null
+++ b/src/gametime.h
@@ -0,0 +1,36 @@
+// gametime.h — game time: the wall clock, scaled by one knob.
+//
+// Lesson 078: the simulation does not advance by the wall clock. It
+// advances by game time, and game time is the wall clock's step scaled
+// by one number the game sets: 0 is pause, a fraction is hitstop, full
+// speed is play. One knob — not a special case threaded through the
+// update, and not a second clock.
+#ifndef GAMETIME_H
+#define GAMETIME_H
+
+namespace engine {
+
+/* Full speed: the scale's top, where a run starts and where play
+   lives. */
+constexpr double GAMETIME_FULL = 1.0;
+
+/* The game-time scale — the one number the game owns. */
+struct GameTime {
+    double scale;
+};
+
+/* The scale the game sets. It runs from 0 (paused) to GAMETIME_FULL
+   (play) and no further: a value outside that range is clamped, so
+   "faster than play" and "backwards" are not accidents a caller can
+   have. */
+void GameTimeSetScale(GameTime &time, double scale);
+
+/* The simulation's step: the wall clock's step, scaled. This is the
+   number the update advances the world by, and the only thing the
+   scale reaches — the platform clock stays the measurer and the frame
+   record keeps measuring wall clock (lesson 079). */
+double GameTimeStep(const GameTime &time, double wall_dt);
+
+} /* namespace engine */
+
+#endif
diff --git a/src/main.cpp b/src/main.cpp
index dbc83e7..ee2f1b4 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -19,6 +19,7 @@
 #include "font.h"
 #include "framebuffer.h"
 #include "frame.h"
+#include "gametime.h"
 #include "platform.h"
 #include "sprite.h"
 #include "table.h"
@@ -305,6 +306,8 @@ int Run(void)
     double last = started;
     double distance = 0.0; /* the score: the world the hero has walked */
     int shake_frames = 0; /* lesson 054: the additive hook's demo */
+    GameTime game_time = { GAMETIME_FULL }; /* lesson 078: the scale, at play */
+    int scale_phase = 0;  /* lesson 078: the demo's script, by wall seconds */
     bool was_blocked = false; /* lesson 077: the mover's state report */
 
     /* The demo's identity: what the run is, named at once — the hero,
@@ -378,9 +381,41 @@ int Run(void)
 
         /* Update: a frame reads state — it never handles events. */
         double now = platform::Now();
-        double dt = now - last;
+        double wall_dt = now - last;
         last = now;
 
+        /* Lesson 078: the game-time scale — the one knob the game sets.
+           The demo's script is the game here: play, then hitstop (a
+           fraction of full speed), then pause (0), then play again —
+           the same three settings Part 5's juice toolkit and pause
+           screen will make. Each transition names the step that comes
+           out: the wall clock's step, scaled. */
+        double running = now - started;
+        if (scale_phase == 0 && running >= 3.0) {
+            GameTimeSetScale(game_time, 0.25);
+            scale_phase = 1;
+            std::printf("engine: game-time: scale %.2f (hitstop) — step %.3f ms of a %.3f ms wall step\n",
+                        game_time.scale, GameTimeStep(game_time, wall_dt) * 1e3,
+                        wall_dt * 1e3);
+        } else if (scale_phase == 1 && running >= 5.0) {
+            GameTimeSetScale(game_time, 0.0);
+            scale_phase = 2;
+            std::printf("engine: game-time: scale %.2f (pause) — step %.3f ms of a %.3f ms wall step\n",
+                        game_time.scale, GameTimeStep(game_time, wall_dt) * 1e3,
+                        wall_dt * 1e3);
+        } else if (scale_phase == 2 && running >= 7.0) {
+            GameTimeSetScale(game_time, GAMETIME_FULL);
+            scale_phase = 3;
+            std::printf("engine: game-time: scale %.2f (play) — step %.3f ms of a %.3f ms wall step\n",
+                        game_time.scale, GameTimeStep(game_time, wall_dt) * 1e3,
+                        wall_dt * 1e3);
+        }
+
+        /* Lesson 078: the update advances by game time — the wall
+           clock's step, scaled. Everything the simulation does with dt
+           is scaled; nothing else is. */
+        double dt = GameTimeStep(game_time, wall_dt);
+
         /* Lesson 076: the hero's intent — polled input state, read once
            per frame and written to the hero's own movement request. The
            walk turns every entity's request into motion; the game never
```

## Exercices

Deux exercices mixtes. Chacun se termine par sa solution — un diff contre
l'état final de cette leçon, plus une visite guidée — après l'énoncé.

### Exercice 1 — Un hitstop qui se termine *(extend-the-code)*

Un hitstop que personne ne termine est une pause. Donnez au jeu un hitstop
qu'il peut *déclencher* : un appel qui règle l'échelle à une fraction et se
souvient de la remettre à pleine vitesse après une durée — puis répondez à la
question que cet appel impose : **une durée de quoi ?** Si le compte à rebours
tourne en temps de jeu, un hitstop de 0,2 s à l'échelle 0.25 dure 0,8 s de
temps réel ; s'il tourne en temps mural, il dure 0,2 s et ce n'est pas
l'horloge du jeu qui l'a terminé. Choisissez-en un, câblez-le à la démo (la
touche espace est libre dans l'exécution de cette leçon si vous laissez le
screenshake au repos) et vérifiez par une exécution que l'échelle revient toute
seule. Lequel avez-vous choisi, et que donne chaque choix au ressenti ?

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-078/ex1.md)

### Exercice 2 — Un bouton, pas trois *(explain-in-prose)*

Pause et hitstop sont le même appel à des positions différentes, et c'est une
affirmation de design. Défendez-la dans vos propres mots, le code sous les
yeux : que fait encore l'update à l'échelle 0 (nommez les choses qui tournent
et celle qui ne tourne pas), et pourquoi un écran de pause *a-t-il besoin* que
ces choses continuent de tourner ? Qu'est-ce qui casserait si l'échelle
atteignait l'horloge de la plateforme au lieu du pas (déroulez l'enregistrement
de frame et l'étape audio dans votre réponse) ? Et quel est l'argument *contre*
ce design — quand un jeu voudrait-il que la pause soit un mécanisme entièrement
différent ?

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-078/ex2.md)

---

**Partie :** [Partie 4 — les services](../../index.md) ·
**Précédente :** [Leçon 077 — le mover sur une entité](lesson-077-mover.md) ·
**Suivante :** [Leçon 079 — la mesure n'est pas mise à l'échelle](lesson-079-wall-clock.md) ·
**Étiquette de code :** [`lesson-078`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-078)

*Page traduite de la version anglaise `book/lessons/part-4/lesson-078-game-time.md`,
révision `cc66198`.*

<!-- translation-source: book/lessons/part-4/lesson-078-game-time.md @ cc66198 -->
