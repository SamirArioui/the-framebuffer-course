# Leçon 043 — la démo de clôture : la couche plateforme terminée

{{#include ../../stability-horizon.md}}

## Prose

Il y a dix-huit leçons, le dépôt avait un `src/` vide et une promesse. Voici à
quoi ressemble la promesse en train de tourner : **une boucle de frame
mesurée** — une fenêtre maintenue en vie, l'entrée scrutée, un framebuffer
adossé à l'arena, présenté, chaque frame chronométrée — qui utilise toutes les
parties de la couche plateforme à la fois. Rien de nouveau n'est inventé
aujourd'hui ; aujourd'hui les pièces *s'emboîtent*, et l'emboîtement est le
sujet. C'est « la couche plateforme terminée ».

### La démo

```
$ DISPLAY=:99 ./build/game
engine: platform layer done — window, polled input, arena-backed framebuffer, measured frames
engine: arrow keys move the marker; close the window to stop
engine: marker at 308,228
engine: marker at 548,228 (t=1.004)
engine: marker at 553,228 (t=1.045)
...
engine: marker at 616,253 (t=1.781)
frame 1: update 0.000 ms, render 0.772 ms, present 1.644 ms, total 2.416 ms
frame 2: update 0.000 ms, render 0.427 ms, present 0.313 ms, total 0.740 ms
...
engine: 41 frames — avg 0.967 ms (update 0.000, render 0.444, present 0.523)
engine: worst frame 2.416 ms (frame 1); present is 54% of the frame
engine: arena: 1228800 of 4194304 bytes used
engine: close reported
engine: closed
```

Lisez l'exécution comme une visite de la partie :

- **La fenêtre** s'ouvre à la taille que le moteur a demandée et reste en vie
  sur la pompe à événements (leçons 027-029). Sa fermeture est signalée, et
  l'exécution se termine avec ses ressources libérées — la fenêtre disparue, la
  connexion fermée, la mémoire rendue à l'OS.
- **L'entrée** déplace le marqueur (leçons 032-034). Des appuis scriptés l'ont
  déplacé vers la droite jusqu'à ce que le bornage l'arrête au bord, puis vers
  le bas — vingt pas, chacun une frame qui a *scruté* l'état plutôt que de
  traiter des événements.
- **L'horloge** a rendu le déplacement honnête (leçons 035-036) : le marqueur
  voyageait à 240 pixels par seconde, à travers des frames qui arrivaient au
  rythme où arrivaient les nouvelles, et le coût de chaque frame était mesuré
  dans l'enregistrement.
- **La mémoire** est celle du moteur (leçons 039-041) : les 1 228 800 octets du
  framebuffer sont sortis de l'arena — ce que montre exactement la dernière
  ligne — et l'arena repose sur une seule réservation d'OS. Une seule
  allocation pour les pixels de toute l'exécution, et une seule libération à la
  fin.
- **La présentation** a mis ces pixels dans la fenêtre, de façon synchrone, à
  chaque frame (leçons 030-031) — et c'est toujours la chose la plus coûteuse
  que le moteur fasse par frame, 54 % de la moyenne.

La vérification par relecture confirme ce que la démo prétend : les pixels du
marqueur sont dans la fenêtre à la position rapportée par le marqueur
(`616,254 .. 638,276` contre un `616,253` rapporté — le coin supérieur gauche du
marqueur). La boucle ne déplace pas une variable ; elle déplace une chose que
vous pouvez voir, à une vitesse qu'elle a choisie, pour un coût qu'elle a
mesuré.

### La forme que la partie 2 hérite

La boucle, ce sont quatre mouvements, et elle restera quatre mouvements :

```
pump  ->  update  ->  render  ->  present
news      state      our bytes   the copy
```

La partie 2 remplace `DrawMarker` par un moteur de rendu et la boucle ne change
pas. La partie 5 mesure les phases de la boucle et l'enregistrement est déjà là.
Les parties 3 et 4 ajoutent des services qui vivent *à côté* de la boucle — son,
assets, entités — et la couche plateforme continue de répondre pour la machine
sous tout cela. Le format du journal de frames (une ligne, un enregistrement)
est la graine que ces parties font grandir.

### Ce que signifie « terminée »

Terminée ne veut pas dire « finie » — le moteur ne fait encore rien. Terminée
veut dire :

- le **contrat** est complet (l'audit de la leçon 042 : quinze fonctions, aucun
  type d'OS dans l'interface, du code de moteur sans OS) ;
- les **scénarios** sont vérifiés (chaque exigence de la spécification a une
  exécution derrière elle — l'exercice 2 les rassemble dans une seule table) ;
- les **coûts** sont mesurés (le cumul des frames, la copie de présentation,
  l'usage de l'arena — rien dans la boucle n'est laissé à la devinette) ;
- et les **limites** sont nommées (les frames arrivent quand les nouvelles
  arrivent ; la copie est le coût le plus lourd ; ASan ne voit pas à l'intérieur
  de l'arena).

Chacun de ces points a été gagné dans une leçon avec une exécution derrière lui.
C'est ce que signifie une couche plateforme terminée — et c'est l'état depuis
lequel la partie 2 démarre.

## Étape de code

Un seul changement pour cette leçon : `main.cpp` devient la démo de clôture —
l'unique boucle de frame mesurée, la mémoire du framebuffer déplacée dans
l'arena du moteur (l'arena la possède désormais ; pas de libération séparée) et
les expériences de démarrage retirées au profit de la démo — et `framebuffer.h`
/ `framebuffer.cpp` accueillent l'allocation adossée à l'arena. Son état final
est étiqueté `lesson-043`.

```diff
diff --git a/src/framebuffer.cpp b/src/framebuffer.cpp
index b8ddcfa..88080d5 100644
--- a/src/framebuffer.cpp
+++ b/src/framebuffer.cpp
@@ -3,35 +3,25 @@
 // Lesson 030: the same arithmetic Part 0's paint did, on a buffer sized for
 // the window. Offset math, byte order, clipping — nothing else.
 //
-// Lesson 040: the buffer's memory comes from an OS-level reservation, not
-// from static storage and not from an allocator — whole pages, zeroed,
-// released when the engine is done with them.
+// Lesson 043: the buffer's memory comes from the engine's arena — one
+// allocation, owned like everything else in the arena, released by
+// releasing the arena.
 
 #include "framebuffer.h"
 
-#include "platform.h"
-
 namespace engine {
 
 static Framebuffer framebuffer = { 0, FRAME_WIDTH, FRAME_HEIGHT };
-static platform::Reservation storage;
 
-Framebuffer *GetFramebuffer(void)
+Framebuffer *GetFramebuffer(Arena &arena)
 {
     if (!framebuffer.pixels) {
-        storage = platform::ReserveMemory((size_t)FRAME_WIDTH *
-                                          FRAME_HEIGHT * 4);
-        framebuffer.pixels = storage.bytes;
+        framebuffer.pixels = (unsigned char *)ArenaAlloc(
+            arena, (size_t)FRAME_WIDTH * FRAME_HEIGHT * 4, 4096);
     }
     return &framebuffer;
 }
 
-void ReleaseFramebuffer(void)
-{
-    platform::ReleaseMemory(storage);
-    framebuffer.pixels = 0;
-}
-
 void ClearBuffer(Framebuffer &fb, unsigned char r, unsigned char g,
                  unsigned char b)
 {
diff --git a/src/framebuffer.h b/src/framebuffer.h
index 871fed7..dd09e45 100644
--- a/src/framebuffer.h
+++ b/src/framebuffer.h
@@ -6,6 +6,8 @@
 #ifndef FRAMEBUFFER_H
 #define FRAMEBUFFER_H
 
+#include "arena.h"
+
 namespace engine {
 
 /* The size the window is opened at (lesson 027) and the size of the
@@ -23,12 +25,10 @@ struct Framebuffer {
     int height;
 };
 
-/* The engine's framebuffer. Its bytes come from an OS-level reservation
-   (lesson 040): whole pages, zeroed, released with ReleaseFramebuffer. */
-Framebuffer *GetFramebuffer(void);
-
-/* Gives the framebuffer's pages back to the OS. */
-void ReleaseFramebuffer(void);
+/* The engine's framebuffer: its bytes are allocated from the caller's
+   arena (lesson 043's shape — the arena owns everything in it, and the
+   framebuffer is no exception). */
+Framebuffer *GetFramebuffer(Arena &arena);
 
 /* Fills every pixel with one color. */
 void ClearBuffer(Framebuffer &fb, unsigned char r, unsigned char g,
diff --git a/src/main.cpp b/src/main.cpp
index 06eba6e..75dfaa6 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -1,9 +1,9 @@
-// main.cpp — the engine, born.
+// main.cpp — the engine: one measured frame loop.
 //
-// Lesson 030: the engine writes its own pixels. The framebuffer is our
-// bytes — Part 0's paint intuition at window size — and Present carries
-// them through the seam. Still no OS headers here; the language law of
-// lesson 026 holds.
+// Lesson 043: the closing demo. The platform layer does its whole job at
+// once — a window kept alive, input polled, an arena-backed framebuffer
+// presented, every frame measured — and this loop is the shape Part 2
+// draws into. The language law of lesson 026 still holds over all of it.
 
 #include <cstdio>
 
@@ -26,13 +26,13 @@ static void DrawMarker(Framebuffer &fb, int x, int y)
             PutPixel(fb, x + i, y + j, 240, 220, 80);
 }
 
-int Run(int argc, char **argv)
+int Run(void)
 {
     platform::WindowResult opened =
         platform::OpenWindow(FRAME_WIDTH, FRAME_HEIGHT);
     if (!opened.window) {
-        /* The error path: nothing was taken that the platform layer did not
-           put back, and the failure is reported by name. */
+        /* The error path: nothing was taken that the platform layer did
+           not put back, and the failure is reported by name. */
         switch (opened.error) {
         case platform::OPEN_NO_DISPLAY:
             std::fprintf(stderr, "engine: no display to open a window on\n");
@@ -48,121 +48,23 @@ int Run(int argc, char **argv)
         return 1;
     }
 
-    /* The clock's contract, checked before anything depends on it: the
-       readings never go backwards, and the finest step between two of them
-       is far below a frame. */
-    double prev = platform::Now();
-    double finest = 1e9;
-    int backwards = 0;
-    for (int i = 0; i < 100000; ++i) {
-        double t = platform::Now();
-        if (t < prev)
-            ++backwards;
-        else if (t > prev && t - prev < finest)
-            finest = t - prev;
-        prev = t;
-    }
-    std::printf("engine: clock %s over 100000 samples, finest step %.0f ns\n",
-                backwards ? "WENT BACKWARDS" : "never backwards", finest * 1e9);
-
-    /* Whole-file writes and reads: bytes leave the engine, come back, and
-       had better be the same bytes — or the failure is the answer. */
-    if (argc > 1) {
-        unsigned char payload[256];
-        for (int i = 0; i < (int)sizeof payload; ++i)
-            payload[i] = (unsigned char)(i * 7); /* a pattern, byte by byte */
-
-        platform::FileError wrote =
-            platform::WriteFile(argv[1], payload, sizeof payload);
-        if (wrote != platform::FILE_OK) {
-            std::printf("engine: %s: could not write\n", argv[1]);
-            platform::CloseWindow(opened.window);
-            return 1;
-        }
-        std::printf("engine: wrote %s: %zu bytes\n", argv[1], sizeof payload);
-
-        platform::FileData file = platform::ReadFile(argv[1]);
-        if (file.error != platform::FILE_OK) {
-            std::printf("engine: %s: could not read back\n", argv[1]);
-            platform::CloseWindow(opened.window);
-            return 1;
-        }
-
-        long mismatch = -1;
-        size_t checked = file.size < sizeof payload ? file.size : sizeof payload;
-        for (size_t i = 0; i < checked; ++i)
-            if (file.data[i] != payload[i]) {
-                mismatch = (long)i;
-                break;
-            }
-        if (file.size == sizeof payload && mismatch < 0) {
-            std::printf("engine: round-trip ok: %zu bytes match\n", file.size);
-        } else {
-            std::printf("engine: round-trip FAILED: %zu bytes back (wanted %zu), first mismatch %ld\n",
-                        file.size, sizeof payload, mismatch);
-        }
-        platform::ReleaseFile(file);
-    }
+    /* The engine's memory: one arena over one reservation. Everything the
+       engine allocates lives in here and is released together. */
+    Arena arena;
+    ArenaInit(arena, 4 * 1024 * 1024);
+    Framebuffer *fb = GetFramebuffer(arena);
 
-    /* The scene: a marker the arrow keys move. The report below is its
-       position and the time it moved — the interactive frame makes itself
-       observable. */
-    Framebuffer *fb = GetFramebuffer();
     double marker_x = (FRAME_WIDTH - MARKER_SIZE) / 2.0;
     double marker_y = (FRAME_HEIGHT - MARKER_SIZE) / 2.0;
     double started = platform::Now();
     double last = started;
 
-    /* The virtual-memory report: every byte the engine owns lives in a
-       mapping the OS keeps, counted in pages. Since lesson 040 the
-       framebuffer's pages are a reservation — sized in whole pages and
-       aligned like one. */
-    size_t page = platform::PageSize();
-    size_t fb_bytes = (size_t)fb->width * fb->height * 4;
-    std::printf("engine: page size %zu bytes\n", page);
-    std::printf("engine: framebuffer reserved at %p — %zu bytes = %.2f pages (page-aligned: %s)\n",
-                (void *)fb->pixels, fb_bytes,
-                (double)fb_bytes / (double)page,
-                (size_t)fb->pixels % page == 0 ? "yes" : "no");
-
+    std::printf("engine: platform layer done — window, polled input, arena-backed framebuffer, measured frames\n");
     std::printf("engine: arrow keys move the marker; close the window to stop\n");
     std::printf("engine: marker at %d,%d\n", (int)marker_x, (int)marker_y);
 
-    /* The arena: bump allocation over a reservation. This report is the
-       allocator's behavior — the same numbers Part 4's services will rely
-       on. */
-    Arena arena;
-    ArenaInit(arena, 65536);
-    std::printf("engine: arena over %zu bytes (%zu pages)\n",
-                arena.memory.size, arena.memory.size / page);
-
-    void *a = ArenaAlloc(arena, 100, 16);
-    std::printf("engine: alloc 100 (align 16) -> offset %ld, used %zu\n",
-                (long)((unsigned char *)a - arena.memory.bytes), arena.used);
-    void *b = ArenaAlloc(arena, 50, 32);
-    std::printf("engine: alloc 50 (align 32) -> offset %ld, used %zu\n",
-                (long)((unsigned char *)b - arena.memory.bytes), arena.used);
-
-    size_t mark = ArenaMark(arena);
-    std::printf("engine: mark at %zu\n", mark);
-    void *c = ArenaAlloc(arena, 3, 1);
-    std::printf("engine: alloc 3 (align 1) -> offset %ld, used %zu\n",
-                (long)((unsigned char *)c - arena.memory.bytes), arena.used);
-    ArenaRollback(arena, mark);
-    std::printf("engine: rollback to %zu — used %zu\n", mark, arena.used);
-    void *d = ArenaAlloc(arena, 3, 1);
-    std::printf("engine: alloc 3 (align 1) -> offset %ld, used %zu\n",
-                (long)((unsigned char *)d - arena.memory.bytes), arena.used);
-
-    void *e = ArenaAlloc(arena, 999999, 16);
-    std::printf("engine: exhausted: alloc 999999 -> %s\n",
-                e ? "handed out (!)" : "0 (as it should be)");
-    ArenaRelease(arena);
-
-    /* The frame step: read news, update from polled state, draw, present.
-       This is the shape every later part fills in — Part 2 draws into it,
-       Part 5 measures it. Every phase is now measured: the frame record is
-       data, not guesswork. */
+    /* The frame step: read news, update from polled state, draw, present —
+       every phase measured, one record per frame. */
     int exit_code = 0;
     long frame_number = 0;
     FrameStats stats = {};
@@ -175,9 +77,7 @@ int Run(int argc, char **argv)
         frame.number = ++frame_number;
         double t0 = platform::Now();
 
-        /* Update: a frame reads state — it never handles events. The step
-           is speed × elapsed: the marker moves 240 pixels per second no
-           matter how often frames happen. */
+        /* Update: a frame reads state — it never handles events. */
         double now = platform::Now();
         double dt = now - last;
         last = now;
@@ -232,8 +132,7 @@ int Run(int argc, char **argv)
         frame.total = platform::Now() - t0;
         AccountFrame(stats, frame);
 
-        /* The frame log: one line per record. This is the format Part 2
-           grows and Part 5's frame-budget report reads. */
+        /* The frame log: one line per record — the format Part 2 grows. */
         std::printf("frame %ld: update %.3f ms, render %.3f ms, present %.3f ms, total %.3f ms\n",
                     frame.number, frame.update * 1e3, frame.render * 1e3,
                     frame.present * 1e3, frame.total * 1e3);
@@ -251,18 +150,20 @@ int Run(int argc, char **argv)
                     stats.worst * 1e3, stats.worst_number,
                     100.0 * stats.present_sum / stats.total_sum);
     }
+    std::printf("engine: arena: %zu of %zu bytes used\n", arena.used,
+                arena.memory.size);
 
     if (platform::CloseRequested(opened.window))
         std::printf("engine: close reported\n");
     platform::CloseWindow(opened.window);
-    ReleaseFramebuffer(); /* the pages go back to the OS (lesson 029's rule) */
+    ArenaRelease(arena);
     std::printf("engine: closed\n");
     return exit_code;
 }
 
 } /* namespace engine */
 
-int main(int argc, char **argv)
+int main(void)
 {
-    return engine::Run(argc, argv);
+    return engine::Run();
 }
```

## Exercices

Deux extensions « faites-le vôtre ». Chacune se termine par sa solution — un
diff contre l'état final de cette leçon, plus une visite guidée — après
l'énoncé.

### Exercice 1 — La couche plateforme terminée, sur votre machine *(port-to-your-own-machine)*

Les nombres du livre viennent de Xvfb sur la machine de l'auteur ; les vôtres
doivent venir de votre bureau. Faites grandir le cumul d'une ligne — combien de
temps l'exécution a duré et combien de frames par seconde cela donne — puis
lancez la démo sur votre propre machine et comparez les formes : ce qui a bougé
entre les machines (le coût du present ? le rythme ?), ce qui n'a pas bougé
(l'update ?), et ce que les différences disent de la provenance des nombres.
Consignez la machine avec les nombres ; une mesure sans sa machine est une
rumeur.

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-043/ex1.md)

### Exercice 2 — La table d'acceptation *(explain-in-prose)*

« La couche plateforme terminée » est une affirmation, et une affirmation veut
une table. Faites en sorte que la démo nomme le contrat qu'elle utilise — une
ligne par groupe : fenêtre et présentation, entrée scrutée, horloge monotone,
E/S de fichiers entiers, réservations et arena. Remplissez ensuite la table
d'acceptation : pour chaque scénario de la spécification plateforme, la leçon
qui l'a construit et les *preuves* tirées de votre propre exécution (la commande
et ce qu'elle a imprimé). Terminez par la question qui rend la table digne
d'être conservée : quelles lignes cassent en premier quand le moteur change, et
comment le remarqueriez-vous ?

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-043/ex2.md)

---

**Partie :** [Partie 1 — la couche plateforme](../../index.md) ·
**Précédente :** [Leçon 042 — l'interface comme contrat](lesson-042-contract.md) ·
**Suivante :** — ·
**Étiquette de code :** [`lesson-043`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-043)

*Page traduite de la version anglaise `book/lessons/part-1/lesson-043-demo.md`,
révision `5ec1553`.*

<!-- translation-source: book/lessons/part-1/lesson-043-demo.md @ 5ec1553 -->
