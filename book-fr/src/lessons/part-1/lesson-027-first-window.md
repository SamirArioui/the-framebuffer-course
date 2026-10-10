# Leçon 027 — la couture plateforme et la première fenêtre X11

{{#include ../../stability-horizon.md}}

## Prose

Le moteur est né ; aujourd'hui il obtient une fenêtre. Non pas en allant la
chercher dans l'OS — en la demandant à la couche plateforme. Cette leçon
construit la **couture** : le seul endroit de la base de code où un OS a le
droit de parler, et l'interface exempte d'idiomes d'OS qui cache duquel il
s'agit. Sur cette ligne principale, l'implémentation est X11, le système de
fenêtres de Linux et de tout Unix qui mérite ce nom ; le moteur ne le saura
jamais.

### La couture

La promesse que fait la couche plateforme est ancienne — le jeu est construit
« contre l'OS derrière une couche plateforme que nous écrivons nous-mêmes » —
et c'est ici qu'elle est tenue. Deux fichiers aux rôles opposés apparaissent
aujourd'hui :

- **`src/platform.h`** — l'interface. Elle nomme des fonctions, des types
  simples, et un type énuméré pour les échecs. Elle n'inclut aucun en-tête
  d'OS et ne mentionne aucun type d'OS. Le code du moteur inclut ce fichier et
  aucune autre vue de l'OS n'existe pour lui.
- **`src/platform_x11.cpp`** — une implémentation. Chaque en-tête d'OS de la
  base de code est inclus ici, chaque type d'OS s'écrit ici, et les appels X11
  passent toute leur vie à l'intérieur de ce fichier.

Un second OS — disons un épilogue Win32 — signifie un second fichier
d'implémentation et une ligne de configuration de construction. Aucun fichier
du moteur ne change. Cette affirmation est vérifiable, pas incantatoire ;
l'exercice 2 la contrôle avec `nm`, et la leçon 042 audite la couture entière
contre exactement ce test.

Pourquoi un simple en-tête de fonctions et pas une classe d'interface avec une
vtable ? Parce que la leçon 025 vous a déjà appris à voir vers quoi se compile
une vtable, et cette couture n'en a pas besoin : il y a une seule
implémentation dans le programme à la fois, le choix se fait par le fichier
que la construction compile, et un appel de fonction est toute la répartition
qui existe jamais. La loi dit qu'une caractéristique est admise si son coût
est explicable — la répartition explicable la moins chère est aucune
répartition du tout.

### L'interface, symbole par symbole

```c++
namespace platform {

struct Window;   /* opaque */

enum OpenError {
    OPEN_OK = 0,
    OPEN_NO_DISPLAY,
    OPEN_NO_WINDOW,
};

struct WindowResult {
    Window *window;
    OpenError error;
};

WindowResult OpenWindow(int width, int height);
void CloseWindow(Window *window);

}
```

**`struct Window`** est déclaré et jamais défini dans l'en-tête — un *type
incomplet*. Le moteur peut garder un `Window *`, le transmettre de main en
main, le comparer à zéro ; il ne peut pas regarder dedans, parce qu'il n'y a
rien à voir de là où il est. Ce dont une fenêtre est faite est l'affaire de
l'implémentation. C'est la douleur du `void *` de la leçon 010, réglée comme
il faut : plutôt que d'effacer le type, nous l'avons déclaré en retenant sa
définition.

**`OpenError` et `WindowResult`** font de l'échec une valeur. `OpenWindow` soit
remet une fenêtre au moteur, soit nomme l'étape qui a échoué — le display de
l'OS n'a pas pu être ouvert (`OPEN_NO_DISPLAY`), ou l'OS a refusé de créer la
fenêtre (`OPEN_NO_WINDOW`). Il n'y a pas de troisième option où une fenêtre à
moitié ouverte serait présentée comme un succès ; les deux champs de
`WindowResult` s'accordent par construction (`error` vaut `OPEN_OK` exactement
quand `window` est non nul). La leçon 037 fera grandir la même forme pour les
fichiers : des octets complets ou un échec typé, jamais de données partielles
déguisées en succès.

**`OpenWindow` / `CloseWindow`** sont tout le cycle de vie de la fenêtre.
`OpenWindow` prend la taille que veut le moteur — *exactement* cette taille,
parce que les pixels du moteur seront disposés pour elle — et `CloseWindow`
libère tout ce que `OpenWindow` a pris à l'OS. Rien d'autre n'est exposé. Pas
d'affichage ni de masquage, pas de redimensionnement, pas de titre, pas
d'événements — non parce qu'ils ne viendront jamais, mais parce que l'interface
grandit un contrat à la fois et que chaque fonction qu'elle contient est une
promesse qu'un second OS devra tenir.

### Le côté OS : display et fenêtre

`platform_x11.cpp` est l'endroit où X11 parle enfin. Deux concepts de l'OS
mènent la danse :

- **Le display** — `XOpenDisplay` ouvre une *connexion* au serveur X qui
  possède l'écran. X11 est un système client/serveur : votre programme est un
  client, l'écran appartient à un processus serveur, et presque chaque appel X
  est en réalité une requête poussée à travers cette connexion.
  `XOpenDisplay(0)` se connecte au display nommé par l'environnement (sur un
  vrai bureau, il est simplement là ; sur une vérification sans écran, c'est
  `DISPLAY=:99` sous Xvfb).
- **La fenêtre** — `XCreateSimpleWindow` demande au serveur de créer un
  rectangle exactement de la taille demandée, avec une couleur de fond et pas
  de bordure. Ce qui revient est un **XID** : un entier que le serveur a
  inventé pour la fenêtre. La fenêtre est un état côté serveur ; notre
  programme détient son identifiant.

Ensuite deux requêtes de plus : `XMapWindow` rend la fenêtre visible (la créer
ne l'affiche pas), et `XFlush` pousse les requêtes en attente à travers la
connexion vers le serveur — les appels X s'accumulent sous forme de requêtes ;
rien ne se passe à l'écran tant qu'elles ne sont pas envoyées.

Un piège de nommage, désamorcé par les espaces de noms de la loi du langage :
X11 a lui aussi un type appelé `Window` (c'est le typedef du XID). À
l'intérieur de `namespace platform`, le type de l'OS s'écrit `::Window` —
celui de l'espace de noms global — et le nôtre est `platform::Window` :

```c++
struct Window {
    Display *display;
    ::Window xwindow;
};
```

La définition de `platform::Window` vit dans ce fichier, à côté de l'inclusion
de Xlib — la déclaration en avant de l'en-tête est tout ce que le moteur voit
jamais.

Et remarquez ce qui n'est *pas* dans ce fichier : pas de `new`, pas de
`delete`. Le seul état d'OS correspondant à une fenêtre siège dans le stockage
statique (`window_state`) et le moteur n'en détient jamais que le pointeur. La
loi du langage de la leçon 026 garde l'allocation hors du moteur, et elle
restera dehors : les grands tampons du moteur viendront de réservations de
mémoire au niveau de l'OS plus loin dans cette partie, pas d'un allocateur
qu'il faudrait désapprendre plus tard.

### Le côté moteur

`main.cpp` est du code de moteur et il en a l'air : il inclut `<cstdio>` et
`platform.h`, et il appelle `platform::OpenWindow(WINDOW_WIDTH,
WINDOW_HEIGHT)`. Si le `window` du résultat est zéro, il rapporte l'échec typé
et sort — les chemins d'erreur autour de cela grandissent dans la leçon 029.
Si une fenêtre est revenue, le moteur la rapporte, la garde ouverte, et la
ferme :

```
$ ./build.sh
build: compiling 2 source(s) from src/
  CC  src/main.cpp
  CC  src/platform_x11.cpp
  LD  build/game
build: OK (2 source(s) compiled -> build/game)
$ ./build/game
engine: window 640x480 open — press enter to close
```

Sur un vrai bureau, une fenêtre 640×480 intitulée *the framebuffer engine*
apparaît et attend. L'attente elle-même est un substitut — `std::getchar()`
bloquant sur le clavier — parce que ce qui *devrait* garder une fenêtre
ouverte est une pompe à événements, et c'est exactement ce que construit la
leçon 028. C'est un état temporaire délibéré : la couture est faite, la
vitalité est empruntée.

### La ligne de construction

La seule chose que `build.sh` a dû apprendre : la ligne de liaison porte
désormais la bibliothèque d'OS qu'enveloppe la couche plateforme, `-lX11`. Les
options du compilateur sont inchangées depuis la leçon 026 — la couture est
faite de C++ ordinaire ; c'est seulement la *liaison* qui a besoin du code de
Xlib. Une implémentation Win32 changerait ce seul défaut vers les bibliothèques
Win32, et rien d'autre de la construction.

### Vérifier la fenêtre sans écran

Le comportement de la fenêtre est observable sans bureau. Sur une machine sans
display (celle qui a servi à écrire ce livre en est une), Xvfb fournit un
écran virtuel et `xdotool` l'interroge comme le ferait un gestionnaire de
fenêtres :

```
$ Xvfb :99 -screen 0 800x600x24 &
$ (sleep 2; echo) | DISPLAY=:99 ./build/game &
engine: window 640x480 open — press enter to close
$ DISPLAY=:99 xdotool search --name "the framebuffer engine"
2097153
$ DISPLAY=:99 xdotool getwindowgeometry 2097153
Window 2097153
  Position: 0,0 (screen: 0)
  Geometry: 640x480
engine: closed
```

Une fenêtre exactement de la taille demandée par le moteur s'est ouverte, a
porté le titre que l'implémentation lui a donné, et s'est fermée proprement
quand la pompe de substitution l'a laissée partir. La machine est la même dans
les deux cas ; seul l'écran est virtuel.

## Étape de code

Un seul changement pour cette leçon : la couture naît — `platform.h` comme
interface exempte d'idiomes d'OS, `platform_x11.cpp` comme première
implémentation, `main.cpp` devenu du code de moteur qui ouvre sa fenêtre à
travers la couture, et la ligne de liaison de `build.sh` qui apprend l'unique
bibliothèque d'OS. Son état final est étiqueté `lesson-027`.

```diff
diff --git a/build.sh b/build.sh
index 2674470..f852f8c 100755
--- a/build.sh
+++ b/build.sh
@@ -13,7 +13,8 @@
 # Environment overrides:
 #   CC, CFLAGS       compiler and flags for C sources
 #   CXX, CXXFLAGS    compiler and flags for C++ sources
-#   LDFLAGS          extra link flags
+#   LDFLAGS          extra link flags (default: the OS library the platform
+#                    layer wraps — -lX11 on the Linux/X11 main line)
 #   BUILD_DIR        output directory (default: build)
 
 set -euo pipefail
@@ -24,7 +25,7 @@ CC="${CC:-gcc}"
 CXX="${CXX:-g++}"
 CFLAGS="${CFLAGS:--std=c11 -O0 -g -Wall -Wextra}"
 CXXFLAGS="${CXXFLAGS:--std=c++17 -O0 -g -Wall -Wextra}"
-LDFLAGS="${LDFLAGS:-}"
+LDFLAGS="${LDFLAGS:--lX11}"
 BUILD_DIR="${BUILD_DIR:-build}"
 OBJ_DIR="$BUILD_DIR/obj"
 BIN="$BUILD_DIR/game"
diff --git a/src/main.cpp b/src/main.cpp
index 7ecbde3..106b3b0 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -1,13 +1,45 @@
 // main.cpp — the engine, born.
 //
-// Lesson 026: the codebase is born from a blank file. This file obeys the
-// language law documented in the lesson: the admitted C++ subset of
-// lesson 025 — references, overloading, namespaces, constexpr, and classes
-// with vtables — and nothing else. `main` is the one global function the
-// runtime looks up; every engine symbol that follows lives in
-// `namespace engine`.
+// Lesson 027: the engine meets the OS through the platform seam. This file
+// includes no OS headers and names no OS type — it sees platform.h and
+// nothing else. The language law of lesson 026 holds: engine code lives in
+// namespace engine, main stays global, and every feature here is one the
+// law admits.
 
-int main(void)
+#include <cstdio>
+
+#include "platform.h"
+
+namespace engine {
+
+constexpr int WINDOW_WIDTH = 640;
+constexpr int WINDOW_HEIGHT = 480;
+
+int Run(void)
 {
+    platform::WindowResult opened =
+        platform::OpenWindow(WINDOW_WIDTH, WINDOW_HEIGHT);
+    if (!opened.window) {
+        std::fprintf(stderr, "engine: no window (platform error %d)\n",
+                     opened.error);
+        return 1;
+    }
+
+    std::printf("engine: window %dx%d open — press enter to close\n",
+                WINDOW_WIDTH, WINDOW_HEIGHT);
+
+    /* Stand-in for the event pump: hold the window open until enter.
+       Lesson 028 replaces exactly this. */
+    std::getchar();
+
+    platform::CloseWindow(opened.window);
+    std::printf("engine: closed\n");
     return 0;
 }
+
+} /* namespace engine */
+
+int main(void)
+{
+    return engine::Run();
+}
diff --git a/src/platform.h b/src/platform.h
new file mode 100644
index 0000000..361251c
--- /dev/null
+++ b/src/platform.h
@@ -0,0 +1,37 @@
+// platform.h — the platform layer's interface: the engine's only view of the OS.
+//
+// Lesson 027: the seam. Nothing in this header names an OS type — no display
+// connection, no window handle, no X11 anything — and it includes no OS
+// headers. A second OS implements the functions below in its own file, and
+// no engine file changes when it does (lesson 042 audits that promise).
+#ifndef PLATFORM_H
+#define PLATFORM_H
+
+namespace platform {
+
+/* What a window is made of is the OS implementation's business. The engine
+   holds pointers to it and never looks inside. */
+struct Window;
+
+/* Failure is a value: OpenWindow either hands the engine a window or names
+   the step that failed. No partial result is ever presented as success. */
+enum OpenError {
+    OPEN_OK = 0,
+    OPEN_NO_DISPLAY, /* the OS's display could not be opened */
+    OPEN_NO_WINDOW,  /* the OS refused to create the window */
+};
+
+struct WindowResult {
+    Window *window;  /* the window, or 0 on failure */
+    OpenError error; /* OPEN_OK exactly when window is non-0 */
+};
+
+/* Opens a window of exactly the requested size on the OS's display. */
+WindowResult OpenWindow(int width, int height);
+
+/* Releases everything OpenWindow took from the OS. */
+void CloseWindow(Window *window);
+
+} /* namespace platform */
+
+#endif
diff --git a/src/platform_x11.cpp b/src/platform_x11.cpp
new file mode 100644
index 0000000..203b181
--- /dev/null
+++ b/src/platform_x11.cpp
@@ -0,0 +1,70 @@
+// platform_x11.cpp — the X11 implementation of the platform layer.
+//
+// Lesson 027: the OS side of the seam. Every OS header in the codebase is
+// included here and nowhere else, and every OS type is spelled here and
+// nowhere else. An implementation for a second OS would live in its own
+// file beside this one and nothing outside would change.
+
+#include "platform.h"
+
+#include <X11/Xlib.h>
+
+namespace platform {
+
+/* What a window is made of on this OS. The definition lives here, where
+   Xlib is visible; the engine sees only the forward declaration. X11 has a
+   type called Window too — `::Window` is its, `platform::Window` is ours. */
+struct Window {
+    Display *display;
+    ::Window xwindow;
+};
+
+/* The OS state for one window, in static storage: no new, no delete — the
+   language law of lesson 026 keeps allocation out of the engine, and the
+   engine's big buffers will come from memory reservations later in this
+   part. The engine only ever holds the pointer. */
+static Window window_state;
+
+WindowResult OpenWindow(int width, int height)
+{
+    WindowResult result = { &window_state, OPEN_NO_DISPLAY };
+
+    Display *display = XOpenDisplay(0);
+    if (!display) {
+        result.window = 0;
+        return result;
+    }
+
+    int screen = DefaultScreen(display);
+    ::Window xwindow = XCreateSimpleWindow(display, RootWindow(display, screen),
+                                           0, 0, width, height, 0,
+                                           BlackPixel(display, screen),
+                                           WhitePixel(display, screen));
+    if (!xwindow) {
+        XCloseDisplay(display);
+        result.window = 0;
+        result.error = OPEN_NO_WINDOW;
+        return result;
+    }
+
+    XStoreName(display, xwindow, "the framebuffer engine");
+    XMapWindow(display, xwindow);
+    XFlush(display);
+
+    window_state.display = display;
+    window_state.xwindow = xwindow;
+    return result;
+}
+
+void CloseWindow(Window *window)
+{
+    if (!window || !window->display)
+        return;
+
+    XDestroyWindow(window->display, window->xwindow);
+    XCloseDisplay(window->display);
+    window->display = 0;
+    window->xwindow = 0;
+}
+
+} /* namespace platform */
```

## Exercices

Deux extensions « faites-les vôtres ». Chacune se termine par sa solution — un
diff contre l'état final de cette leçon, plus une visite guidée — après
l'énoncé.

### Exercice 1 — La fenêtre prend votre titre *(extend-the-code)*

La couture ne nomme aucun concept d'OS — mais un titre de fenêtre n'est pas un
concept d'OS, c'est juste du texte. Donnez au moteur une voix sur sa propre
fenêtre : ajoutez un titre à l'interface (un paramètre `const char *` sur
`OpenWindow`), transmettez-le à travers l'implémentation X11 jusqu'à l'OS, et
faites ouvrir au moteur sa fenêtre comme
`the framebuffer engine — lesson 027`. Montrez que ça marche : lancez le
moteur et trouvez votre fenêtre par son titre — sur votre bureau, ou sans
écran comme cette leçon l'a fait avec `xdotool search --name`. Combien de
fichiers le changement a-t-il touchés, et lequel d'entre eux a le droit de
savoir de quoi un titre est fait ?

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-027/ex1.md)

### Exercice 2 — Où se trouve la frontière *(explain-in-prose)*

La promesse de la couture est vérifiable : le code du moteur ne nomme jamais
un type d'OS. Vérifiez-le. Construisez l'état final de la leçon, puis compilez
`src/main.cpp` et `src/platform_x11.cpp` en objets chacun de leur côté
(`g++ -std=c++17 -Wall -Wextra -c`) et inspectez ce que chacun référence avec
`nm` (à travers `c++filt`). Avant de regarder, prédisez quel objet nomme
`XOpenDisplay` et lequel nomme `platform::OpenWindow`. Puis écrivez en prose ce
qu'un portage Win32 de ce programme toucherait — et ce qui, dans les deux
fichiers objet, le prouve. Rendez la frontière visible à l'exécution aussi,
avec une ligne d'instrumentation à l'intérieur de l'implémentation qui nomme
quel OS sert l'appel.

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-027/ex2.md)

---

**Partie :** [Partie 1 — la couche plateforme](../../index.md) ·
**Précédente :** [Leçon 026 — la base de code naît](lesson-026-birth.md) ·
**Suivante :** [Leçon 028 — la pompe à événements](lesson-028-event-pump.md) ·
**Étiquette de code :** [`lesson-027`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-027)

*Page traduite de la version anglaise `book/lessons/part-1/lesson-027-first-window.md`, révision `a8e1cba`.*

<!-- translation-source: book/lessons/part-1/lesson-027-first-window.md @ a8e1cba -->
