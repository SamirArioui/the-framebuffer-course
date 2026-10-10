# Leçon 032 — l'état d'entrée par scrutation

{{#include ../../stability-horizon.md}}

## Prose

Le moteur sait ouvrir une fenêtre, la garder en vie, la remplir de ses propres
pixels — et il ne sait pas si quelqu'un touche au clavier. Aujourd'hui, cela
change, et cela change la façon dont le moteur pense : non pas en lui tendant un
flux d'événements à consommer, mais en lui donnant **un état à scruter**.
Quelles touches sont enfoncées, maintenant, chaque fois que le moteur demande.
C'est le contrat sur lequel s'appuieront le code de déplacement de la partie 2
et la boîte à outils du game feel de la partie 5.

### L'état, pas les événements

La couture gagne deux choses :

```c++
enum Key {
    KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT,
    KEY_SPACE, KEY_ENTER, KEY_ESCAPE, KEY_COUNT
};

bool KeyDown(const Window *window, Key key);
```

`KeyDown` répond à une seule question — *cette touche est-elle enfoncée au
moment de l'appel ?* — et c'est cette question que la logique de jeu pose
réellement. « Déplace le marqueur tant que gauche est maintenue » ne veut pas
une file d'événements de touche horodatés ; il veut savoir, une fois par frame,
si gauche est enfoncée. Les événements sont la façon de penser de l'OS ; l'état
scruté est la façon de penser d'une frame.

La division du travail de la leçon 028 porte désormais ses fruits :
**les nouvelles entrent dans l'état ; le moteur lit l'état**. Les événements de
touche sont pliés dans une petite table de bits à l'intérieur de la couche
plateforme, puis jetés. Le moteur scrute la table à travers une seule fonction
et ne voit jamais un objet événement, un code de touche de l'OS, ni une file
d'événements.

### Les touches appartiennent à la couture

`Key` appartient au vocabulaire de la couture, pas à celui de X11. L'OS a ses
propres codes de touche (X11 les appelle des keysyms — `XK_Left`, `XK_space`)
et ils ne franchissent jamais la frontière. La traduction vit dans une seule
fonction, dans un seul fichier :

```c++
static int KeyIndex(KeySym sym)
{
    switch (sym) {
    case XK_Up:     return KEY_UP;
    ...
    case XK_space:  return KEY_SPACE;
    ...
    }
}
```

Un second OS traduirait ses propres codes de touche vers les mêmes valeurs de
`Key` sans que le moteur s'en aperçoive. Les touches que le moteur *suit* sont
une décision de contrat — sept suffisent amplement pour un jeu qui se déplace
avec les flèches et valide avec espace — et l'exercice 1 fait grandir cet
ensemble à exactement trois endroits.

### Une touche maintenue reste maintenue

Un comportement de l'OS mérite son propre paragraphe, car il casse les
implémentations naïves : **l'auto-repeat**. Maintenez une touche sur un vrai
clavier et l'OS émet des appuis répétés — mais avec le réglage par défaut, il
entrelace aussi des *relâchements* : appui, relâchement, appui, relâchement,
appui... Un pliage qui ferait confiance à chaque événement ferait clignoter
l'état entre enfoncée et relâchée alors que votre doigt n'a jamais bougé.

L'OS sait pourtant dire la vérité. `XkbSetDetectableAutoRepeat` fait arriver
les répétitions comme des appuis *uniquement* — aucun relâchement fantôme — si
bien que l'état plié reste enfoncé aussi longtemps que la touche est maintenue :

```
$ DISPLAY=:99 xdotool keydown --window <id> Left
$ DISPLAY=:99 xdotool keydown --window <id> Right
$ DISPLAY=:99 xdotool keyup --window <id> Right
$ DISPLAY=:99 xdotool keyup --window <id> Left
engine: polled: left
engine: polled: left right
engine: polled: left
engine: polled: -
```

Quatre scrutations, une par geste. `left` apparaît dans la première et est
*encore là* dans la troisième — une touche maintenue le reste d'une scrutation
à l'autre — tandis que `right` arrive et repart entre-temps. La dernière
scrutation revoit un état vide.

### Ce que la scrutation rapporte, et quand

Le moteur scrute une fois par étape de frame — après que la pompe a plié toutes
les nouvelles arrivées. Ce moment est un contrat en soi : **la scrutation
rapporte l'état tel qu'au dernier pliage**. C'est exactement ce que veut dire
« quelles touches sont enfoncées au moment de la scrutation ».

C'est aussi là que se trouve la limite de ce contrat. Une touche peut être
enfoncée *et* relâchée entre deux scrutations — plus vite que la frame — et
l'état ne le montre jamais : enfoncement et relâchement s'annulent avant que
quiconque ne demande. L'état est honnête et l'appui est perdu. Ce n'est pas un
bug du pliage (les deux événements sont pliés ; les deux affectations
s'exécutent) ; c'est la forme de l'entrée par état seul, et c'est tout le sujet
de la leçon 033 : le moteur a besoin d'une pièce d'état de plus — pas seulement
*est enfoncée maintenant*, mais *s'est enfoncée depuis la dernière scrutation*.

## Étape de code

Un seul changement pour cette leçon : la couture gagne `Key` et `KeyDown`, le
côté X11 plie les événements de touche dans une table d'état (avec
l'auto-repeat détectable pour qu'une touche maintenue le reste), et `main.cpp`
scrute l'état à chaque étape de frame et rapporte ce qu'il voit. Son état final
est étiqueté `lesson-032`.

```diff
diff --git a/src/main.cpp b/src/main.cpp
index 2dc455a..91be4f0 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -12,6 +12,11 @@
 
 namespace engine {
 
+/* The seam's keys, by name — for the report below. */
+static const char *const key_names[platform::KEY_COUNT] = {
+    "up", "down", "left", "right", "space", "enter", "escape",
+};
+
 int Run(void)
 {
     platform::WindowResult opened =
@@ -73,6 +78,18 @@ int Run(void)
         platform::PumpEvents(opened.window);
         if (platform::CloseRequested(opened.window))
             break;
+
+        /* The polled state: what is down right now, as of this poll. */
+        std::printf("engine: polled:");
+        bool any = false;
+        for (int k = 0; k < platform::KEY_COUNT; ++k) {
+            if (platform::KeyDown(opened.window, (platform::Key)k)) {
+                std::printf(" %s", key_names[k]);
+                any = true;
+            }
+        }
+        std::printf(any ? "\n" : " -\n");
+
         if (!platform::Present(opened.window, fb->pixels, fb->width,
                                fb->height)) {
             /* A present can fail because the window died mid-copy — that
diff --git a/src/platform.h b/src/platform.h
index e23434d..aac8d48 100644
--- a/src/platform.h
+++ b/src/platform.h
@@ -29,6 +29,23 @@ struct WindowResult {
 /* Opens a window of exactly the requested size on the OS's display. */
 WindowResult OpenWindow(int width, int height);
 
+/* The keys the engine tracks. Plain values — no OS key code ever crosses
+   the seam. */
+enum Key {
+    KEY_UP = 0,
+    KEY_DOWN,
+    KEY_LEFT,
+    KEY_RIGHT,
+    KEY_SPACE,
+    KEY_ENTER,
+    KEY_ESCAPE,
+    KEY_COUNT
+};
+
+/* The polled input state: true while the key is down at the moment of the
+   call. State, not events — the engine asks, it never consumes a stream. */
+bool KeyDown(const Window *window, Key key);
+
 /* Reads whatever news the OS has about this window and folds it into the
    platform layer's state. The engine never sees an event object — it polls
    state afterwards. Blocks until there is news or the run is interrupted. */
diff --git a/src/platform_x11.cpp b/src/platform_x11.cpp
index 19bff08..22f4c19 100644
--- a/src/platform_x11.cpp
+++ b/src/platform_x11.cpp
@@ -10,8 +10,10 @@
 
 #include "platform.h"
 
+#include <X11/XKBlib.h>
 #include <X11/Xlib.h>
 #include <X11/Xutil.h>
+#include <X11/keysym.h>
 
 #include <cstdio>
 #include <poll.h>
@@ -26,6 +28,7 @@ struct Window {
     Display *display;
     ::Window xwindow;
     bool close_requested;
+    bool keys[KEY_COUNT];
 };
 
 /* The OS state for one window, in static storage: no new, no delete — the
@@ -89,11 +92,20 @@ WindowResult OpenWindow(int width, int height)
 
     /* Register the close request as the way to go, and subscribe to the
        window's lifecycle news (map, configure, destroy) — plus Expose, the
-       "your pixels are gone" news. The engine handles Expose by doing the
-       only thing that repairs a window: presenting again. */
+       "your pixels are gone" news, and the keyboard. The engine handles
+       Expose by doing the only thing that repairs a window: presenting
+       again. */
     wm_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", False);
     XSetWMProtocols(display, xwindow, &wm_delete_window, 1);
-    XSelectInput(display, xwindow, StructureNotifyMask | ExposureMask);
+    XSelectInput(display, xwindow,
+                 StructureNotifyMask | ExposureMask | KeyPressMask |
+                     KeyReleaseMask);
+
+    /* Auto-repeat would otherwise look like release-then-press every
+       repeat: a held key would flicker in polled state. Detectable
+       auto-repeat makes repeats arrive as presses only, so "held" stays
+       held. */
+    XkbSetDetectableAutoRepeat(display, True, 0);
 
     XStoreName(display, xwindow, "the framebuffer engine");
     XMapWindow(display, xwindow);
@@ -102,6 +114,8 @@ WindowResult OpenWindow(int width, int height)
     window_state.display = display;
     window_state.xwindow = xwindow;
     window_state.close_requested = false;
+    for (int i = 0; i < KEY_COUNT; ++i)
+        window_state.keys[i] = false;
 
     /* The interrupt is part of the window's take: from here on, Ctrl+C is
        news like any other — and X errors are recorded, not fatal. */
@@ -111,6 +125,23 @@ WindowResult OpenWindow(int width, int height)
     return result;
 }
 
+/* Which of our keys an OS key event is about, or -1 for keys we do not
+   track. The translation from OS key codes to the seam's Key lives here
+   and nowhere else. */
+static int KeyIndex(KeySym sym)
+{
+    switch (sym) {
+    case XK_Up:     return KEY_UP;
+    case XK_Down:   return KEY_DOWN;
+    case XK_Left:   return KEY_LEFT;
+    case XK_Right:  return KEY_RIGHT;
+    case XK_space:  return KEY_SPACE;
+    case XK_Return: return KEY_ENTER;
+    case XK_Escape: return KEY_ESCAPE;
+    default:        return -1;
+    }
+}
+
 /* One piece of news, folded into state. The request and the deed both mean
    the same thing to the engine. */
 static void HandleEvent(Window *window, XEvent &event)
@@ -121,9 +152,18 @@ static void HandleEvent(Window *window, XEvent &event)
     } else if (event.type == DestroyNotify) {
         window->close_requested = true; /* the window is already gone */
         window->xwindow = 0;
+    } else if (event.type == KeyPress || event.type == KeyRelease) {
+        int key = KeyIndex(XLookupKeysym(&event.xkey, 0));
+        if (key >= 0)
+            window->keys[key] = (event.type == KeyPress);
     }
 }
 
+bool KeyDown(const Window *window, Key key)
+{
+    return window && key >= 0 && key < KEY_COUNT && window->keys[key];
+}
+
 void PumpEvents(Window *window)
 {
     if (!window || !window->display)
```

## Exercices

Deux extensions « faites-les vôtres ». Chacune se termine par sa solution — un
diff contre l'état final de cette leçon, plus une visite guidée — après
l'énoncé.

### Exercice 1 — Vos propres touches *(extend-the-code)*

Les flèches ne sont pas la seule façon de se déplacer. Faites grandir
l'ensemble des touches suivies par la couture avec W/A/S/D — l'énum, la table
de correspondance et le rapport — et montrez-les suivre indépendamment :
maintenez `w`, ajoutez et retirez `d`, observez chaque scrutation. Où vit la
traduction des codes de touche de l'OS vers les touches de la couture, et
combien de fichiers votre changement a-t-il dû toucher ? (Regardez ce que sont
`XK_w` et `XK_W`, et demandez-vous laquelle des deux une entrée de déplacement
doit prendre en compte.)

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-032/ex1.md)

### Exercice 2 — L'appui qui a disparu *(predict-the-output)*

Une touche peut être enfoncée et relâchée plus vite que le moteur ne scrute.
*Prédisez* ce que montre le rapport pour un geste unique sans attente —
`xdotool key --delay 0 --window <id> space` place un appui et un relâchement
dans un même lot de pompe. Écrivez la prédiction avant de lancer quoi que ce
soit. Rendez ensuite le pliage visible — une ligne d'instrumentation par
événement de touche, imprimée depuis l'intérieur de l'implémentation — lancez
le geste, et réconciliez. Que perd ici le contrat fondé sur l'état, et de quoi
le moteur aurait-il besoin pour ne pas le perdre ?

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-032/ex2.md)

---

**Partie :** [Partie 1 — la couche plateforme](../../index.md) ·
**Précédente :** [Leçon 031 — la présentation à travers la couche plateforme](lesson-031-present.md) ·
**Suivante :** [Leçon 033 — mémoriser les appuis brefs et suivre le focus](lesson-033-latching.md) ·
**Étiquette de code :** [`lesson-032`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-032)

*Page traduite de la version anglaise `book/lessons/part-1/lesson-032-polled-input.md`, révision `a3b9017`.*

<!-- translation-source: book/lessons/part-1/lesson-032-polled-input.md @ a3b9017 -->
