# Leçon 035 — l'horloge de la plateforme

{{#include ../../stability-horizon.md}}

## Prose

La leçon 034 se terminait sur une excuse dans le code : la *vitesse* du
marqueur appartenait encore au clavier, parce que les frames advenaient au
rythme des nouvelles. Aujourd'hui, le moteur obtient sa propre horloge — la
même horloge monotone que la leçon 020 vous a appris à lire dans `snek`,
désormais derrière la couture de plateforme — et le temps devient quelque
chose que le moteur mesure, au lieu de quelque chose qui lui arrive. À partir
d'ici, le déplacement se compte en **pixels par seconde**.

### Le contrat de l'horloge

Une fonction sur la couture :

```c++
double Now(void);
```

Des secondes depuis un point de départ arbitraire — et c'est le contrat qui
compte :

- **Monotone.** Les lectures ne reculent jamais. Pas « en général » : jamais,
  et aucune correction, de qui que ce soit, ne peut les faire reculer.
- **Insensible à l'horloge murale.** Régler l'horloge système en avant, en
  arrière ou de travers ne change rien ici. L'horloge murale répond à
  « quelle heure est-il ? » ; celle-ci répond à « depuis combien de temps ? »
  — et seule la seconde question a une réponse qu'une frame peut soustraire.
- **Assez fine pour une frame.** Sa résolution est très en dessous de la
  milliseconde — l'horloge sait mesurer ce que la leçon 036 mettra dans
  l'enregistrement de frame.

Derrière la couture, l'implémentation est l'appel que la leçon 020 enseignait
déjà : `clock_gettime(CLOCK_MONOTONIC, ...)`, l'interface d'horloge de POSIX.
Le `CLOCK_REALTIME` d'à côté est l'horloge murale, et ce n'est *pas* elle que
la couture expose — l'exercice 2 met les deux côte à côte et explique
pourquoi.

### Le contrat, vérifié

Le moteur vérifie l'horloge avant que quoi que ce soit en dépende — cent mille
échantillons d'affilée, à guetter une lecture plus petite que la précédente et
le plus fin écart entre deux lectures :

```
$ DISPLAY=:99 ./build/game
engine: clock never backwards over 100000 samples, finest step 20 ns
engine: arrow keys move the marker; close the window to stop
engine: marker at 308,228
...
```

Jamais en arrière sur cent mille échantillons, et le pas le plus fin entre
deux d'entre eux est de vingt nanosecondes — quatre ordres de grandeur sous la
sous-milliseconde que le contrat exige. (La valeur absolue de `Now()` ne veut
rien dire — environ 37 725 secondes sur la machine où ceci a été rédigé, à peu
près son temps de fonctionnement. Point de départ arbitraire, rappelez-vous.)

### Le pas est vitesse × temps écoulé

La mise à jour se réduit à une arithmétique que n'importe qui peut vérifier :

```c++
double dt = now - last;
if (platform::KeyDown(opened.window, platform::KEY_RIGHT))
    marker_x += MARKER_SPEED * dt;
```

`MARKER_SPEED` vaut 240 — *pixels par seconde* — et `dt` le convertit en pas
de cette frame. Si les frames arrivent vite, les pas sont petits ; si une
frame est lente, son pas est proportionnellement plus grand ; la vitesse est
la même dans les deux cas. Le clavier ne décide plus rien d'autre que la
direction.

La preuve est une comparaison : faites courir le marqueur une seconde avec des
frames rapides, puis de nouveau avec des frames trois fois plus lentes. Même
durée, même touche, même vitesse — le marqueur doit parcourir la même
distance.

```
frames at 33 ms:  engine: marker at 548,228 (t=1.002)
frames at 100 ms: engine: marker at 548,228 (t=1.002)
```

Déplacement identique au même instant, malgré un écart de 3× dans la fréquence
de frames. Voilà ce que signifie « le moteur possède sa vitesse », mesuré.

### À quoi sert l'horloge autour de la couture

Cette horloge n'est pas un accessoire d'une seule leçon. Le chronométrage des
frames de la partie 2 y branche ses instruments, le rapport de budget de
frames de la partie 5 la lit, et la leçon 036 ouvre l'enregistrement par frame
à partir duquel l'une et l'autre grandiront — la durée de la frame elle-même,
comme donnée que le moteur garde. Le plafond de frames dont `snek` avait
besoin (le `SleepSec` de la leçon 020) est encore devant nous, sous une forme
ou une autre : avec `dt` en main, le rythme des frames devient une question de
*politique* (à quelle fréquence les frames doivent-elles advenir ?) plutôt
qu'un prérequis de correction.

## Étape de code

Un seul changement pour cette leçon : la couture gagne `Now` — l'horloge
monotone de la plateforme — le moteur vérifie son contrat au démarrage et
déplace le marqueur par `speed × dt` au lieu d'un pas fixe, et le rapport
transporte l'instant de chaque déplacement. (Un include inutilisé qui traînait
depuis le débogage de la leçon 031 est rangé au passage dans le même fichier.)
Son état final est étiqueté `lesson-035`.

```diff
diff --git a/src/main.cpp b/src/main.cpp
index 47d7360..c66a75f 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -12,11 +12,10 @@
 
 namespace engine {
 
-/* The marker: one square the arrow keys move. Its position is whole
-   pixels and its speed is pixels-per-frame — lesson 035's clock turns
-   that into pixels-per-second. */
+/* The marker: one square the arrow keys move. Its speed is the engine's —
+   pixels per second — and the clock's dt turns it into a per-frame step. */
 constexpr int MARKER_SIZE = 24;
-constexpr int MARKER_STEP = 8;
+constexpr double MARKER_SPEED = 240.0; /* pixels per second */
 
 static void DrawMarker(Framebuffer &fb, int x, int y)
 {
@@ -47,13 +46,33 @@ int Run(void)
         return 1;
     }
 
+    /* The clock's contract, checked before anything depends on it: the
+       readings never go backwards, and the finest step between two of them
+       is far below a frame. */
+    double prev = platform::Now();
+    double finest = 1e9;
+    int backwards = 0;
+    for (int i = 0; i < 100000; ++i) {
+        double t = platform::Now();
+        if (t < prev)
+            ++backwards;
+        else if (t > prev && t - prev < finest)
+            finest = t - prev;
+        prev = t;
+    }
+    std::printf("engine: clock %s over 100000 samples, finest step %.0f ns\n",
+                backwards ? "WENT BACKWARDS" : "never backwards", finest * 1e9);
+
     /* The scene: a marker the arrow keys move. The report below is its
-       position — the interactive frame makes itself observable. */
+       position and the time it moved — the interactive frame makes itself
+       observable. */
     Framebuffer *fb = GetFramebuffer();
-    int marker_x = (FRAME_WIDTH - MARKER_SIZE) / 2;
-    int marker_y = (FRAME_HEIGHT - MARKER_SIZE) / 2;
+    double marker_x = (FRAME_WIDTH - MARKER_SIZE) / 2.0;
+    double marker_y = (FRAME_HEIGHT - MARKER_SIZE) / 2.0;
+    double started = platform::Now();
+    double last = started;
     std::printf("engine: arrow keys move the marker; close the window to stop\n");
-    std::printf("engine: marker at %d,%d\n", marker_x, marker_y);
+    std::printf("engine: marker at %d,%d\n", (int)marker_x, (int)marker_y);
 
     /* The frame step: read news, update from polled state, draw, present.
        This is the shape every later part fills in — Part 2 draws into it,
@@ -64,16 +83,22 @@ int Run(void)
         if (platform::CloseRequested(opened.window))
             break;
 
-        /* Update: a frame reads state — it never handles events. */
-        int old_x = marker_x, old_y = marker_y;
+        /* Update: a frame reads state — it never handles events. The step
+           is speed × elapsed: the marker moves 240 pixels per second no
+           matter how often frames happen. */
+        double now = platform::Now();
+        double dt = now - last;
+        last = now;
+
+        int old_x = (int)marker_x, old_y = (int)marker_y;
         if (platform::KeyDown(opened.window, platform::KEY_LEFT))
-            marker_x -= MARKER_STEP;
+            marker_x -= MARKER_SPEED * dt;
         if (platform::KeyDown(opened.window, platform::KEY_RIGHT))
-            marker_x += MARKER_STEP;
+            marker_x += MARKER_SPEED * dt;
         if (platform::KeyDown(opened.window, platform::KEY_UP))
-            marker_y -= MARKER_STEP;
+            marker_y -= MARKER_SPEED * dt;
         if (platform::KeyDown(opened.window, platform::KEY_DOWN))
-            marker_y += MARKER_STEP;
+            marker_y += MARKER_SPEED * dt;
 
         /* The marker stays on screen — lesson 015's fold at frame scale. */
         if (marker_x < 0)
@@ -85,12 +110,13 @@ int Run(void)
         if (marker_y > FRAME_HEIGHT - MARKER_SIZE)
             marker_y = FRAME_HEIGHT - MARKER_SIZE;
 
-        if (marker_x != old_x || marker_y != old_y)
-            std::printf("engine: marker at %d,%d\n", marker_x, marker_y);
+        if ((int)marker_x != old_x || (int)marker_y != old_y)
+            std::printf("engine: marker at %d,%d (t=%.3f)\n", (int)marker_x,
+                        (int)marker_y, platform::Now() - started);
 
         /* Render: every frame draws the whole scene — clear, then marker. */
         ClearBuffer(*fb, 32, 32, 64);
-        DrawMarker(*fb, marker_x, marker_y);
+        DrawMarker(*fb, (int)marker_x, (int)marker_y);
 
         if (!platform::Present(opened.window, fb->pixels, fb->width,
                                fb->height)) {
diff --git a/src/platform.h b/src/platform.h
index 643bdef..cea5fc6 100644
--- a/src/platform.h
+++ b/src/platform.h
@@ -56,6 +56,12 @@ bool KeyPressed(Window *window, Key key);
    for them. */
 bool HasFocus(const Window *window);
 
+/* The platform clock: seconds since an arbitrary starting point. It is
+   monotonic — it never goes backwards, and the wall clock cannot move it —
+   and its resolution is fine enough to measure one frame. This is the
+   clock Part 2's frame timing and Part 5's frame-budget report stand on. */
+double Now(void);
+
 /* Reads whatever news the OS has about this window and folds it into the
    platform layer's state. The engine never sees an event object — it polls
    state afterwards. Blocks until there is news or the run is interrupted. */
diff --git a/src/platform_x11.cpp b/src/platform_x11.cpp
index 4e23966..d35cf08 100644
--- a/src/platform_x11.cpp
+++ b/src/platform_x11.cpp
@@ -7,6 +7,11 @@
 //
 // Lesson 028: the event pump. The OS's news arrives here as X events and is
 // folded into state the engine polls — the engine never reads an event.
+//
+// Lesson 035: the platform clock. POSIX, not ISO C — clock_gettime is the
+// OS's clock interface (the one lesson 020 taught inside snek, now behind
+// the seam), so the feature-test macro goes before the includes.
+#define _POSIX_C_SOURCE 200809L
 
 #include "platform.h"
 
@@ -15,9 +20,9 @@
 #include <X11/Xutil.h>
 #include <X11/keysym.h>
 
-#include <cstdio>
 #include <poll.h>
 #include <signal.h>
+#include <time.h>
 
 namespace platform {
 
@@ -200,6 +205,13 @@ bool HasFocus(const Window *window)
     return window && window->focused;
 }
 
+double Now(void)
+{
+    struct timespec ts;
+    clock_gettime(CLOCK_MONOTONIC, &ts);
+    return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
+}
+
 void PumpEvents(Window *window)
 {
     if (!window || !window->display)
```

## Exercices

Deux extensions « faites-le vôtre ». Chacune se termine par sa solution — un
diff contre l'état final de cette leçon, plus une visite guidée — après
l'énoncé.

### Exercice 1 — La diagonale est trop rapide *(fix-the-crash)*

Maintenez deux flèches et regardez le marqueur filer 41 % plus vite que sur
les axes : chaque axe ajoute `speed × dt` pour son propre compte. Corrigez
pour que le marqueur avance à `MARKER_SPEED` dans *toutes* les directions —
traitez les touches pressées comme un seul vecteur de direction et
normalisez-le avant d'appliquer la vitesse. Prouvez-le : maintenez une touche
pendant exactement une seconde, puis maintenez la diagonale pendant exactement
une seconde, et comparez les distances parcourues (un auxiliaire qui presse,
dort le temps exact, et relâche garde la comparaison honnête).

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-035/ex1.md)

### Exercice 2 — L'horloge qui ment *(explain-in-prose)*

L'exercice « deux horloges » de la leçon 020, revisité derrière la couture.
Ajoutez une seconde lecture instrumentée — l'horloge murale, `CLOCK_REALTIME`
— à côté de la monotone dans l'implémentation de la plateforme, et imprimez
les deux dans la vérification de démarrage. Expliquez ensuite, en prose, ce
que ferait le pas de frame `dt = now − last` si le moteur chronométrait ses
frames avec *cette* horloge : ce qui arrive au marqueur quand l'horloge
système recule, quand elle avance, et pourquoi l'horloge monotone est
immunisée contre les deux.

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-035/ex2.md)

---

**Partie :** [Partie 1 — la couche plateforme](../../index.md) ·
**Précédente :** [Leçon 034 — la première frame interactive](lesson-034-first-frame.md) ·
**Suivante :** [Leçon 036 — le temps de frame comme donnée mesurée](lesson-036-frame-time.md) ·
**Étiquette de code :** [`lesson-035`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-035)

*Page traduite de la version anglaise `book/lessons/part-1/lesson-035-clock.md`,
révision `9b3ec3d`.*

<!-- translation-source: book/lessons/part-1/lesson-035-clock.md @ 9b3ec3d -->
