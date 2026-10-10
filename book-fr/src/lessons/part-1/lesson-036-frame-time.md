# Leçon 036 — le temps de frame comme donnée mesurée

{{#include ../../stability-horizon.md}}

## Prose

Le moteur possède désormais ses pixels, sa saisie et son temps. Aujourd'hui, il
commence à tenir des **enregistrements** : ce que chaque frame a coûté, mesuré
— pas estimé — sur l'horloge de la plateforme. C'est la graine de deux choses
que le cours va cultiver : l'instrumentation du temps de frame de la partie 2
et le rapport de budget de frames de la partie 5 liront tous deux
l'enregistrement que cette leçon définit. Et le premier nombre qu'il rapporte
mérite une franchise précoce : **la copie de présentation est du vrai travail,
et elle a lieu à chaque frame**.

### L'enregistrement

```c++
struct FrameRecord {
    long number;
    double update;  /* reading state, moving the world */
    double render;  /* drawing the scene into the framebuffer */
    double present; /* the copy to the window, sync included */
    double total;   /* the whole frame step */
};
```

Un enregistrement par frame, quatre chronomètres. La discipline de mesure est
celle que la leçon 020 appliquait à la boucle de `snek` : lire l'horloge avant
une phase, la lire après, soustraire. `Present` est mesuré *avec* son `XSync`
— et c'est ce qui rend le nombre honnête : le contrat de la leçon 031 dit que
les pixels sont à l'écran quand `Present` rend la main, donc la mesure s'arrête
quand le travail s'arrête. Un present asynchrone ne pourrait jamais être
chronométré autrement que comme « le temps de confier le travail à quelqu'un
d'autre ».

Les champs de l'enregistrement sont la forme de la frame devenue donnée.
`frame.h` porte le format — une ligne du journal est un enregistrement :

```
frame 30: update 0.000 ms, render 0.470 ms, present 0.518 ms, total 0.988 ms
```

La partie 2 étoffe l'enregistrement de ses propres phases (elle voudra savoir
ce que le *moteur de rendu* a coûté, séparément de la copie de la plateforme) ;
la partie 5 agrège les enregistrements en un rapport de budget. Ni l'une ni
l'autre n'aura à changer ce qu'est un enregistrement — seulement ce que le
moteur y met.

### Ce que les frames coûtent réellement

Une session en mouvement — le marqueur piloté à travers l'écran pendant
quelques secondes — a mesuré 101 frames :

```
frame 1:  update 0.000 ms, render 0.809 ms, present 0.664 ms, total 1.474 ms
frame 2:  update 0.000 ms, render 0.446 ms, present 1.395 ms, total 1.842 ms
frame 30: update 0.000 ms, render 0.470 ms, present 0.518 ms, total 0.988 ms
frame 60: update 0.000 ms, render 0.435 ms, present 0.359 ms, total 0.794 ms
...
engine: 101 frames — avg 1.015 ms (update 0.000, render 0.462, present 0.552)
engine: worst frame 1.842 ms (frame 2); present is 54% of the frame
```

Lisez ces nombres pour ce qu'ils sont — des mesures sur *cette* machine, sous
Xvfb, avec un seul carré à l'écran — et trois choses méritent encore d'être
dites :

- **Le present représente 54 % de la frame.** La copie vers la fenêtre est de
  loin la plus grosse chose que le moteur fait par frame, et ce n'est même pas
  du code du moteur — c'est le prix des pixels qui quittent notre processus.
  L'exercice 1 le sépare en copie et attente.
- **Le rendu propre du moteur n'est pas gratuit.** `ClearBuffer` sur 307 200
  pixels plus le marqueur coûte à peu près autant que la copie. « Nous
  possédons nos pixels » veut dire que nous possédons aussi leur coût — la
  partie 2 rendra ce coût intéressant.
- **La pire frame est au début.** La frame 1 est lente (les premiers contacts
  avec un tampon de 1,2 Mo ne sont pas gratuits — la leçon 039 expliquera
  pourquoi), et la pire frame ici vaut 1,8 ms contre une moyenne de 1,0 ms. Un
  rapport de budget de frames existe exactement pour cette queue de
  distribution, pas pour la moyenne.

Et la mise en garde honnête, qui fait partie de la mesure : Xvfb est un
affichage virtuel — pas de compositeur, pas d'autres clients, pas de vrai
écran. Les nombres d'un bureau varient. La *forme* — un present qui pèse une
vraie part de chaque frame — ne varie pas.

### Pourquoi mesurer maintenant

Parce que tout ce qui suit voudra ces nombres et ne pourra pas les rattraper
après coup. Le moteur de rendu de la partie 2 doit savoir ce qu'il coûte
*avant* que les passes d'optimisation de la partie 5 n'en discutent. Le rapport
de budget de frames à la fin du cours n'est crédible que si la mesure a
commencé ici et n'a jamais cessé. (C'est la règle « le profileur précède
l'optimisation » du programme de la chaîne d'outils, appliquée d'abord à notre
propre moteur.)

## Étape de code

Un seul changement pour cette leçon : `frame.h` et `frame.cpp` gagnent
l'enregistrement par frame et son cumul, `main.cpp` mesure chaque phase de
chaque frame et journalise une ligne par enregistrement, et l'exécution se
termine par le cumul — y compris la part de la copie de présentation dans la
frame. Son état final est étiqueté `lesson-036`.

```diff
diff --git a/src/frame.cpp b/src/frame.cpp
new file mode 100644
index 0000000..d5e2920
--- /dev/null
+++ b/src/frame.cpp
@@ -0,0 +1,22 @@
+// frame.cpp — the frame account: sums, worst case, nothing else.
+//
+// Lesson 036: measured data is just data — this file does arithmetic on it.
+
+#include "frame.h"
+
+namespace engine {
+
+void AccountFrame(FrameStats &stats, const FrameRecord &frame)
+{
+    stats.frames += 1;
+    stats.update_sum += frame.update;
+    stats.render_sum += frame.render;
+    stats.present_sum += frame.present;
+    stats.total_sum += frame.total;
+    if (frame.total > stats.worst) {
+        stats.worst = frame.total;
+        stats.worst_number = frame.number;
+    }
+}
+
+} /* namespace engine */
diff --git a/src/frame.h b/src/frame.h
new file mode 100644
index 0000000..3ca1cdb
--- /dev/null
+++ b/src/frame.h
@@ -0,0 +1,37 @@
+// frame.h — the per-frame record: what a frame cost, measured on the
+// platform clock.
+//
+// Lesson 036: frame time as measured data. The engine does not guess what
+// its frames cost — it measures each phase and keeps the numbers. Part 2
+// grows this record with its own phases; Part 5's frame-budget report
+// reads it. The format of one line of the log is the format of one record.
+#ifndef FRAME_H
+#define FRAME_H
+
+namespace engine {
+
+/* Seconds, each field: how long one frame's phase took. */
+struct FrameRecord {
+    long number;   /* the frame's count since the run started */
+    double update; /* reading state, moving the world */
+    double render; /* drawing the scene into the framebuffer */
+    double present;/* the copy to the window, sync included */
+    double total;  /* the whole frame step */
+};
+
+/* The running account: every frame measured so far. */
+struct FrameStats {
+    long frames;
+    double update_sum;
+    double render_sum;
+    double present_sum;
+    double total_sum;
+    double worst;      /* the longest frame so far */
+    long worst_number; /* and which one it was */
+};
+
+void AccountFrame(FrameStats &stats, const FrameRecord &frame);
+
+} /* namespace engine */
+
+#endif
diff --git a/src/main.cpp b/src/main.cpp
index c66a75f..942d825 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -8,6 +8,7 @@
 #include <cstdio>
 
 #include "framebuffer.h"
+#include "frame.h"
 #include "platform.h"
 
 namespace engine {
@@ -76,13 +77,20 @@ int Run(void)
 
     /* The frame step: read news, update from polled state, draw, present.
        This is the shape every later part fills in — Part 2 draws into it,
-       Part 5 measures it. */
+       Part 5 measures it. Every phase is now measured: the frame record is
+       data, not guesswork. */
     int exit_code = 0;
+    long frame_number = 0;
+    FrameStats stats = {};
     while (!platform::CloseRequested(opened.window)) {
         platform::PumpEvents(opened.window);
         if (platform::CloseRequested(opened.window))
             break;
 
+        FrameRecord frame;
+        frame.number = ++frame_number;
+        double t0 = platform::Now();
+
         /* Update: a frame reads state — it never handles events. The step
            is speed × elapsed: the marker moves 240 pixels per second no
            matter how often frames happen. */
@@ -110,6 +118,9 @@ int Run(void)
         if (marker_y > FRAME_HEIGHT - MARKER_SIZE)
             marker_y = FRAME_HEIGHT - MARKER_SIZE;
 
+        frame.update = platform::Now() - t0;
+        double t1 = platform::Now();
+
         if ((int)marker_x != old_x || (int)marker_y != old_y)
             std::printf("engine: marker at %d,%d (t=%.3f)\n", (int)marker_x,
                         (int)marker_y, platform::Now() - started);
@@ -118,6 +129,9 @@ int Run(void)
         ClearBuffer(*fb, 32, 32, 64);
         DrawMarker(*fb, (int)marker_x, (int)marker_y);
 
+        frame.render = platform::Now() - t1;
+        double t2 = platform::Now();
+
         if (!platform::Present(opened.window, fb->pixels, fb->width,
                                fb->height)) {
             /* A present can fail because the window died mid-copy — that
@@ -129,6 +143,29 @@ int Run(void)
             exit_code = 1;
             break;
         }
+
+        frame.present = platform::Now() - t2;
+        frame.total = platform::Now() - t0;
+        AccountFrame(stats, frame);
+
+        /* The frame log: one line per record. This is the format Part 2
+           grows and Part 5's frame-budget report reads. */
+        std::printf("frame %ld: update %.3f ms, render %.3f ms, present %.3f ms, total %.3f ms\n",
+                    frame.number, frame.update * 1e3, frame.render * 1e3,
+                    frame.present * 1e3, frame.total * 1e3);
+    }
+
+    /* The account: what the frames actually cost, including the honest
+       price of the presentation copy. */
+    if (stats.frames) {
+        double n = (double)stats.frames;
+        std::printf("engine: %ld frames — avg %.3f ms (update %.3f, render %.3f, present %.3f)\n",
+                    stats.frames, stats.total_sum / n * 1e3,
+                    stats.update_sum / n * 1e3, stats.render_sum / n * 1e3,
+                    stats.present_sum / n * 1e3);
+        std::printf("engine: worst frame %.3f ms (frame %ld); present is %.0f%% of the frame\n",
+                    stats.worst * 1e3, stats.worst_number,
+                    100.0 * stats.present_sum / stats.total_sum);
     }
 
     if (platform::CloseRequested(opened.window))
```

## Exercices

Deux extensions « faites-le vôtre ». Chacune se termine par sa solution — un
diff contre l'état final de cette leçon, plus une visite guidée — après
l'énoncé.

### Exercice 1 — La copie, isolée *(measure-the-performance)*

Le temps mesuré de `Present` colle ensemble deux choses différentes :
`XPutImage` — pousser les pixels vers le serveur — et `XSync` — attendre que
le serveur ait fini. Instrumentez les deux moitiés séparément (deux
chronomètres dans l'implémentation, une ligne par present) et découvrez quelle
moitié est la copie et quelle moitié est l'aller-retour. Répondez ensuite à la
question que la conception garde ouverte : quelle moitié la mémoire partagée
(MIT-SHM) supprimerait-elle, et que resterait-il ?

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-036/ex1.md)

### Exercice 2 — Le budget de frames *(extend-the-code)*

Une mesure ne devient une décision que lorsqu'il existe une ligne dont on peut
se retrouver du mauvais côté. Donnez au cumul un **budget de frames** — la
ligne des 60 fps est 1/60 = 16,7 ms — et faites rapporter à l'exécution
combien de frames tiennent dedans : `within the 16.7 ms budget: N/M frames`.
Où le budget doit-il vivre : dans la couche plateforme ou dans le code
d'enregistrement du moteur ? Répondez avant de le placer, et laissez la
réponse décider du fichier.

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-036/ex2.md)

---

**Partie :** [Partie 1 — la couche plateforme](../../index.md) ·
**Précédente :** [Leçon 035 — l'horloge de la plateforme](lesson-035-clock.md) ·
**Suivante :** [Leçon 037 — lectures de fichier entier](lesson-037-file-read.md) ·
**Étiquette de code :** [`lesson-036`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-036)

*Page traduite de la version anglaise `book/lessons/part-1/lesson-036-frame-time.md`,
révision `5342420`.*

<!-- translation-source: book/lessons/part-1/lesson-036-frame-time.md @ 5342420 -->
