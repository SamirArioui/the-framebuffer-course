# Leçon 029 — fermeture propre et chemins d'erreur : les ressources de l'OS libérées à chaque sortie

{{#include ../../stability-horizon.md}}

## Prose

Une fenêtre n'est pas la seule chose qu'une exécution peut laisser derrière
elle. La leçon 028 a donné au moteur une fin propre — l'utilisateur a fermé la
fenêtre — mais une exécution peut se terminer autrement : l'OS peut refuser
tout bonnement de donner une fenêtre, la fenêtre peut être détruite sous les
pieds de l'exécution, et l'utilisateur peut l'interrompre. Chacune de ces fins est un
**chemin de sortie**, et chaque chemin de sortie doit la même dette : tout ce
qui a été acquis auprès de l'OS est libéré, exactement une fois. Aujourd'hui,
cela devient une règle que le code fait respecter et que les exécutions
peuvent prouver.

### La règle

> **Chaque ressource de l'OS est acquise exactement une fois et libérée
> exactement une fois, sur chaque chemin de sortie.**

Pas « l'OS le récupère à la mort du processus » — c'est vrai, et ce n'est pas
la règle. La mort du processus est un filet de sécurité, pas de la
comptabilité : une exécution qui fuit une connexion au display à chaque frame
survivrait des heures sous la comptabilité « mort du processus » et serait
quand même cassée. La règle est ce qui rend le comportement du programme
*vérifiable*, et c'est la même discipline dont le moteur aura besoin quand les
ressources seront plus grosses qu'une fenêtre — les réservations de mémoire et
les arenas qui arrivent plus loin dans cette partie.

### Les chemins d'erreur

`OpenWindow` peut échouer de deux façons, et les deux pannes se situent à des
points différents de la séquence d'acquisitions — c'est pourquoi la règle
compte le plus ici même :

- **`OPEN_NO_DISPLAY`** — la connexion au display de l'OS n'a pas pu être
  ouverte. Cette panne survient *avant toute acquisition* : il n'y a rien à
  libérer, et rien n'a été touché.
- **`OPEN_NO_WINDOW`** — le display s'est connecté mais l'OS a refusé la
  fenêtre. Cette panne survient *après une acquisition* : la connexion au
  display est déjà à nous, et l'implémentation la remet en place avant de
  rapporter la panne. Les états à moitié ouverts sont libérés en sortant, pas
  ramenés à la maison.

Le chemin d'erreur du moteur rapporte la panne **par son nom** — un `switch`
sur l'erreur typée, chaque cas son propre message — et sort en code non nul
sans toucher de fenêtre, puisqu'il n'y en a aucune à toucher. Les deux pannes
sont vérifiables sans écran, parce que toutes deux concernent le display :

```
$ env -u DISPLAY ./build/game
engine: no display to open a window on
$ echo $?
1
$ DISPLAY=:77 ./build/game
engine: no display to open a window on
$ echo $?
1
```

(`:77` est un display sans serveur à l'écoute — même panne, un cran plus
loin.) Le cas `OPEN_NO_WINDOW` est la seconde raison du contrat ; une
occurrence réelle est rare sous X11, ce qui est exactement pourquoi l'erreur
typée la nomme au lieu d'espérer.

### La sortie interrompue

Il reste une façon de terminer une exécution que la leçon 028 ne pouvait pas
gérer du tout : Ctrl+C. L'action par défaut d'une interruption est de tuer le
processus sur place — pas de `CloseWindow`, pas de rapport, rien que le filet
de sécurité de l'OS. Et la pompe de la leçon 028 ne peut pas entendre le
signal, même quand un gestionnaire l'attrape : l'exécution est endormie *à
l'intérieur* de `XNextEvent`, qui ne se réveille pas pour les signaux. Une
sonde qui bloque là et compte les signaux reste bloquée :

```
sigprobe: blocked in XNextEvent
STILL BLOCKED after SIGINT
```

La correction est d'attendre là où les signaux peuvent interrompre. `poll`
sur la connexion au display de l'OS fait exactement cela — il revient quand il
y a des nouvelles *ou* quand un signal l'interrompt avec `EINTR` — et c'est le
même `poll` que la leçon 021 utilisait sur le terminal :

```c++
struct pollfd pfd = { ConnectionNumber(window->display), POLLIN, 0 };
poll(&pfd, 1, -1);

if (interrupted)
    window->close_requested = true;
```

Le gestionnaire lui-même fait la seule chose qu'un gestionnaire de signal peut
faire sans danger — poser un drapeau `volatile sig_atomic_t`, la règle de la
leçon 020 — et la pompe plie le drapeau dans l'état comme n'importe quelle
autre nouvelle. **Chaque façon dont l'exécution peut se terminer rapporte à
travers `CloseRequested`** : la demande de l'utilisateur, l'acte de l'OS, et
l'interruption font passer le même drapeau à vrai, parce que la réaction du
moteur est la même pour les trois — s'arrêter, et libérer. Le gestionnaire est
lui-même une ressource : il est installé à l'ouverture de la fenêtre et
restauré à sa valeur par défaut à la fermeture.

### Exactement une fois, en sortant

`CloseWindow` ne libère que ce qui existe encore. Si l'OS a déjà détruit la
fenêtre (`DestroyNotify`), le pliage a remis l'id à zéro — la détruire à
nouveau serait une erreur X — et seule la connexion au display reste à
libérer. Si la fenêtre est encore à nous, elle est détruite d'abord, puis la
connexion est fermée. Dans un cas comme dans l'autre : une libération par
acquisition, et le gestionnaire d'interruption repasse à la valeur par défaut
sur le même chemin.

L'exécution interrompue se termine comme n'importe quelle autre :

```
$ DISPLAY=:99 ./build/game &
$ kill -INT %1
engine: window 640x480 open — waiting for news
engine: close reported
engine: closed
$ DISPLAY=:99 xdotool search --name "the framebuffer engine"
$ 
```

Fenêtre disparue, connexion fermée, code de sortie 0 — une interruption est
une fin, pas un accident.

## Étape de code

Un seul changement pour cette leçon : l'interruption devient une nouvelle (un
drapeau `sig_atomic_t` plié dans l'état par la pompe, qui attend désormais dans
`poll` là où les signaux peuvent la réveiller), `CloseWindow` libère exactement
ce qu'il a acquis — gestionnaire compris — et le chemin d'erreur du moteur
rapporte chaque panne typée par son nom. Son état final est étiqueté
`lesson-029`.

```diff
diff --git a/src/main.cpp b/src/main.cpp
index 1fa92f3..efb0b34 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -19,8 +19,20 @@ int Run(void)
     platform::WindowResult opened =
         platform::OpenWindow(WINDOW_WIDTH, WINDOW_HEIGHT);
     if (!opened.window) {
-        std::fprintf(stderr, "engine: no window (platform error %d)\n",
-                     opened.error);
+        /* The error path: nothing was taken that the platform layer did not
+           put back, and the failure is reported by name. */
+        switch (opened.error) {
+        case platform::OPEN_NO_DISPLAY:
+            std::fprintf(stderr, "engine: no display to open a window on\n");
+            break;
+        case platform::OPEN_NO_WINDOW:
+            std::fprintf(stderr, "engine: the OS refused the window\n");
+            break;
+        default:
+            std::fprintf(stderr, "engine: platform error %d\n",
+                         opened.error);
+            break;
+        }
         return 1;
     }
 
diff --git a/src/platform.h b/src/platform.h
index d6aa25e..da3e0f1 100644
--- a/src/platform.h
+++ b/src/platform.h
@@ -31,10 +31,12 @@ WindowResult OpenWindow(int width, int height);
 
 /* Reads whatever news the OS has about this window and folds it into the
    platform layer's state. The engine never sees an event object — it polls
-   state afterwards. Blocks until there is news. */
+   state afterwards. Blocks until there is news or the run is interrupted. */
 void PumpEvents(Window *window);
 
-/* True once the user has asked for this window to close. */
+/* True once the user has asked for this window to close. An interrupted run
+   counts: every way the run can end reports here, so the engine has exactly
+   one ending to get right. */
 bool CloseRequested(const Window *window);
 
 /* Releases everything OpenWindow took from the OS. */
diff --git a/src/platform_x11.cpp b/src/platform_x11.cpp
index 68de824..0438d3f 100644
--- a/src/platform_x11.cpp
+++ b/src/platform_x11.cpp
@@ -12,6 +12,9 @@
 
 #include <X11/Xlib.h>
 
+#include <poll.h>
+#include <signal.h>
+
 namespace platform {
 
 /* What a window is made of on this OS. The definition lives here, where
@@ -35,6 +38,16 @@ static Window window_state;
    server hands out as integers; asking for them is how you spell them. */
 static Atom wm_delete_window;
 
+/* The interrupt: Ctrl+C is an exit too, and the run owes it the same clean
+   close as any other. The handler does the only thing a signal handler may
+   safely do here — set a flag (lesson 020's rule). */
+static volatile sig_atomic_t interrupted;
+
+static void OnInterrupt(int)
+{
+    interrupted = 1;
+}
+
 WindowResult OpenWindow(int width, int height)
 {
     WindowResult result = { &window_state, OPEN_NO_DISPLAY };
@@ -70,6 +83,11 @@ WindowResult OpenWindow(int width, int height)
     window_state.display = display;
     window_state.xwindow = xwindow;
     window_state.close_requested = false;
+
+    /* The interrupt is part of the window's take: from here on, Ctrl+C is
+       news like any other. */
+    interrupted = 0;
+    signal(SIGINT, OnInterrupt);
     return result;
 }
 
@@ -91,12 +109,19 @@ void PumpEvents(Window *window)
     if (!window || !window->display)
         return;
 
-    /* Block for the first piece of news, then drain whatever else piled up.
-       Blocking is the point: the engine waits here instead of spinning. */
-    XEvent event;
-    XNextEvent(window->display, &event);
-    HandleEvent(window, event);
+    /* Wait for news where a signal can wake us. Lesson 028 slept inside
+       XNextEvent, where Ctrl+C could not reach it; poll on the OS
+       connection returns when there is news *or* when a signal interrupts
+       it — then the flag below is folded in like any other news. */
+    struct pollfd pfd = { ConnectionNumber(window->display), POLLIN, 0 };
+    poll(&pfd, 1, -1);
+
+    if (interrupted)
+        window->close_requested = true;
+
+    /* Drain whatever piled up: one blocking wait, then the whole batch. */
     while (XPending(window->display)) {
+        XEvent event;
         XNextEvent(window->display, &event);
         HandleEvent(window, event);
     }
@@ -117,6 +142,7 @@ void CloseWindow(Window *window)
     if (window->xwindow)
         XDestroyWindow(window->display, window->xwindow);
     XCloseDisplay(window->display);
+    signal(SIGINT, SIG_DFL); /* the handler is taken and released like the rest */
     window->display = 0;
     window->xwindow = 0;
 }
```

## Exercices

Deux extensions « faites-le vôtre ». Chacune se termine par sa solution — un
diff contre l'état final de cette leçon, plus une visite guidée — après
l'énoncé.

### Exercice 1 — La fenêtre qu'on ouvre deux fois *(fix-the-crash)*

Le code d'un coéquipier appelle `OpenWindow` une seconde fois alors que la
première fenêtre est encore ouverte. Rien ne plante — c'est là le bug. La
couture redonne le même bloc d'état : le handle de la première connexion au
display est écrasé et fuit, et les deux fenêtres partagent un seul état.
Prouvez le bug d'abord (un second appel, et observez ce qui arrive à la
première fenêtre), puis corrigez-le pour que la couture ne possède qu'une
fenêtre à la fois : la seconde ouverture est une panne typée que le moteur
peut nommer, la première fenêtre continue de fonctionner, et une fermeture
libère toujours exactement une acquisition.

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-029/ex1.md)

### Exercice 2 — Le registre *(predict-the-output)*

Tenez un registre sur la couche plateforme : chaque ressource de l'OS qu'elle
acquiert reçoit une ligne (`take display`, `take window`,
`take signal handler`), chaque libération aussi (`release window`,
`release display`, `release signal handler`), imprimées sur stderr au moment
où elles se produisent. Avant de lancer quoi que ce soit, *prédisez* le
registre pour chacune des quatre sorties — la demande de fermeture de
l'utilisateur, la fenêtre détruite, Ctrl+C, et le display manquant. Puis
produisez les quatre et réconciliez. Quel bilan de chemin vous a surpris, et
qui a effectué la libération qui vous a surpris ?

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-029/ex2.md)

---

**Partie :** [Partie 1 — la couche plateforme](../../index.md) ·
**Précédente :** [Leçon 028 — la pompe à événements : garder la fenêtre en vie et signaler la fermeture](lesson-028-event-pump.md) ·
**Suivante :** [Leçon 030 — le framebuffer comme nos propres octets](lesson-030-framebuffer.md) ·
**Étiquette de code :** [`lesson-029`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-029)

*Page traduite de la version anglaise `book/lessons/part-1/lesson-029-clean-close.md`, révision `71a2428`.*

<!-- translation-source: book/lessons/part-1/lesson-029-clean-close.md @ 71a2428 -->
