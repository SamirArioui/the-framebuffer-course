# Leçon 028 — la pompe à événements : garder la fenêtre en vie et signaler la fermeture

{{#include ../../stability-horizon.md}}

## Prose

La fenêtre de la leçon 027 avait un pouls emprunté : `std::getchar()` la
gardait ouverte et l'OS n'entendait plus jamais parler du programme. Ce n'est
pas ainsi qu'une fenêtre reste en vie. La connexion à l'OS est
bidirectionnelle — elle rapporte des nouvelles *au* programme : l'utilisateur
a bougé quelque chose, le gestionnaire de fenêtres veut quelque chose, la
fenêtre est morte. Un programme qui ne lit jamais ces nouvelles cesse de
répondre, et une fenêtre dont le propriétaire cesse de répondre est exactement
ce qu'un OS appelle « ne répond pas ». Aujourd'hui le moteur apprend à
écouter : la **pompe à événements**, et la première nouvelle qu'il doit
prendre au sérieux — la *fermeture*.

### La couture gagne deux fonctions

L'interface plateforme gagne la pompe et une interrogation :

```c++
void PumpEvents(Window *window);
bool CloseRequested(const Window *window);
```

Regardez la division du travail que le couple encode, parce que c'est la forme
que tout ce qui suit reproduit : **les nouvelles entrent dans l'état ; le
moteur lit l'état**. `PumpEvents` est la seule fonction qui voie jamais un
objet événement — et elle les voit *à l'intérieur de la couche plateforme*,
plie chacun d'eux dans de simples champs, puis jette l'événement. Le moteur
appelle la pompe puis scrute `CloseRequested`, comme un conducteur surveille
la route plutôt que d'attraper des papillons. La saisie de la leçon 032
fonctionnera de la même façon : les événements clavier de l'OS seront pliés
dans un état de touches interrogeable, et le moteur le scrutera — jamais de
consommation d'un flux d'événements.

### Le côté OS : attendre, pas tourner à vide

La moitié X11 de la pompe, ce sont trois appels qui font un seul travail :

```c++
XEvent event;
XNextEvent(window->display, &event);   /* blocks until there is news */
HandleEvent(window, event);
while (XPending(window->display)) {    /* what else piled up? */
    XNextEvent(window->display, &event);
    HandleEvent(window, event);
}
```

`XNextEvent` **bloque** : si aucune nouvelle n'est arrivée, le programme dort
à l'intérieur de l'OS jusqu'à ce qu'il en arrive une — aucun CPU n'est brûlé à
attendre. Ensuite `XPending` indique combien d'événements sont déjà dans la
file, et la boucle les vide tous avant de revenir au moteur. Une lecture
bloquante plus une vidange, c'est la pompe standard : le moteur se réveille
quand il y a quelque chose à savoir, traite tout le lot, et se rendort.

Le blocage est le bon comportement *pour le moteur de cette leçon*, qui ne
fait rien entre deux nouvelles. Il cesse d'être le bon au moment où le moteur
doit une frame à l'écran à chaque tick — la frame interactive de la leçon 034
— et c'est là que la pompe devient non bloquante à dessein. Pour l'instant,
attendre est honnête.

### La conversation de fermeture

« Fermer la fenêtre » ressemble à un seul événement. Sur X11, c'est un petit
protocole à deux issues :

- **La demande polie.** Un gestionnaire de fenêtres ne détruit pas une fenêtre
  dans le dos de son propriétaire. Il *demande*. La demande est un
  `ClientMessage` — un événement ordinaire portant deux atomes — et l'atome
  qui signifie « veuillez fermer » est `WM_DELETE_WINDOW`. Les atomes sont des
  noms que le serveur X distribue sous forme d'entiers : `XInternAtom`
  transforme la chaîne `"WM_DELETE_WINDOW"` en son numéro, et `XSetWMProtocols`
  déclare au gestionnaire de fenêtres que ce programme accepte la demande. Le
  bouton de fermeture de votre bureau produit exactement ce message.
- **Le passage à l'acte.** La fenêtre peut aussi simplement cesser d'exister —
  détruite directement (`xdotool windowclose` fait cela), ou démantelée par le
  serveur. La nouvelle est `DestroyNotify`, et ce n'est pas une demande : la
  ressource d'OS a déjà disparu.

Les deux issues se plient dans un seul et même état, parce que la réaction du
moteur est la même : s'arrêter. Mais le pliage se souvient de la différence là
où elle compte — après `DestroyNotify`, l'identifiant de la fenêtre est mort,
et le nettoyage ne doit pas demander à l'OS de la détruire une seconde fois
(c'est une erreur X, et le gestionnaire par défaut de celles-ci termine le
programme sur-le-champ). `HandleEvent` remet l'identifiant à zéro quand la
fenêtre a déjà disparu ; `CloseWindow` ne détruit que ce qui existe encore. La
leçon 029 reprend ce fil — *chaque* chemin de sortie libère exactement ses
ressources, exactement une fois — et en fait tout le sujet.

### Le côté moteur

La boucle du moteur fait quatre lignes et tout l'enjeu de la leçon :

```c++
while (!platform::CloseRequested(opened.window))
    platform::PumpEvents(opened.window);
```

Lire les nouvelles, les plier, réagir à l'état, recommencer. Quand le drapeau
passe à vrai, la boucle se termine, le moteur le rapporte, et ferme ce qu'il a
ouvert :

```
$ ./build.sh
build: compiling 2 source(s) from src/
  CC  src/main.cpp
  CC  src/platform_x11.cpp
  LD  build/game
build: OK (2 source(s) compiled -> build/game)
$ DISPLAY=:99 ./build/game &
$ DISPLAY=:99 xdotool search --name "the framebuffer engine"
2097153
$ DISPLAY=:99 xdotool windowclose 2097153
engine: window 640x480 open — waiting for news
engine: close reported
engine: closed
```

C'est la vérification sans écran du comportement documenté de la leçon : la
fenêtre s'ouvre à la taille demandée par le moteur, l'exécution **reste en
vie** (elle est encore là une seconde plus tard, toujours en train de
répondre), la fermeture est **rapportée** au moteur, et l'exécution se termine
**avec ses ressources d'OS libérées** — après la sortie du processus, la
fenêtre a disparu du serveur et la connexion au display est fermée :

```
$ DISPLAY=:99 xdotool search --name "the framebuffer engine"
$ 
```

Sur un vrai bureau, la même exécution ouvre la fenêtre, attend, et votre
bouton de fermeture envoie la demande polie plutôt que le passage à l'acte —
même drapeau, même sortie. L'exercice 2 instrumente le pliage pour que vous
voyiez quelle issue produit votre machine.

## Étape de code

Un seul changement pour cette leçon : la couture gagne `PumpEvents` et
`CloseRequested`, le côté X11 apprend à attendre et à écouter les deux façons
dont la nouvelle de fermeture arrive, `main.cpp` remplace son substitut
`getchar` par la vraie boucle de pompe, et `CloseWindow` apprend à ne pas
détruire deux fois. Son état final est étiqueté `lesson-028`.

```diff
diff --git a/src/main.cpp b/src/main.cpp
index 106b3b0..1fa92f3 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -1,10 +1,9 @@
 // main.cpp — the engine, born.
 //
-// Lesson 027: the engine meets the OS through the platform seam. This file
-// includes no OS headers and names no OS type — it sees platform.h and
-// nothing else. The language law of lesson 026 holds: engine code lives in
-// namespace engine, main stays global, and every feature here is one the
-// law admits.
+// Lesson 028: the engine stays alive by reading the OS's news through the
+// seam. Still no OS headers, still no OS types — the pump is one more
+// platform function and the engine polls what it leaves behind. The
+// language law of lesson 026 holds.
 
 #include <cstdio>
 
@@ -25,13 +24,15 @@ int Run(void)
         return 1;
     }
 
-    std::printf("engine: window %dx%d open — press enter to close\n",
+    std::printf("engine: window %dx%d open — waiting for news\n",
                 WINDOW_WIDTH, WINDOW_HEIGHT);
 
-    /* Stand-in for the event pump: hold the window open until enter.
-       Lesson 028 replaces exactly this. */
-    std::getchar();
+    /* The event pump: read news, fold it into state, react to state,
+       repeat. This loop is what keeps the window alive. */
+    while (!platform::CloseRequested(opened.window))
+        platform::PumpEvents(opened.window);
 
+    std::printf("engine: close reported\n");
     platform::CloseWindow(opened.window);
     std::printf("engine: closed\n");
     return 0;
diff --git a/src/platform.h b/src/platform.h
index 361251c..d6aa25e 100644
--- a/src/platform.h
+++ b/src/platform.h
@@ -29,6 +29,14 @@ struct WindowResult {
 /* Opens a window of exactly the requested size on the OS's display. */
 WindowResult OpenWindow(int width, int height);
 
+/* Reads whatever news the OS has about this window and folds it into the
+   platform layer's state. The engine never sees an event object — it polls
+   state afterwards. Blocks until there is news. */
+void PumpEvents(Window *window);
+
+/* True once the user has asked for this window to close. */
+bool CloseRequested(const Window *window);
+
 /* Releases everything OpenWindow took from the OS. */
 void CloseWindow(Window *window);
 
diff --git a/src/platform_x11.cpp b/src/platform_x11.cpp
index 203b181..68de824 100644
--- a/src/platform_x11.cpp
+++ b/src/platform_x11.cpp
@@ -4,6 +4,9 @@
 // included here and nowhere else, and every OS type is spelled here and
 // nowhere else. An implementation for a second OS would live in its own
 // file beside this one and nothing outside would change.
+//
+// Lesson 028: the event pump. The OS's news arrives here as X events and is
+// folded into state the engine polls — the engine never reads an event.
 
 #include "platform.h"
 
@@ -17,6 +20,7 @@ namespace platform {
 struct Window {
     Display *display;
     ::Window xwindow;
+    bool close_requested;
 };
 
 /* The OS state for one window, in static storage: no new, no delete — the
@@ -25,6 +29,12 @@ struct Window {
    part. The engine only ever holds the pointer. */
 static Window window_state;
 
+/* The two atoms of the window-close conversation. The window manager does
+   not destroy a window behind its owner's back — it *asks*, by sending a
+   ClientMessage whose first word is WM_DELETE_WINDOW. X atoms are names the
+   server hands out as integers; asking for them is how you spell them. */
+static Atom wm_delete_window;
+
 WindowResult OpenWindow(int width, int height)
 {
     WindowResult result = { &window_state, OPEN_NO_DISPLAY };
@@ -47,21 +57,65 @@ WindowResult OpenWindow(int width, int height)
         return result;
     }
 
+    /* Register the close request as the way to go, and subscribe to the
+       window's lifecycle news (map, configure, destroy). */
+    wm_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", False);
+    XSetWMProtocols(display, xwindow, &wm_delete_window, 1);
+    XSelectInput(display, xwindow, StructureNotifyMask);
+
     XStoreName(display, xwindow, "the framebuffer engine");
     XMapWindow(display, xwindow);
     XFlush(display);
 
     window_state.display = display;
     window_state.xwindow = xwindow;
+    window_state.close_requested = false;
     return result;
 }
 
+/* One piece of news, folded into state. The request and the deed both mean
+   the same thing to the engine. */
+static void HandleEvent(Window *window, XEvent &event)
+{
+    if (event.type == ClientMessage &&
+        (Atom)event.xclient.data.l[0] == wm_delete_window) {
+        window->close_requested = true; /* the polite request */
+    } else if (event.type == DestroyNotify) {
+        window->close_requested = true; /* the window is already gone */
+        window->xwindow = 0;
+    }
+}
+
+void PumpEvents(Window *window)
+{
+    if (!window || !window->display)
+        return;
+
+    /* Block for the first piece of news, then drain whatever else piled up.
+       Blocking is the point: the engine waits here instead of spinning. */
+    XEvent event;
+    XNextEvent(window->display, &event);
+    HandleEvent(window, event);
+    while (XPending(window->display)) {
+        XNextEvent(window->display, &event);
+        HandleEvent(window, event);
+    }
+}
+
+bool CloseRequested(const Window *window)
+{
+    return window && window->close_requested;
+}
+
 void CloseWindow(Window *window)
 {
     if (!window || !window->display)
         return;
 
-    XDestroyWindow(window->display, window->xwindow);
+    /* xwindow is zero once the OS has already destroyed the window;
+       destroying it twice would be an X error. */
+    if (window->xwindow)
+        XDestroyWindow(window->display, window->xwindow);
     XCloseDisplay(window->display);
     window->display = 0;
     window->xwindow = 0;
```

## Exercices

Deux extensions « faites-les vôtres ». Chacune se termine par sa solution — un
diff contre l'état final de cette leçon, plus une visite guidée — après
l'énoncé.

### Exercice 1 — Le redimensionnement est une nouvelle aussi *(extend-the-code)*

La pompe plie une nouvelle ; faites-lui en plier une autre. Suivez la taille
de la fenêtre à travers les événements `ConfigureNotify` — l'OS rapporte la
nouvelle largeur et la nouvelle hauteur chaque fois que la géométrie change —
enregistrez-la dans l'état de la fenêtre dès que `OpenWindow` demande une
taille, et rapportez-la quand l'exécution se termine
(`engine: closed at 800x600`). Ajoutez l'interrogation à la couture comme
cette leçon a ajouté `CloseRequested` — types simples uniquement. Montrez que
ça marche : redimensionnez la fenêtre pendant que le moteur tourne (tirez un
coin sur votre bureau, ou `xdotool windowsize <id> 800 600` sans écran) et
fermez-la.

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-028/ex1.md)

### Exercice 2 — Deux façons dont la nouvelle arrive *(port-to-your-own-machine)*

Faites s'annoncer le pliage : une ligne d'instrumentation par chemin de
fermeture — la demande (`ClientMessage`) et le passage à l'acte
(`DestroyNotify`) — imprimée depuis l'intérieur de l'implémentation. Puis
produisez les deux issues sur votre propre machine : fermez la fenêtre avec le
bouton de fermeture de votre bureau, et de nouveau de façon abrupte
(`xdotool windowclose <id>` depuis un autre terminal, ce qui détruit la
fenêtre sans demander). Notez ce que chaque exécution a produit, laquelle
produit votre geste de fermeture, et pourquoi la pompe plie les deux dans le
même drapeau alors que le nettoyage les traite encore différemment.

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-028/ex2.md)

---

**Partie :** [Partie 1 — la couche plateforme](../../index.md) ·
**Précédente :** [Leçon 027 — la couture plateforme et la première fenêtre X11](lesson-027-first-window.md) ·
**Suivante :** [Leçon 029 — fermeture propre et chemins d'erreur](lesson-029-clean-close.md) ·
**Étiquette de code :** [`lesson-028`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-028)

*Page traduite de la version anglaise `book/lessons/part-1/lesson-028-event-pump.md`, révision `8904d11`.*

<!-- translation-source: book/lessons/part-1/lesson-028-event-pump.md @ 8904d11 -->
