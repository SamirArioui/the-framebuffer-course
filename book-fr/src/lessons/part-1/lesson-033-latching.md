# Leçon 033 — mémoriser les appuis brefs et suivre le focus

{{#include ../../stability-horizon.md}}

## Prose

La leçon 032 s'est arrêtée au bord d'une falaise : l'état scruté répond
« qu'est-ce qui est enfoncé *maintenant* », et un appui déjà terminé n'est pas
enfoncé maintenant — un enfoncement et un relâchement entre deux scrutations,
et le moteur ne les voit jamais. Cette leçon bouche ce trou avec une seconde
pièce d'état, la **mémorisation** (latch), puis corrige le bug suivant qui
attend dans l'état d'entrée : la touche qui continue de bouger après que la
fenêtre a perdu le clavier.

### La mémorisation

La couture gagne une requête :

```c++
bool KeyPressed(Window *window, Key key);
```

`KeyDown` dit *est enfoncée maintenant* ; `KeyPressed` dit **s'est enfoncée
depuis la dernière fois que vous avez demandé**. C'est la promesse de la
spécification rendue mécanique : *une touche enfoncée et relâchée dans une même
frame n'est pas perdue* — l'appui allume la mémorisation au moment où il se
produit, et elle reste allumée jusqu'à ce que le moteur l'ait vue. La lire
l'efface : la mémorisation est une petite pièce d'état à un seul consommateur,
et le moteur la scrute comme le reste. Toujours aucun flux d'événements.

Le pliage l'allume sur le **front** — la touche qui passe de relâchée à
enfoncée :

```c++
if (event.type == KeyPress) {
    if (!window->keys[key])
        window->pressed[key] = true;
    window->keys[key] = true;
} else {
    window->keys[key] = false;
}
```

Le front compte à cause de l'auto-repeat de la leçon 032 : une touche maintenue
émet des appuis en flux, mais une touche maintenue ne *s'est pas enfoncée* de
nouveau. Un appui, une mémorisation, quelle que soit la durée du maintien — et
quel que soit le nombre de répétitions qui arrivent.

La preuve, avec une frappe plus rapide que la scrutation — appui et
relâchement atterrissant dans un même lot :

```
$ DISPLAY=:99 xdotool key --delay 0 --window <id> space
engine: polled: -
engine: pressed space
engine: polled: -
```

L'état scruté est honnête — rien n'est enfoncé — et l'appui est *toujours là*,
rapporté par la mémorisation à la scrutation suivante. (Qu'une frappe atterrisse
dans un lot de pompe ou dans deux est une question de timing ; la mémorisation
rend le résultat identique dans les deux cas. C'est tout son intérêt.)

### Suivre le focus

Le second bug est plus ancien que cette leçon et il a un nom : **la touche
bloquée**. Maintenez la touche de déplacement, changez de fenêtre avec alt-tab,
relâchez la touche dans l'autre fenêtre. Le relâchement se produit là-bas —
notre fenêtre ne le voit jamais — et `keys[KEY_LEFT]` reste à vrai. Lâchez le
clavier et regardez le marqueur continuer.

L'OS sait quand cela arrive : `FocusIn` et `FocusOut` sont des événements comme
les autres, et le pliage les traite comme des nouvelles d'entrée — parce que
c'est exactement ce qu'ils sont :

```c++
} else if (event.type == FocusOut) {
    window->focused = false;
    for (int i = 0; i < KEY_COUNT; ++i)
        window->keys[i] = false;
}
```

Toute touche maintenue est **abandonnée** à la perte du focus. Pas marquée
relâchée, pas laissée pour plus tard — abandonnée, parce qu'aucun événement de
relâchement n'arrivera jamais pour elle et qu'un état qui ne peut jamais finir
n'est pas un état. Le focus revient (`FocusIn`) mais pas les touches : le focus
n'est pas de l'état d'entrée, c'est une *nouvelle à propos de* l'état d'entrée.

Le moteur voit l'autre moitié à travers une seconde requête, `HasFocus` —
encore de l'état brut — et peut atténuer, mettre en pause ou ignorer l'entrée
quand la fenêtre n'est pas celle où l'on tape :

```
$ DISPLAY=:99 xdotool windowfocus <id>
$ DISPLAY=:99 xdotool keydown --window <id> Left
$ DISPLAY=:99 xdotool windowfocus 0
engine: polled: -
engine: focus gained
engine: polled: left
engine: pressed left
engine: polled: -
engine: focus lost
```

`left` maintenue, le focus parti, et la scrutation suivante est vide. L'appui
n'est même pas perdu — `pressed left` a déjà été rapporté quand la touche s'est
enfoncée. L'état est exactement l'entrée que la fenêtre possède vraiment.

### Ce que le moteur sait désormais

Trois requêtes, trois questions, toutes de l'état :

| Requête | Question |
| ------- | -------- |
| `KeyDown` | la touche est-elle enfoncée maintenant ? |
| `KeyPressed` | s'est-elle enfoncée depuis ma dernière consultation ? |
| `HasFocus` | cette fenêtre est-elle celle où l'on tape ? |

La leçon 034 déplace un marqueur avec la première — et les mémorisations sont la
raison pour laquelle le saut du marqueur n'aura pas besoin d'un second appui
« juste pour être sûr ».

## Étape de code

Un seul changement pour cette leçon : la couture gagne `KeyPressed` et
`HasFocus`, le pliage mémorise les appuis sur leur front et abandonne les
touches maintenues quand le focus est perdu, et `main.cpp` rapporte les
mémorisations et les changements de focus aux côtés de l'état scruté. Son état
final est étiqueté `lesson-033`.

```diff
diff --git a/src/main.cpp b/src/main.cpp
index 91be4f0..0d0d64b 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -74,6 +74,7 @@ int Run(void)
        React first: if the news was "the window is gone", there is nothing
        left to present to. */
     int exit_code = 0;
+    bool had_focus = platform::HasFocus(opened.window);
     while (!platform::CloseRequested(opened.window)) {
         platform::PumpEvents(opened.window);
         if (platform::CloseRequested(opened.window))
@@ -90,6 +91,18 @@ int Run(void)
         }
         std::printf(any ? "\n" : " -\n");
 
+        /* The latches: presses that ended before this poll are not lost. */
+        for (int k = 0; k < platform::KEY_COUNT; ++k)
+            if (platform::KeyPressed(opened.window, (platform::Key)k))
+                std::printf("engine: pressed %s\n", key_names[k]);
+
+        /* Focus: reported when it changes. */
+        bool focus = platform::HasFocus(opened.window);
+        if (focus != had_focus) {
+            std::printf("engine: focus %s\n", focus ? "gained" : "lost");
+            had_focus = focus;
+        }
+
         if (!platform::Present(opened.window, fb->pixels, fb->width,
                                fb->height)) {
             /* A present can fail because the window died mid-copy — that
diff --git a/src/platform.h b/src/platform.h
index aac8d48..643bdef 100644
--- a/src/platform.h
+++ b/src/platform.h
@@ -46,6 +46,16 @@ enum Key {
    call. State, not events — the engine asks, it never consumes a stream. */
 bool KeyDown(const Window *window, Key key);
 
+/* True if the key went down since the last time that key was polled this
+   way. A press that ended before the poll is not lost; the latch clears
+   when the engine has seen it. */
+bool KeyPressed(Window *window, Key key);
+
+/* True while the window has keyboard focus. Keys held when focus is lost
+   are dropped by the platform layer — no release event will ever arrive
+   for them. */
+bool HasFocus(const Window *window);
+
 /* Reads whatever news the OS has about this window and folds it into the
    platform layer's state. The engine never sees an event object — it polls
    state afterwards. Blocks until there is news or the run is interrupted. */
diff --git a/src/platform_x11.cpp b/src/platform_x11.cpp
index 22f4c19..4e23966 100644
--- a/src/platform_x11.cpp
+++ b/src/platform_x11.cpp
@@ -29,6 +29,8 @@ struct Window {
     ::Window xwindow;
     bool close_requested;
     bool keys[KEY_COUNT];
+    bool pressed[KEY_COUNT]; /* latched: went down since last observed */
+    bool focused;
 };
 
 /* The OS state for one window, in static storage: no new, no delete — the
@@ -99,7 +101,7 @@ WindowResult OpenWindow(int width, int height)
     XSetWMProtocols(display, xwindow, &wm_delete_window, 1);
     XSelectInput(display, xwindow,
                  StructureNotifyMask | ExposureMask | KeyPressMask |
-                     KeyReleaseMask);
+                     KeyReleaseMask | FocusChangeMask);
 
     /* Auto-repeat would otherwise look like release-then-press every
        repeat: a held key would flicker in polled state. Detectable
@@ -114,8 +116,11 @@ WindowResult OpenWindow(int width, int height)
     window_state.display = display;
     window_state.xwindow = xwindow;
     window_state.close_requested = false;
-    for (int i = 0; i < KEY_COUNT; ++i)
+    for (int i = 0; i < KEY_COUNT; ++i) {
         window_state.keys[i] = false;
+        window_state.pressed[i] = false;
+    }
+    window_state.focused = false;
 
     /* The interrupt is part of the window's take: from here on, Ctrl+C is
        news like any other — and X errors are recorded, not fatal. */
@@ -154,8 +159,26 @@ static void HandleEvent(Window *window, XEvent &event)
         window->xwindow = 0;
     } else if (event.type == KeyPress || event.type == KeyRelease) {
         int key = KeyIndex(XLookupKeysym(&event.xkey, 0));
-        if (key >= 0)
-            window->keys[key] = (event.type == KeyPress);
+        if (key >= 0) {
+            if (event.type == KeyPress) {
+                /* The latch lights on the edge — a key going from up to
+                   down. Auto-repeat presses (a held key) arrive as presses
+                   too, but a held key did not go down again. */
+                if (!window->keys[key])
+                    window->pressed[key] = true;
+                window->keys[key] = true;
+            } else {
+                window->keys[key] = false;
+            }
+        }
+    } else if (event.type == FocusIn) {
+        window->focused = true;
+    } else if (event.type == FocusOut) {
+        /* Keys held while focus left will never send their release here —
+           drop them or they stay down forever. */
+        window->focused = false;
+        for (int i = 0; i < KEY_COUNT; ++i)
+            window->keys[i] = false;
     }
 }
 
@@ -164,6 +187,19 @@ bool KeyDown(const Window *window, Key key)
     return window && key >= 0 && key < KEY_COUNT && window->keys[key];
 }
 
+bool KeyPressed(Window *window, Key key)
+{
+    if (!window || key < 0 || key >= KEY_COUNT || !window->pressed[key])
+        return false;
+    window->pressed[key] = false; /* observed; the latch clears */
+    return true;
+}
+
+bool HasFocus(const Window *window)
+{
+    return window && window->focused;
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

### Exercice 1 — Le compteur d'appuis *(extend-the-code)*

La mémorisation répond à « s'est-elle enfoncée ? » — au moins une fois. Deux
frappes dans un même lot l'allument exactement comme une seule. Faites-la
grandir vers la forme générale : un compteur par touche — `KeyPressCount`
rapporte combien de fois la touche s'est enfoncée depuis la dernière scrutation
de cette touche — et gardez à `KeyPressed` son sens actuel (`count > 0`).
Montrez-le compter : deux frappes dans un même lot (`xdotool key --delay 0
--repeat 2 --window <id> space`) doivent rapporter `pressed space x2`. Tant que
vous y êtes : une touche maintenue qui fait de l'auto-repeat incrémente-t-elle
le compteur ? Pourquoi non ?

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-033/ex1.md)

### Exercice 2 — La touche qui ne veut pas lâcher *(port-to-your-own-machine)*

La build d'un coéquipier a le bug classique : maintenez la touche de
déplacement, changez de fenêtre avec alt-tab, relâchez la touche dans l'autre
fenêtre — et le marqueur continue de bouger après que vous avez lâché. Prédisez
exactement quelle pièce d'état survit et pourquoi aucun événement ne la
corrigera jamais. Faites ensuite raconter le pliage : une ligne
d'instrumentation sur `FocusIn` et `FocusOut` à l'intérieur de
l'implémentation. Sur votre propre machine, maintenez une touche et cliquez sur
une autre fenêtre ; regardez l'abandon se produire dans le rapport — et
expliquez pourquoi abandonner *toutes* les touches maintenues à la perte du
focus est correct même si certaines sont peut-être encore physiquement
enfoncées.

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-033/ex2.md)

---

**Partie :** [Partie 1 — la couche plateforme](../../index.md) ·
**Précédente :** [Leçon 032 — l'état d'entrée par scrutation](lesson-032-polled-input.md) ·
**Suivante :** [Leçon 034 — la première frame interactive](lesson-034-first-frame.md) ·
**Étiquette de code :** [`lesson-033`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-033)

*Page traduite de la version anglaise `book/lessons/part-1/lesson-033-latching.md`, révision `3cf02bf`.*

<!-- translation-source: book/lessons/part-1/lesson-033-latching.md @ 3cf02bf -->
