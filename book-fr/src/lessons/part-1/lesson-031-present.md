# Leçon 031 — la présentation à travers la couche plateforme

{{#include ../../stability-horizon.md}}

## Prose

La leçon 030 s'est terminée avec des pixels sur une fenêtre — une fois.
Aujourd'hui, cela devient le contrat sur lequel tout le moteur va reposer :
**les pixels que le moteur a écrits sont les pixels que la fenêtre montre**,
vrai à chaque instant de l'exécution, et prouvable. La présentation cesse
d'être quelque chose que le moteur fait au démarrage et devient la réponse
qu'il donne à chaque événement — y compris les événements qui signifient « vos
pixels ont disparu ».

### Le contrat

`Present` dans `platform.h` fait maintenant une promesse qui a des dents :

> Retourne `false` si la plateforme n'a pas pu porter les pixels du tout ;
> quand il retourne `true`, les pixels sont à l'écran — la copie a eu lieu.

« A eu lieu », pas « a été envoyée ». Le `Present` de la leçon 030 terminait
par `XFlush`, qui pousse la copie vers le serveur et rend la main — les pixels
sont *en vol* quand `Present` retourne, et toute affirmation sur ce que la
fenêtre montre serait une supposition. `XSync` est la différence : il fait un
aller-retour sur la connexion et ne retourne que quand le serveur a fait le
travail. Un appel changé, et la valeur de retour de la fonction est devenue un
fait.

Pourquoi une présentation synchrone, d'ailleurs — pourquoi pas
*fire-and-forget* ? Parce que le moteur a besoin de *savoir*. La leçon 036
mesurera ce que coûte la copie, et la réponse honnête a besoin d'un chronomètre
qui s'arrête quand le travail est fait ; l'exercice 1 de cette leçon relit la
fenêtre et compare, et une comparaison avec des pixels en vol ne compare rien.
Le contrat est la fondation sur laquelle les mesures et les vérifications
reposent.

### La présentation n'est pas un événement

L'étape de frame fait trois mouvements et une règle :

```c++
while (!platform::CloseRequested(opened.window)) {
    platform::PumpEvents(opened.window);
    if (platform::CloseRequested(opened.window))
        break;
    if (!platform::Present(opened.window, fb->pixels, fb->width, fb->height))
        ...
}
```

**Réagissez d'abord, `Present` ensuite** — si la nouvelle était « la fenêtre a
disparu », il n'y a plus rien à présenter. Cet ordre est ce qui garde la
boucle honnête, et la frame interactive de la leçon 034 glissera son étape de
mise à jour exactement dans cette forme.

La règle est la partie intéressante : le moteur **rappelle `Present` après
chaque lot de nouvelles**, et c'est tout le mécanisme de réparation d'une
fenêtre abîmée. Les fenêtres X11 ne sont pas conservées — quand une autre
fenêtre recouvre la vôtre, les pixels recouverts sont *perdus* ; le serveur ne
garde rien et ne reconstruit rien. Ce qu'il envoie à la place, c'est `Expose` :
« vos pixels ont disparu ». Le moteur ne traite pas `Expose` du tout. Il ne
suit pas les dommages, ne décide pas quoi redessiner, ne répare pas de
régions. Il possède un seul framebuffer, et sa réponse à chaque événement est
d'en refaire un `Present`. L'événement ne sert qu'à réveiller la boucle.

(C'est pourquoi la pompe s'abonne maintenant à `ExposureMask` — la nouvelle
doit pouvoir arriver. Le double tampon de la leçon 022 avait une grille `front`
pour exactement ce problème ; ici, le framebuffer *est* le tampon frontal.)

### La preuve

Le scénario est vérifiable sans écran, et il a été vérifié : après un
`Present`, une relecture de la fenêtre — `XGetImage` sur la fenêtre entière —
comparée au framebuffer, pixel par pixel :

```
readback: 6348 samples, 0 mismatches; (0,0)=ff0000 (639,479)=00ff00
```

Puis la fenêtre a été endommagée exprès — dé-mappée et mappée à nouveau, ce à
quoi ressemblent les dommages vus du côté client — et la boucle a refait un
`Present` :

```
readback: 6348 samples, 0 mismatches; (0,0)=ff0000 (639,479)=00ff00
```

Chaque pixel échantillonné égale toujours les octets du moteur. L'exercice 1
intègre ce vérificateur à votre propre build pour que l'affirmation soit à
vous de vérifier, quand vous voulez.

### La fenêtre qui meurt en plein `Present`

Il reste une façon de tester le contrat, et c'est l'honnête. Fermer une
fenêtre n'est pas atomique vis-à-vis d'une exécution qui fait un `Present` :
l'OS peut détruire la fenêtre **pendant qu'un `Present` est en vol**. La copie
atteint le serveur après la disparition de la fenêtre, et la réponse du
serveur est une erreur X — `BadDrawable` — qui, avec le gestionnaire par
défaut, termine le processus sur place. Une course comme celle-là n'est pas un
crash à accepter ; c'est une panne à rapporter.

Alors la couche plateforme traite les erreurs X comme des **valeurs**, de la
même façon qu'elle traite toutes les autres pannes : un gestionnaire
enregistre ce qui s'est passé, `Present` l'interprète après la
synchronisation, et une erreur sur notre propre fenêtre est pliée dans la même
nouvelle de fermeture que `DestroyNotify` — l'acte, arrivé par une autre voie.
L'exécution ne plante pas ; elle se termine comme toutes les autres fins :

```
$ DISPLAY=:99 ./build/game &
$ DISPLAY=:99 xdotool windowclose <id>
engine: framebuffer 640x480, 1228800 bytes, stride 2560
engine: pixel (0,0) = 255 0 0
engine: pixel (639,479) = 0 255 0
engine: pixel (60,101) = 32 32 64
engine: presented
engine: close reported
engine: closed
```

Que la destruction soit remarquée avant, pendant ou après le `Present` suivant,
la fin est la même — ressources libérées, code de sortie 0. Fermer l'exécution
huit fois de suite à des moments arbitraires a produit huit fins identiques et
sans histoire. C'est ce que « l'exécution libère ses ressources d'OS à chaque
sortie » a toujours voulu dire.

## Étape de code

Un seul changement pour cette leçon : `Present` se termine de façon synchrone
et rapporte ses pannes comme des valeurs — y compris la fenêtre qui meurt en
pleine copie — la pompe s'abonne à `Expose` pour que les dommages puissent
réveiller le moteur, et `main.cpp` gagne l'étape de frame qui rappelle
`Present` après chaque lot de nouvelles. Son état final est étiqueté
`lesson-031`.

```diff
diff --git a/src/main.cpp b/src/main.cpp
index 3245656..2dc455a 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -55,16 +55,42 @@ int Run(void)
     std::printf("engine: pixel (60,101) = %d %d %d\n", r, g, b);
 
     /* First light: the bytes go to the window through the seam. */
-    platform::Present(opened.window, fb->pixels, fb->width, fb->height);
+    if (!platform::Present(opened.window, fb->pixels, fb->width, fb->height)) {
+        std::fprintf(stderr, "engine: presentation failed\n");
+        platform::CloseWindow(opened.window);
+        return 1;
+    }
     std::printf("engine: presented\n");
 
-    while (!platform::CloseRequested(opened.window))
+    /* The frame step: read news, react, present our pixels, repeat.
+       Presentation is not an event — it is the engine's answer to every
+       event: the pixels the engine wrote are the pixels the window shows,
+       and re-presenting is what repairs the window when the OS damaged it.
+       React first: if the news was "the window is gone", there is nothing
+       left to present to. */
+    int exit_code = 0;
+    while (!platform::CloseRequested(opened.window)) {
         platform::PumpEvents(opened.window);
+        if (platform::CloseRequested(opened.window))
+            break;
+        if (!platform::Present(opened.window, fb->pixels, fb->width,
+                               fb->height)) {
+            /* A present can fail because the window died mid-copy — that
+               is close news and the fold already said so. Anything else is
+               a real failure and is reported as one. */
+            if (platform::CloseRequested(opened.window))
+                break;
+            std::fprintf(stderr, "engine: presentation failed\n");
+            exit_code = 1;
+            break;
+        }
+    }
 
-    std::printf("engine: close reported\n");
+    if (platform::CloseRequested(opened.window))
+        std::printf("engine: close reported\n");
     platform::CloseWindow(opened.window);
     std::printf("engine: closed\n");
-    return 0;
+    return exit_code;
 }
 
 } /* namespace engine */
diff --git a/src/platform.h b/src/platform.h
index a2e0298..e23434d 100644
--- a/src/platform.h
+++ b/src/platform.h
@@ -42,8 +42,10 @@ bool CloseRequested(const Window *window);
 /* Presents the engine's framebuffer in the window: the pixels the engine
    wrote are the pixels the window shows. The format is this interface's
    contract, not any OS's — width * height pixels of 4 bytes each (blue,
-   green, red, one unused byte), one row after another. */
-void Present(Window *window, const unsigned char *pixels, int width,
+   green, red, one unused byte), one row after another. Returns false if
+   the platform could not carry the pixels at all; when it returns true the
+   pixels are on screen — the copy has happened. */
+bool Present(Window *window, const unsigned char *pixels, int width,
              int height);
 
 /* Releases everything OpenWindow took from the OS. */
diff --git a/src/platform_x11.cpp b/src/platform_x11.cpp
index 8c93ccc..19bff08 100644
--- a/src/platform_x11.cpp
+++ b/src/platform_x11.cpp
@@ -13,6 +13,7 @@
 #include <X11/Xlib.h>
 #include <X11/Xutil.h>
 
+#include <cstdio>
 #include <poll.h>
 #include <signal.h>
 
@@ -49,6 +50,21 @@ static void OnInterrupt(int)
     interrupted = 1;
 }
 
+/* X errors are values in this layer, not process death. The default X
+   error handler would end the run on the spot — but a window can die while
+   a present is in flight, and that is news, not a crash. The handler
+   records what happened; Present decides what it means. */
+static int last_xerror;
+static unsigned long last_xerror_resource;
+
+static int OnXError(Display *display, XErrorEvent *event)
+{
+    (void)display;
+    last_xerror = event->error_code;
+    last_xerror_resource = event->resourceid;
+    return 0;
+}
+
 WindowResult OpenWindow(int width, int height)
 {
     WindowResult result = { &window_state, OPEN_NO_DISPLAY };
@@ -72,10 +88,12 @@ WindowResult OpenWindow(int width, int height)
     }
 
     /* Register the close request as the way to go, and subscribe to the
-       window's lifecycle news (map, configure, destroy). */
+       window's lifecycle news (map, configure, destroy) — plus Expose, the
+       "your pixels are gone" news. The engine handles Expose by doing the
+       only thing that repairs a window: presenting again. */
     wm_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", False);
     XSetWMProtocols(display, xwindow, &wm_delete_window, 1);
-    XSelectInput(display, xwindow, StructureNotifyMask);
+    XSelectInput(display, xwindow, StructureNotifyMask | ExposureMask);
 
     XStoreName(display, xwindow, "the framebuffer engine");
     XMapWindow(display, xwindow);
@@ -86,9 +104,10 @@ WindowResult OpenWindow(int width, int height)
     window_state.close_requested = false;
 
     /* The interrupt is part of the window's take: from here on, Ctrl+C is
-       news like any other. */
+       news like any other — and X errors are recorded, not fatal. */
     interrupted = 0;
     signal(SIGINT, OnInterrupt);
+    XSetErrorHandler(OnXError);
     return result;
 }
 
@@ -133,11 +152,11 @@ bool CloseRequested(const Window *window)
     return window && window->close_requested;
 }
 
-void Present(Window *window, const unsigned char *pixels, int width,
+bool Present(Window *window, const unsigned char *pixels, int width,
              int height)
 {
-    if (!window || !window->display)
-        return;
+    if (!window || !window->display || !window->xwindow)
+        return false; /* nothing to present to (the OS may have destroyed it) */
 
     int screen = DefaultScreen(window->display);
 
@@ -150,10 +169,12 @@ void Present(Window *window, const unsigned char *pixels, int width,
                                  ZPixmap, 0, (char *)pixels,
                                  width, height, 32, width * 4);
     if (!image)
-        return;
+        return false;
 
     /* XPutImage is where the copy happens — our bytes to the server. Its
-       cost is real; lesson 036 measures it. */
+       cost is real; lesson 036 measures it. (Its return value is not a
+       status in practice — this call either copies or raises an X error,
+       which is the OS error handler's territory.) */
     XPutImage(window->display, window->xwindow,
               DefaultGC(window->display, screen),
               image, 0, 0, 0, 0, width, height);
@@ -163,7 +184,24 @@ void Present(Window *window, const unsigned char *pixels, int width,
        lesson 004's ownership drills). */
     image->data = 0;
     XDestroyImage(image);
-    XFlush(window->display);
+
+    /* The contract: when Present returns, the pixels are on screen. XFlush
+       would only send the copy; XSync waits for the server to have done
+       it. */
+    last_xerror = 0;
+    XSync(window->display, False);
+
+    if (last_xerror) {
+        /* An error on our own window means it died mid-copy — the same
+           news as DestroyNotify, the deed: report it and stop presenting. */
+        if ((last_xerror == BadDrawable || last_xerror == BadWindow) &&
+            last_xerror_resource == (unsigned long)window->xwindow) {
+            window->close_requested = true;
+            window->xwindow = 0;
+        }
+        return false;
+    }
+    return true;
 }
 
 void CloseWindow(Window *window)
```

## Exercices

Deux extensions « faites-le vôtre ». Chacune se termine par sa solution — un
diff contre l'état final de cette leçon, plus une visite guidée — après
l'énoncé.

### Exercice 1 — La vérification de la présentation *(extend-the-code)*

Le contrat mérite un vérificateur, et le vérificateur appartient au côté OS
de la couture. Faites grandir `platform.h` d'une relecture —
`PresentedMatches` reçoit les octets du `Present` du moteur, capture la
fenêtre avec `XGetImage`, et compare chaque pixel — et faites vérifier au
moteur sa propre affirmation juste après la première lumière :
`engine: presentation verified — window matches framebuffer`. Quels pixels la
comparaison doit-elle *ignorer*, et pourquoi ? (L'exercice 2 de la leçon 030
connaît la réponse.)

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-031/ex1.md)

### Exercice 2 — Recouvrir et révéler *(port-to-your-own-machine)*

Les fenêtres X11 ne sont pas conservées : quand une autre fenêtre recouvre la
vôtre, les pixels recouverts ont disparu et le serveur vous demande de
redessiner. D'abord, *prédisez* ce que votre fenêtre montrerait après avoir
été recouverte puis révélée si le moteur ne faisait qu'un seul `Present` au
démarrage (la forme de la leçon 030). Puis rendez la réparation visible : une
ligne d'instrumentation dans le pliage, imprimée quand une nouvelle `Expose`
arrive. Sur votre propre bureau, recouvrez la fenêtre avec une autre et
révélez-la — regardez la nouvelle arriver et les pixels revenir. Notez ce qui
a réveillé le moteur, et pourquoi la réponse du moteur est « un nouveau
`Present` » plutôt que « redessiner le rectangle exposé ».

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-031/ex2.md)

---

**Partie :** [Partie 1 — la couche plateforme](../../index.md) ·
**Précédente :** [Leçon 030 — le framebuffer comme nos propres octets](lesson-030-framebuffer.md) ·
**Suivante :** [Leçon 032 — l'état d'entrée par scrutation](lesson-032-polled-input.md) ·
**Étiquette de code :** [`lesson-031`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-031)

*Page traduite de la version anglaise `book/lessons/part-1/lesson-031-present.md`, révision `e5b6743`.*

<!-- translation-source: book/lessons/part-1/lesson-031-present.md @ e5b6743 -->
