# Leçon 034 — la première frame interactive

{{#include ../../stability-horizon.md}}

## Prose

Trois leçons de machinerie se rejoignent aujourd'hui : l'entrée par scrutation,
un framebuffer qui appartient au moteur, et la présentation à travers la
couture. Le résultat est la plus petite chose complète que fasse un moteur —
**la frame interactive** : lire l'entrée, mettre le monde à jour, le dessiner,
le montrer. Un carré jaune, déplacé par les touches fléchées. Tout ce que la
partie 2 et au-delà feront à cette boucle est une expansion d'elle, jamais un
remplacement.

### L'étape de frame

La boucle est quatre mouvements avec une règle chacun :

```c++
while (!platform::CloseRequested(opened.window)) {
    platform::PumpEvents(opened.window);   /* news -> state */
    ...                                     /* react to close first */
    ...                                     /* update: read state */
    ...                                     /* render: draw the scene */
    platform::Present(...);                 /* show it */
}
```

- **Pump** plie les nouvelles dans l'état et rend la main. C'est le seul endroit
  où les événements existent (la règle de la leçon 028).
- **Update** lit de l'*état* — quatre appels à `KeyDown`, un par flèche — et
  déplace le marqueur. Une frame ne traite jamais d'événements ; elle pose des
  questions.
- **Render** dessine toute la scène dans le framebuffer : effacer, puis le
  marqueur. Chaque frame, toute la scène. Les redessins partiels sont une
  optimisation avec un coût de comptabilité, et la version honnête est un
  redessin complet de notre propre tampon — 307 200 pixels de `ClearBuffer` ne
  sont rien pour une machine et tout pour un lecteur.
- **Present** copie le tampon vers la fenêtre (l'XImage de la leçon 030, le
  contrat de la leçon 031).

L'update lit l'état, donc deux choses découlent de la forme au lieu d'être des
cas particuliers : une touche maintenue continue de déplacer le marqueur (l'état
reste enfoncé), et une frappe le déplace exactement d'autant que ses frames le
disent (la mémorisation de la leçon 033 est ce qui garantit qu'une frappe est
vue, tout simplement).

### Garder le marqueur honnête

Une pièce de logique de jeu vit dans l'update à côté du déplacement : le
marqueur reste à l'écran. Le bornage est le pliage de la leçon 015 à l'échelle
de la frame — la position est vérifiée après le déplacement, et le marqueur
s'arrête au bord au lieu de partir vers des coordonnées qui n'ont pas de pixels.
Chaque `PutPixel` de `DrawMarker` est *aussi* découpé (le pliage, encore), si
bien que les deux couches d'honnêteté ne dépendent pas l'une de l'autre : le
bornage concerne les règles du jeu, le découpage concerne la mémoire.

### À quelle vitesse va une frame ?

Voici la limite honnête de cette leçon : **les frames arrivent quand les
nouvelles arrivent**. La pompe bloque entre les lots, donc la boucle tourne
quand les événements tournent — et tant qu'une touche est maintenue, la nouvelle
qui fait venir les frames, c'est l'auto-repeat du clavier. Le pas du marqueur
appartient au moteur (`MARKER_STEP`, huit pixels) ; le *rythme* du marqueur
appartient encore au clavier.

Ce n'est pas une décision de conception à conserver ; c'est la frontière de ce
qui existe avant l'horloge. La leçon 035 donne au moteur une horloge monotone et
à la frame son `dt`, et à partir de là le pas vaut `speed × elapsed` — des
pixels par seconde, peu importe qui réveille la boucle. (L'alternative par
attente active — faire tourner les frames aussi vite que le CPU le permet — est
pire en tout point et meurt ici sans jamais naître.)

### Le marqueur, vérifié

Entrée scriptée, dix répétitions de la flèche droite espacées de cinquante
millisecondes :

```
$ DISPLAY=:99 ./build/game &
$ DISPLAY=:99 xdotool key --delay 50 --repeat 10 --window <id> Right
engine: arrow keys move the marker; close the window to stop
engine: marker at 308,228
engine: marker at 316,228
engine: marker at 324,228
...
engine: marker at 388,228
engine: close reported
engine: closed
```

Dix appuis, dix frames, dix pas de huit pixels — le rapport dit que le marqueur
est en `388,228`, et la fenêtre confirme : localiser la couleur du marqueur dans
les pixels présentés le trouve exactement là.

```
locate: marker pixels span 308,228 .. 330,250 (size 23x23)
locate: marker pixels span 388,228 .. 410,250 (size 23x23)
```

(Échantillonné un pixel sur deux, d'où 23 des 24 lignes et colonnes.) Les pixels
que le moteur a écrits sont les pixels que la fenêtre montre, à la position que
le moteur annonce — la boucle ne déplace pas juste une variable, elle déplace
une chose que vous pouvez voir.

## Étape de code

Un seul changement pour cette leçon : `main.cpp` gagne la frame interactive —
un marqueur déplacé par les flèches scrutées, borné à l'écran, redessiné en
entier à chaque frame et présenté — et le motif de test à usage unique de la
leçon 030 est remplacé par la boucle sur laquelle chaque partie ultérieure se
construira. Son état final est étiqueté `lesson-034`.

```diff
diff --git a/src/main.cpp b/src/main.cpp
index 0d0d64b..47d7360 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -12,10 +12,18 @@
 
 namespace engine {
 
-/* The seam's keys, by name — for the report below. */
-static const char *const key_names[platform::KEY_COUNT] = {
-    "up", "down", "left", "right", "space", "enter", "escape",
-};
+/* The marker: one square the arrow keys move. Its position is whole
+   pixels and its speed is pixels-per-frame — lesson 035's clock turns
+   that into pixels-per-second. */
+constexpr int MARKER_SIZE = 24;
+constexpr int MARKER_STEP = 8;
+
+static void DrawMarker(Framebuffer &fb, int x, int y)
+{
+    for (int j = 0; j < MARKER_SIZE; ++j)
+        for (int i = 0; i < MARKER_SIZE; ++i)
+            PutPixel(fb, x + i, y + j, 240, 220, 80);
+}
 
 int Run(void)
 {
@@ -39,69 +47,50 @@ int Run(void)
         return 1;
     }
 
-    /* Paint: clear, then pixels — the two instincts Part 0's paint taught,
-       now onto the engine's own buffer. */
+    /* The scene: a marker the arrow keys move. The report below is its
+       position — the interactive frame makes itself observable. */
     Framebuffer *fb = GetFramebuffer();
-    ClearBuffer(*fb, 32, 32, 64);
-    PutPixel(*fb, 0, 0, 255, 0, 0);
-    PutPixel(*fb, 639, 479, 0, 255, 0);
-    PutPixel(*fb, 700, 100, 0, 0, 255); /* out of bounds: dropped */
-
-    /* The byte-level report: what is actually in the buffer. */
-    unsigned char r, g, b;
-    std::printf("engine: framebuffer %dx%d, %d bytes, stride %d\n",
-                fb->width, fb->height, fb->width * fb->height * 4,
-                fb->width * 4);
-    GetPixel(*fb, 0, 0, r, g, b);
-    std::printf("engine: pixel (0,0) = %d %d %d\n", r, g, b);
-    GetPixel(*fb, 639, 479, r, g, b);
-    std::printf("engine: pixel (639,479) = %d %d %d\n", r, g, b);
-    GetPixel(*fb, 60, 101, r, g, b);
-    std::printf("engine: pixel (60,101) = %d %d %d\n", r, g, b);
-
-    /* First light: the bytes go to the window through the seam. */
-    if (!platform::Present(opened.window, fb->pixels, fb->width, fb->height)) {
-        std::fprintf(stderr, "engine: presentation failed\n");
-        platform::CloseWindow(opened.window);
-        return 1;
-    }
-    std::printf("engine: presented\n");
-
-    /* The frame step: read news, react, present our pixels, repeat.
-       Presentation is not an event — it is the engine's answer to every
-       event: the pixels the engine wrote are the pixels the window shows,
-       and re-presenting is what repairs the window when the OS damaged it.
-       React first: if the news was "the window is gone", there is nothing
-       left to present to. */
+    int marker_x = (FRAME_WIDTH - MARKER_SIZE) / 2;
+    int marker_y = (FRAME_HEIGHT - MARKER_SIZE) / 2;
+    std::printf("engine: arrow keys move the marker; close the window to stop\n");
+    std::printf("engine: marker at %d,%d\n", marker_x, marker_y);
+
+    /* The frame step: read news, update from polled state, draw, present.
+       This is the shape every later part fills in — Part 2 draws into it,
+       Part 5 measures it. */
     int exit_code = 0;
-    bool had_focus = platform::HasFocus(opened.window);
     while (!platform::CloseRequested(opened.window)) {
         platform::PumpEvents(opened.window);
         if (platform::CloseRequested(opened.window))
             break;
 
-        /* The polled state: what is down right now, as of this poll. */
-        std::printf("engine: polled:");
-        bool any = false;
-        for (int k = 0; k < platform::KEY_COUNT; ++k) {
-            if (platform::KeyDown(opened.window, (platform::Key)k)) {
-                std::printf(" %s", key_names[k]);
-                any = true;
-            }
-        }
-        std::printf(any ? "\n" : " -\n");
-
-        /* The latches: presses that ended before this poll are not lost. */
-        for (int k = 0; k < platform::KEY_COUNT; ++k)
-            if (platform::KeyPressed(opened.window, (platform::Key)k))
-                std::printf("engine: pressed %s\n", key_names[k]);
-
-        /* Focus: reported when it changes. */
-        bool focus = platform::HasFocus(opened.window);
-        if (focus != had_focus) {
-            std::printf("engine: focus %s\n", focus ? "gained" : "lost");
-            had_focus = focus;
-        }
+        /* Update: a frame reads state — it never handles events. */
+        int old_x = marker_x, old_y = marker_y;
+        if (platform::KeyDown(opened.window, platform::KEY_LEFT))
+            marker_x -= MARKER_STEP;
+        if (platform::KeyDown(opened.window, platform::KEY_RIGHT))
+            marker_x += MARKER_STEP;
+        if (platform::KeyDown(opened.window, platform::KEY_UP))
+            marker_y -= MARKER_STEP;
+        if (platform::KeyDown(opened.window, platform::KEY_DOWN))
+            marker_y += MARKER_STEP;
+
+        /* The marker stays on screen — lesson 015's fold at frame scale. */
+        if (marker_x < 0)
+            marker_x = 0;
+        if (marker_x > FRAME_WIDTH - MARKER_SIZE)
+            marker_x = FRAME_WIDTH - MARKER_SIZE;
+        if (marker_y < 0)
+            marker_y = 0;
+        if (marker_y > FRAME_HEIGHT - MARKER_SIZE)
+            marker_y = FRAME_HEIGHT - MARKER_SIZE;
+
+        if (marker_x != old_x || marker_y != old_y)
+            std::printf("engine: marker at %d,%d\n", marker_x, marker_y);
+
+        /* Render: every frame draws the whole scene — clear, then marker. */
+        ClearBuffer(*fb, 32, 32, 64);
+        DrawMarker(*fb, marker_x, marker_y);
 
         if (!platform::Present(opened.window, fb->pixels, fb->width,
                                fb->height)) {
```

## Exercices

Deux extensions « faites-les vôtres ». Chacune se termine par sa solution — un
diff contre l'état final de cette leçon, plus une visite guidée — après
l'énoncé.

### Exercice 1 — Huit directions *(extend-the-code)*

Quatre `if` donnent quatre directions ; un marqueur en veut huit. Reprenez
l'update pour que les deux axes soient lus indépendamment — un seul pas selon
la direction sommée — et montrez la diagonale fonctionner avec une combinaison
de deux flèches maintenues (`xdotool key --delay 50 --repeat 5 --window <id>
Right+Down`). Tant que vous y êtes : que fait le marqueur quand gauche *et*
droite sont enfoncées en même temps, et pourquoi est-ce la bonne réponse ?

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-034/ex1.md)

### Exercice 2 — La vitesse qui appartient au clavier *(port-to-your-own-machine)*

Le marqueur bouge quand une touche est maintenue — mais *à quelle vitesse*, ce
n'est pas encore le moteur qui répond. Rendez la frame dénombrable : une ligne
d'instrumentation qui rapporte le numéro de frame à côté de chaque déplacement.
Maintenez ensuite une touche fléchée une seconde sur votre propre machine et
comptez : combien de pas, et combien de frames ? Expliquez ce que les nombres
disent sur qui possède actuellement la vitesse du marqueur — et ce que la leçon
035 devra donner au moteur pour que cela change.

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-034/ex2.md)

---

**Partie :** [Partie 1 — la couche plateforme](../../index.md) ·
**Précédente :** [Leçon 033 — mémoriser les appuis brefs et suivre le focus](lesson-033-latching.md) ·
**Suivante :** [Leçon 035 — l'horloge de la plateforme](lesson-035-clock.md) ·
**Étiquette de code :** [`lesson-034`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-034)

*Page traduite de la version anglaise `book/lessons/part-1/lesson-034-first-frame.md`, révision `54451b2`.*

<!-- translation-source: book/lessons/part-1/lesson-034-first-frame.md @ 54451b2 -->
