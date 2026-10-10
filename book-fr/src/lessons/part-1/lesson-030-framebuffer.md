# Leçon 030 — le framebuffer comme nos propres octets

{{#include ../../stability-horizon.md}}

## Prose

La fenêtre est vivante mais elle ne montre rien de nous — chaque pixel dessus
vient de l'OS. Cela cesse aujourd'hui. Le moteur reçoit un **framebuffer** :
un écran de mémoire qui nous appartient, disposé selon un contrat que nous
définissons, écrit par du code que nous possédons. C'est le tampon `paint` de
la Partie 0 agrandi à la taille de la fenêtre — les mêmes instincts
(`ClearBuffer`, `PutPixel`, `GetPixel`), la même arithmétique d'offsets — et à
la fin de la leçon la fenêtre montre ces octets pour la première fois.

### Le tampon

```c++
struct Framebuffer {
    unsigned char *pixels;
    int width;
    int height;
};
```

Un écran d'octets et deux nombres. `FRAME_WIDTH`×`FRAME_HEIGHT` font 640×480 —
la taille de la fenêtre de la leçon 027 — et à quatre octets par pixel cela
fait `1228800` octets de pixels : la plus grosse chose que le moteur possède à
ce jour, et la raison pour laquelle la plongée dans la mémoire virtuelle de la
leçon 039 arrive.

Où vivent 1,2 Mo de pixels ? Dans le stockage statique, pour l'instant : un
tableau à l'intérieur de `framebuffer.cpp`, rendu par `GetFramebuffer`. La loi
du langage de la leçon 026 tient `new`/`delete` hors du moteur, donc le tampon
est simplement *là* — et c'est le fil que la leçon 040 tire : un tampon de
cette taille devrait venir d'une *réservation* mémoire au niveau de l'OS, pas
de l'allocateur de qui que ce soit, et le framebuffer sera le premier à
déménager.

### Le format est celui de la couture

Quatre octets par pixel, dans cet ordre en mémoire : **bleu, vert, rouge, un
octet inutilisé**. C'est le pixel compacté de la leçon 013 — mais remarquez
*de qui* est ce contrat. Ce n'est pas le format de X11, et ce n'est pas un
fait matériel avec lequel le moteur doit vivre ; c'est le format de la
**couture plateforme**, déclaré dans `platform.h` à côté de `Present` :

> width × height pixels de 4 octets chacun (bleu, vert, rouge, un octet
> inutilisé), une ligne après l'autre.

Toute implémentation d'OS derrière la couture doit porter ces octets jusqu'à
sa fenêtre. Sur la ligne principale Linux/X11, cela se fait gratuitement — cet
ordre d'octets est ce que X11 transporte nativement sur x86-64 en petit
boutisme — et un épilogue Win32 traduirait en sortant. Le code du moteur écrit
des couleurs à travers `PutPixel` et ne raisonne plus jamais sur l'ordre des
octets.

L'arithmétique est celle des leçons 013 et 007, une fois pour toutes :

```
pixel (x, y) starts at (y * width + x) * 4
one row is width * 4 bytes — the stride
```

À 640 de large, le pas fait 2560 octets : un nombre entier de mots de
8 octets, donc aucune frontière de ligne ne chevauche jamais un mot machine.
L'octet inutilisé est le pas le moins coûteux qui existe.

### Écrire des pixels

`PutPixel` est celui de paint, avec une différence qui vaut toute la fonction :
il **découpe**. Une écriture hors limites est abandonnée — le pliage de la
leçon 015 — jamais laissée déborder dans les octets d'un autre pixel. La
preuve est arithmétique : une écriture à `(700, 100)` sans la découpe
atterrirait à l'index de pixel `100 * 640 + 700 = 64700` — ligne 101,
colonne 60 — corrompant un pixel qui n'a rien à voir avec la requête. Avec le
pliage, elle ne change rien, et l'auto-vérification peut *montrer* le pixel
intact.

`ClearBuffer` remplit tout le tampon d'une seule couleur et `GetPixel` relit
un pixel — les mêmes octets que `PutPixel` a écrits. Ensemble, ils rendent le
tampon vérifiable sans aucune fenêtre.

### L'XImage qui peut les porter

Les octets sont à nous ; les amener jusqu'à la fenêtre est le travail de la
couche plateforme, et son outil est l'`XImage` de X11. Un `XImage` n'est pas
une image — c'est une *vue* : la description d'un tampon de pixels (géométrie,
profondeur, disposition des octets) sur laquelle Xlib peut agir.
`XCreateImage` **enveloppe nos octets sans les copier** ; le pointeur `pixels`
du moteur devient les données de l'image, et la structure d'image n'est que la
description autour.

Deux choses se produisent dans `Present`, et une seule coûte :

1. `XCreateImage` enveloppe le tampon — pas de copie, juste une description.
2. `XPutImage` **copie** les pixels vers le serveur — c'est la copie dont le
   coût est réel, et la leçon 036 le mesurera honnêtement. (La mémoire
   partagée — MIT-SHM — peut éviter cette copie ; c'est une optimisation
   nommée pour plus tard, pas un sujet de la Partie 1.)

Puis la structure d'image est détruite — et notez *comment* : `image->data`
est d'abord mis à zéro. La structure est à nous, à libérer ; les octets en
dessous sont au moteur. La règle de propriété de la leçon 004 dans sa plus
petite forme : vous libérez ce que vous avez pris, et vous n'avez pris que la
structure.

### Première lumière

Le moteur peint, puis fait un `Present` : effacement vers un bleu-gris sombre,
un pixel rouge à l'origine, du vert au coin opposé, et une écriture hors
limites que le pliage abandonne. Le rapport relit le tampon :

```
$ ./build.sh
build: compiling 3 source(s) from src/
  CC  src/framebuffer.cpp
  CC  src/main.cpp
  CC  src/platform_x11.cpp
  LD  build/game
build: OK (3 source(s) compiled -> build/game)
$ DISPLAY=:99 ./build/game
engine: framebuffer 640x480, 1228800 bytes, stride 2560
engine: pixel (0,0) = 255 0 0
engine: pixel (639,479) = 0 255 0
engine: pixel (60,101) = 32 32 64
engine: presented
engine: close reported
engine: closed
```

`(0,0)` et `(639,479)` sont les deux pixels que nous avons écrits. `(60,101)`
est la preuve du découpage : il lit toujours le fond `32 32 64` — l'écriture à
`(700, 100)` qui aurait atterri là a été abandonnée au pliage.

Et la fenêtre — sur un vrai bureau, vous pouvez la voir — montre exactement
ces pixels : bleu-gris sombre, un point rouge dans le coin supérieur gauche,
un point vert dans le coin inférieur droit. « Les pixels que le moteur a
écrits sont les pixels que la fenêtre montre » est maintenant vrai en fait ;
la leçon 031 en fait un contrat et le prouve en relisant la fenêtre.

## Étape de code

Un seul changement pour cette leçon : le framebuffer naît — `framebuffer.h` et
`framebuffer.cpp` avec les octets, la disposition, et les écritures découpées —
la couture s'enrichit de `Present`, dont l'`XImage` enveloppe les octets du
moteur et dont `XPutImage` les copie vers la fenêtre, et `main.cpp` peint,
vérifie, puis fait un `Present` de sa première frame. Son état final est
étiqueté `lesson-030`.

```diff
diff --git a/src/framebuffer.cpp b/src/framebuffer.cpp
new file mode 100644
index 0000000..7e6fe13
--- /dev/null
+++ b/src/framebuffer.cpp
@@ -0,0 +1,59 @@
+// framebuffer.cpp — the engine's pixels, by hand.
+//
+// Lesson 030: the same arithmetic Part 0's paint did, on a buffer sized for
+// the window. Offset math, byte order, clipping — nothing else.
+
+#include "framebuffer.h"
+
+namespace engine {
+
+/* One screenful of pixels, in static storage. */
+static unsigned char pixels[FRAME_WIDTH * FRAME_HEIGHT * 4];
+static Framebuffer framebuffer = { pixels, FRAME_WIDTH, FRAME_HEIGHT };
+
+Framebuffer *GetFramebuffer(void)
+{
+    return &framebuffer;
+}
+
+void ClearBuffer(Framebuffer &fb, unsigned char r, unsigned char g,
+                 unsigned char b)
+{
+    int count = fb.width * fb.height;
+    for (int i = 0; i < count; ++i) {
+        unsigned char *p = fb.pixels + i * 4;
+        p[0] = b;
+        p[1] = g;
+        p[2] = r;
+        p[3] = 0;
+    }
+}
+
+void PutPixel(Framebuffer &fb, int x, int y, unsigned char r,
+              unsigned char g, unsigned char b)
+{
+    if (x < 0 || x >= fb.width || y < 0 || y >= fb.height)
+        return; /* dropped, not wrapped (lesson 015's fold) */
+
+    unsigned char *p = fb.pixels + (y * fb.width + x) * 4;
+    p[0] = b;
+    p[1] = g;
+    p[2] = r;
+    p[3] = 0;
+}
+
+void GetPixel(const Framebuffer &fb, int x, int y, unsigned char &r,
+              unsigned char &g, unsigned char &b)
+{
+    if (x < 0 || x >= fb.width || y < 0 || y >= fb.height) {
+        r = g = b = 0;
+        return;
+    }
+
+    const unsigned char *p = fb.pixels + (y * fb.width + x) * 4;
+    b = p[0];
+    g = p[1];
+    r = p[2];
+}
+
+} /* namespace engine */
diff --git a/src/framebuffer.h b/src/framebuffer.h
new file mode 100644
index 0000000..1f38723
--- /dev/null
+++ b/src/framebuffer.h
@@ -0,0 +1,47 @@
+// framebuffer.h — the engine's pixels: our own bytes, our own layout.
+//
+// Lesson 030: the framebuffer is memory the engine owns — Part 0's paint
+// buffer, grown to window size. The pixel format is the platform seam's
+// contract, not any OS's: platform.h's Present carries these exact bytes.
+#ifndef FRAMEBUFFER_H
+#define FRAMEBUFFER_H
+
+namespace engine {
+
+/* The size the window is opened at (lesson 027) and the size of the
+   framebuffer behind it: one format, one geometry. */
+constexpr int FRAME_WIDTH = 640;
+constexpr int FRAME_HEIGHT = 480;
+
+/* 32 bits per pixel in memory: blue, green, red, one unused byte — the
+   packed pixel of lesson 013, four bytes for the alignment lesson 007
+   explained. Rows run top to bottom, one pixel after another:
+   the pixel at (x, y) starts at (y * width + x) * 4. */
+struct Framebuffer {
+    unsigned char *pixels;
+    int width;
+    int height;
+};
+
+/* The engine's framebuffer. Its bytes live in static storage — the
+   language law of lesson 026 keeps allocation out of the engine, and
+   lesson 040 gives buffers like this a real home. */
+Framebuffer *GetFramebuffer(void);
+
+/* Fills every pixel with one color. */
+void ClearBuffer(Framebuffer &fb, unsigned char r, unsigned char g,
+                 unsigned char b);
+
+/* Writes one pixel. Out-of-bounds writes are dropped — the fold of
+   lesson 015, never a wrap into someone else's memory. */
+void PutPixel(Framebuffer &fb, int x, int y, unsigned char r,
+              unsigned char g, unsigned char b);
+
+/* Reads one pixel back — the same bytes PutPixel wrote. Out of bounds, the
+   result is black. */
+void GetPixel(const Framebuffer &fb, int x, int y, unsigned char &r,
+              unsigned char &g, unsigned char &b);
+
+} /* namespace engine */
+
+#endif
diff --git a/src/main.cpp b/src/main.cpp
index efb0b34..3245656 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -1,23 +1,21 @@
 // main.cpp — the engine, born.
 //
-// Lesson 028: the engine stays alive by reading the OS's news through the
-// seam. Still no OS headers, still no OS types — the pump is one more
-// platform function and the engine polls what it leaves behind. The
-// language law of lesson 026 holds.
+// Lesson 030: the engine writes its own pixels. The framebuffer is our
+// bytes — Part 0's paint intuition at window size — and Present carries
+// them through the seam. Still no OS headers here; the language law of
+// lesson 026 holds.
 
 #include <cstdio>
 
+#include "framebuffer.h"
 #include "platform.h"
 
 namespace engine {
 
-constexpr int WINDOW_WIDTH = 640;
-constexpr int WINDOW_HEIGHT = 480;
-
 int Run(void)
 {
     platform::WindowResult opened =
-        platform::OpenWindow(WINDOW_WIDTH, WINDOW_HEIGHT);
+        platform::OpenWindow(FRAME_WIDTH, FRAME_HEIGHT);
     if (!opened.window) {
         /* The error path: nothing was taken that the platform layer did not
            put back, and the failure is reported by name. */
@@ -36,11 +34,30 @@ int Run(void)
         return 1;
     }
 
-    std::printf("engine: window %dx%d open — waiting for news\n",
-                WINDOW_WIDTH, WINDOW_HEIGHT);
+    /* Paint: clear, then pixels — the two instincts Part 0's paint taught,
+       now onto the engine's own buffer. */
+    Framebuffer *fb = GetFramebuffer();
+    ClearBuffer(*fb, 32, 32, 64);
+    PutPixel(*fb, 0, 0, 255, 0, 0);
+    PutPixel(*fb, 639, 479, 0, 255, 0);
+    PutPixel(*fb, 700, 100, 0, 0, 255); /* out of bounds: dropped */
+
+    /* The byte-level report: what is actually in the buffer. */
+    unsigned char r, g, b;
+    std::printf("engine: framebuffer %dx%d, %d bytes, stride %d\n",
+                fb->width, fb->height, fb->width * fb->height * 4,
+                fb->width * 4);
+    GetPixel(*fb, 0, 0, r, g, b);
+    std::printf("engine: pixel (0,0) = %d %d %d\n", r, g, b);
+    GetPixel(*fb, 639, 479, r, g, b);
+    std::printf("engine: pixel (639,479) = %d %d %d\n", r, g, b);
+    GetPixel(*fb, 60, 101, r, g, b);
+    std::printf("engine: pixel (60,101) = %d %d %d\n", r, g, b);
+
+    /* First light: the bytes go to the window through the seam. */
+    platform::Present(opened.window, fb->pixels, fb->width, fb->height);
+    std::printf("engine: presented\n");
 
-    /* The event pump: read news, fold it into state, react to state,
-       repeat. This loop is what keeps the window alive. */
     while (!platform::CloseRequested(opened.window))
         platform::PumpEvents(opened.window);
 
diff --git a/src/platform.h b/src/platform.h
index da3e0f1..a2e0298 100644
--- a/src/platform.h
+++ b/src/platform.h
@@ -39,6 +39,13 @@ void PumpEvents(Window *window);
    one ending to get right. */
 bool CloseRequested(const Window *window);
 
+/* Presents the engine's framebuffer in the window: the pixels the engine
+   wrote are the pixels the window shows. The format is this interface's
+   contract, not any OS's — width * height pixels of 4 bytes each (blue,
+   green, red, one unused byte), one row after another. */
+void Present(Window *window, const unsigned char *pixels, int width,
+             int height);
+
 /* Releases everything OpenWindow took from the OS. */
 void CloseWindow(Window *window);
 
diff --git a/src/platform_x11.cpp b/src/platform_x11.cpp
index 0438d3f..8c93ccc 100644
--- a/src/platform_x11.cpp
+++ b/src/platform_x11.cpp
@@ -11,6 +11,7 @@
 #include "platform.h"
 
 #include <X11/Xlib.h>
+#include <X11/Xutil.h>
 
 #include <poll.h>
 #include <signal.h>
@@ -132,6 +133,39 @@ bool CloseRequested(const Window *window)
     return window && window->close_requested;
 }
 
+void Present(Window *window, const unsigned char *pixels, int width,
+             int height)
+{
+    if (!window || !window->display)
+        return;
+
+    int screen = DefaultScreen(window->display);
+
+    /* An XImage is a *view*: XCreateImage wraps the engine's bytes without
+       copying them — the description of a pixel buffer, not the buffer.
+       It carries our bytes exactly as the seam's contract defines them. */
+    XImage *image = XCreateImage(window->display,
+                                 DefaultVisual(window->display, screen),
+                                 DefaultDepth(window->display, screen),
+                                 ZPixmap, 0, (char *)pixels,
+                                 width, height, 32, width * 4);
+    if (!image)
+        return;
+
+    /* XPutImage is where the copy happens — our bytes to the server. Its
+       cost is real; lesson 036 measures it. */
+    XPutImage(window->display, window->xwindow,
+              DefaultGC(window->display, screen),
+              image, 0, 0, 0, 0, width, height);
+
+    /* The image struct is ours to destroy; the bytes under it are the
+       engine's, so they are detached before destruction (the same rule as
+       lesson 004's ownership drills). */
+    image->data = 0;
+    XDestroyImage(image);
+    XFlush(window->display);
+}
+
 void CloseWindow(Window *window)
 {
     if (!window || !window->display)
```

## Exercices

Deux extensions « faites-le vôtre ». Chacune se termine par sa solution — un
diff contre l'état final de cette leçon, plus une visite guidée — après
l'énoncé.

### Exercice 1 — FillRect, de retour de la Partie 0 *(extend-the-code)*

Le `FillRect` de paint a sa place ici autant que `PutPixel`. Faites grandir le
module framebuffer d'un remplissage de rectangle découpé — et placez la
découpe là où le pliage de la leçon 015 dit qu'elle appartient : dans l'espace
du rectangle, avant les boucles de pixels. Utilisez-le pour peindre trois
blocs : un entièrement à l'intérieur du tampon, un qui traverse le bord
gauche, un qui traverse les coins droit et bas. Relisez des pixels pour
montrer quelles parties de chaque bloc ont atterri — et regardez ce que votre
troisième bloc fait au pixel vert en (639, 479).

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-030/ex1.md)

### Exercice 2 — Les quatre octets *(explain-in-prose)*

Le rapport lit les pixels à travers `GetPixel`, qui retraduit les octets en
couleurs. Rendez la machine visible à la place : deux lignes d'instrumentation
qui dumpent les quatre octets bruts au début du pixel (0,0) et les quatre
octets bruts au début du pixel (1,0) — comme la leçon 013 dumpait les fichiers
d'image. Prédisez les deux dumps avant de lancer (le pixel (0,0) est rouge ;
(1,0) est le fond). Puis expliquez, en prose, pourquoi l'ordre en mémoire est
bleu-vert-rouge-x et non rouge-vert-bleu — ce que le système de fenêtrage a à
voir là-dedans, ce que l'octet inutilisé achète (l'arithmétique de la
leçon 007 à l'échelle du tampon), et où vit dans le code le contrat qui le
définit.

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-030/ex2.md)

---

**Partie :** [Partie 1 — la couche plateforme](../../index.md) ·
**Précédente :** [Leçon 029 — fermeture propre et chemins d'erreur : les ressources de l'OS libérées à chaque sortie](lesson-029-clean-close.md) ·
**Suivante :** [Leçon 031 — la présentation à travers la couche plateforme](lesson-031-present.md) ·
**Étiquette de code :** [`lesson-030`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-030)

*Page traduite de la version anglaise `book/lessons/part-1/lesson-030-framebuffer.md`, révision `da55ef9`.*

<!-- translation-source: book/lessons/part-1/lesson-030-framebuffer.md @ da55ef9 -->
