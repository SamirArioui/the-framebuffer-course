# Leçon 040 — des tampons adossés à une réservation

{{#include ../../stability-horizon.md}}

## Prose

Jusqu'ici, le framebuffer vivait là où le chargeur avait bien voulu laisser de
la place — du stockage statique dans le mappage de données de l'exécutable
lui-même. Aujourd'hui, le moteur cesse d'accepter l'emplacement qu'on lui
laisse et se met à *demander* : **une réservation** — des pages entières de
mémoire prises à l'OS exprès, dimensionnées par le moteur, possédées par le
moteur, libérées quand le moteur a fini. C'est le vocabulaire des mappages de
la leçon 039 transformé en API, et c'est désormais là que vivra chaque grand
tampon du moteur.

### Le contrat

```c++
struct Reservation {
    unsigned char *bytes;
    size_t size;   /* whole pages */
    MemoryError error;
};

Reservation ReserveMemory(size_t bytes);
void ReleaseMemory(Reservation &reservation);
```

Trois propriétés, chacune une décision :

- **Des pages entières.** Vous demandez en octets et vous recevez en pages — un
  octet, c'est une page (l'exercice 1 le mesure). La carte se compte en pages,
  la table de pages traduit des pages, et une réservation est un *mappage* :
  prétendre le contraire ne ferait que cacher l'arrondi.
- **Mise à zéro.** Les octets valent zéro avant que quoi que ce soit ne les
  écrive. Pas parce que l'OS met la mémoire à zéro pour la galerie, mais à
  cause du point suivant.
- **Adossée à la demande.** La réservation réserve de l'*espace d'adressage* ;
  les cadres de page physiques n'arrivent qu'au premier contact avec les
  octets — le comportement *demand-zero* (mise à zéro à la demande) que la
  leçon 039 a nommé. Prendre une réservation de 100 Mo ne coûte presque rien ;
  écrire 100 Mo, non.

L'échec est une valeur, comme partout dans la couture : `MEMORY_NO_MEMORY` si
l'OS refuse. Et la libération obéit à une règle de propriété, celle de la leçon
029 dans sa forme la plus simple : `ReleaseMemory` rend les pages et remet la
structure à zéro — libérée une fois, sans danger à rappeler, rien ne pointe
plus vers de la mémoire que l'OS a récupérée.

### Le côté OS

Un seul appel fait toute l'implémentation sur cet OS :

```c++
mmap(0, rounded, PROT_READ | PROT_WRITE,
     MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
```

Lisez-le avec les mots de la leçon 039 : un mappage **anonyme** (aucun fichier
derrière lui) avec la **protection** lecture/écriture, privé pour ce processus,
de `rounded` pages entières, placé là où l'OS a de la place (le `0`). `munmap`
en est la libération. Sur un second OS, le même contrat passe par l'appel de
mappage de cet OS — la couture garde le mot *réservation*, l'implémentation
garde l'idiome.

### Le framebuffer a déménagé

`GetFramebuffer` réserve désormais, au lieu de pointer dans l'exécutable :

```c++
storage = platform::ReserveMemory((size_t)FRAME_WIDTH * FRAME_HEIGHT * 4);
```

et le rapport mémoire de la leçon 039 montre la différence immédiatement :

```
engine: page size 4096 bytes
engine: framebuffer reserved at 0x7566dd4d4000 — 1228800 bytes = 300.00 pages (page-aligned: yes)
```

**Page-aligned: yes** — c'était « no » en stockage statique, quand le tampon
commençait 64 octets après le début d'une page parce que c'est là que l'éditeur
de liens l'avait mis. Une réservation commence là où commence une page, parce
qu'elle *est* des pages. Les mêmes 300,00 pages de données tiennent maintenant
exactement dans 300 pages d'espace d'adressage.

La carte confirme, et le mappage du tampon a maintenant sa propre ligne, avec
sa propre taille :

```
7566dd4d4000-7566dd600000 rw-p 00000000 00:00 0   -> 1228800 bytes
```

Un mappage anonyme en lecture/écriture, exactement de la taille du framebuffer
— aucun fichier, aucun nom, aucun propriétaire partagé. Dans la leçon 041,
c'est dans cette ligne que vivra l'arena.

### À quoi ressemble la mise à zéro à la demande, vue de l'extérieur

L'adossement à la demande se mesure. Une petite sonde (réserver 1,2 Mo,
surveiller l'ensemble résident (RSS) du processus au fil de l'eau) :

```
zeroprobe: rss before 1280 kB, after reserve 1408 kB, after reading 3 bytes 1408 kB, after writing all 2688 kB
zeroprobe: first three bytes read: 0 0 0 (demand-zero: 0 0 0)
```

Prendre la réservation et y lire des octets n'a coûté aucune mémoire physique
digne de ce nom — les cadres n'y ont jamais été. Écrire chaque octet a coûté
les 1,2 Mo complets, d'un coup, en zéros puis en données. (Le petit saut au
moment de la réservation est la comptabilité de l'OS pour le mappage, pas les
pages.) C'est pourquoi une réservation est le bon foyer pour les *grands*
tampons : le moteur peut réserver pour ce dont il pourrait avoir besoin et ne
payer que pour ce qu'il touche.

### Libéré en sortant

Les pages du framebuffer repartent avec le reste de ce que l'exécution a pris :

```c++
platform::CloseWindow(opened.window);
ReleaseFramebuffer();
```

La règle de la leçon 029, qui couvre désormais la mémoire : chaque sortie
libère ce qu'elle a pris. L'OS récupérerait de toute façon la mémoire à la mort
du processus — et une règle qu'on peut vérifier vaut mieux qu'un filet de
sécurité qu'on ne peut qu'espérer.

## Étape de code

Un seul changement pour cette leçon : la couture gagne `Reservation`,
`ReserveMemory` et `ReleaseMemory` (des mappages anonymes de pages entières
derrière l'interface), le framebuffer passe du stockage statique à une
réservation et est libéré à la fin de l'exécution, et le rapport mémoire nomme
la réservation. Son état final est étiqueté `lesson-040`.

```diff
diff --git a/src/framebuffer.cpp b/src/framebuffer.cpp
index 7e6fe13..b8ddcfa 100644
--- a/src/framebuffer.cpp
+++ b/src/framebuffer.cpp
@@ -2,20 +2,36 @@
 //
 // Lesson 030: the same arithmetic Part 0's paint did, on a buffer sized for
 // the window. Offset math, byte order, clipping — nothing else.
+//
+// Lesson 040: the buffer's memory comes from an OS-level reservation, not
+// from static storage and not from an allocator — whole pages, zeroed,
+// released when the engine is done with them.
 
 #include "framebuffer.h"
 
+#include "platform.h"
+
 namespace engine {
 
-/* One screenful of pixels, in static storage. */
-static unsigned char pixels[FRAME_WIDTH * FRAME_HEIGHT * 4];
-static Framebuffer framebuffer = { pixels, FRAME_WIDTH, FRAME_HEIGHT };
+static Framebuffer framebuffer = { 0, FRAME_WIDTH, FRAME_HEIGHT };
+static platform::Reservation storage;
 
 Framebuffer *GetFramebuffer(void)
 {
+    if (!framebuffer.pixels) {
+        storage = platform::ReserveMemory((size_t)FRAME_WIDTH *
+                                          FRAME_HEIGHT * 4);
+        framebuffer.pixels = storage.bytes;
+    }
     return &framebuffer;
 }
 
+void ReleaseFramebuffer(void)
+{
+    platform::ReleaseMemory(storage);
+    framebuffer.pixels = 0;
+}
+
 void ClearBuffer(Framebuffer &fb, unsigned char r, unsigned char g,
                  unsigned char b)
 {
diff --git a/src/framebuffer.h b/src/framebuffer.h
index 1f38723..871fed7 100644
--- a/src/framebuffer.h
+++ b/src/framebuffer.h
@@ -23,11 +23,13 @@ struct Framebuffer {
     int height;
 };
 
-/* The engine's framebuffer. Its bytes live in static storage — the
-   language law of lesson 026 keeps allocation out of the engine, and
-   lesson 040 gives buffers like this a real home. */
+/* The engine's framebuffer. Its bytes come from an OS-level reservation
+   (lesson 040): whole pages, zeroed, released with ReleaseFramebuffer. */
 Framebuffer *GetFramebuffer(void);
 
+/* Gives the framebuffer's pages back to the OS. */
+void ReleaseFramebuffer(void);
+
 /* Fills every pixel with one color. */
 void ClearBuffer(Framebuffer &fb, unsigned char r, unsigned char g,
                  unsigned char b);
diff --git a/src/main.cpp b/src/main.cpp
index 0eedfe1..c345bb4 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -113,12 +113,13 @@ int Run(int argc, char **argv)
     double last = started;
 
     /* The virtual-memory report: every byte the engine owns lives in a
-       mapping the OS keeps, counted in pages. This is what the deep dive
-       explains — the numbers here are measured, not illustrative. */
+       mapping the OS keeps, counted in pages. Since lesson 040 the
+       framebuffer's pages are a reservation — sized in whole pages and
+       aligned like one. */
     size_t page = platform::PageSize();
     size_t fb_bytes = (size_t)fb->width * fb->height * 4;
     std::printf("engine: page size %zu bytes\n", page);
-    std::printf("engine: framebuffer at %p — %zu bytes = %.2f pages (page-aligned: %s)\n",
+    std::printf("engine: framebuffer reserved at %p — %zu bytes = %.2f pages (page-aligned: %s)\n",
                 (void *)fb->pixels, fb_bytes,
                 (double)fb_bytes / (double)page,
                 (size_t)fb->pixels % page == 0 ? "yes" : "no");
@@ -222,6 +223,7 @@ int Run(int argc, char **argv)
     if (platform::CloseRequested(opened.window))
         std::printf("engine: close reported\n");
     platform::CloseWindow(opened.window);
+    ReleaseFramebuffer(); /* the pages go back to the OS (lesson 029's rule) */
     std::printf("engine: closed\n");
     return exit_code;
 }
diff --git a/src/platform.h b/src/platform.h
index 1c9bf6c..29e8dd9 100644
--- a/src/platform.h
+++ b/src/platform.h
@@ -67,6 +67,26 @@ double Now(void);
 /* The OS's memory page: the unit every mapping is counted in. */
 size_t PageSize(void);
 
+/* A memory reservation: whole pages of OS memory, zeroed, owned by the
+   engine until it releases them. This is where engine buffers come from
+   (lesson 040) — not from an allocator. */
+enum MemoryError {
+    MEMORY_OK = 0,
+    MEMORY_NO_MEMORY, /* the OS refused the reservation */
+};
+
+struct Reservation {
+    unsigned char *bytes; /* the reserved bytes, or 0 */
+    size_t size;          /* whole pages */
+    MemoryError error;
+};
+
+/* Reserves at least this many bytes from the OS, rounded to whole pages. */
+Reservation ReserveMemory(size_t bytes);
+
+/* Gives the reservation back to the OS. */
+void ReleaseMemory(Reservation &reservation);
+
 /* A file's complete bytes — or a typed failure. Never partial data
    presented as success. */
 enum FileError {
diff --git a/src/platform_x11.cpp b/src/platform_x11.cpp
index 58c6339..17dae4e 100644
--- a/src/platform_x11.cpp
+++ b/src/platform_x11.cpp
@@ -29,6 +29,7 @@
 #include <poll.h>
 #include <signal.h>
 #include <stdlib.h>
+#include <sys/mman.h>
 #include <sys/stat.h>
 #include <time.h>
 #include <unistd.h>
@@ -226,6 +227,38 @@ size_t PageSize(void)
     return (size_t)sysconf(_SC_PAGESIZE);
 }
 
+/* Reservations are anonymous mappings (lesson 039's vocabulary): whole
+   pages of virtual memory, zeroed by the OS, with no file behind them.
+   Physical frames arrive only when the bytes are touched — the demand-zero
+   behavior the deep dive described. */
+Reservation ReserveMemory(size_t bytes)
+{
+    Reservation reservation = { 0, 0, MEMORY_NO_MEMORY };
+
+    size_t page = PageSize();
+    size_t rounded = (bytes + page - 1) / page * page;
+    if (rounded == 0)
+        rounded = page;
+
+    void *mapping = mmap(0, rounded, PROT_READ | PROT_WRITE,
+                         MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
+    if (mapping == MAP_FAILED)
+        return reservation;
+
+    reservation.bytes = (unsigned char *)mapping;
+    reservation.size = rounded;
+    reservation.error = MEMORY_OK;
+    return reservation;
+}
+
+void ReleaseMemory(Reservation &reservation)
+{
+    if (reservation.bytes)
+        munmap(reservation.bytes, reservation.size);
+    reservation.bytes = 0;
+    reservation.size = 0;
+}
+
 /* File I/O is the OS side too — POSIX here, Win32's own calls in a second
    implementation. The bytes the OS reads for us live in memory the OS
    gives us (its allocator) and leave through ReleaseFile. */
```

## Exercices

Deux extensions « faites-le vôtre ». Chacune se termine par sa solution — un
diff contre l'état final de cette leçon, plus une visite guidée — après
l'énoncé.

### Exercice 1 — Un octet, s'il vous plaît *(predict-the-output)*

Deux prédictions avant de lancer quoi que ce soit : que rapporte
`ReserveMemory(1)` comme **taille** de la réservation, et quels sont les
**premiers octets** d'une mémoire que rien n'a encore écrite ? Ajoutez la plus
petite expérience au démarrage du moteur — réservez un octet, rapportez sa
taille, son adresse et ses trois premiers octets — et accordez le tout.
Qu'est-ce que l'adresse vous dit que la taille seule ne dit pas ?

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-040/ex1.md)

### Exercice 2 — La page qui riposte *(explain-in-prose)*

**Cet exercice fait planter le moteur exprès, une fois** — c'est tout son
propos, et le patch est signalé comme un état pédagogique délibéré. Dotez la
couture de `MakeInaccessible(bytes, size)` — derrière, `mprotect` avec
`PROT_NONE` —, réservez un petit tampon, marquez-le inaccessible et touchez-le.
Regardez l'exécution mourir, puis expliquez, en termes machine, chaque étape
depuis le `mov` jusqu'au `Segmentation fault` : ce que disait la table de
pages, ce que le CPU en a fait, ce que l'OS a fait ensuite, et pourquoi aucune
construction C++ n'aurait pu l'attraper. (Imprimez vos derniers mots sur
`stderr` — un crash emporte le tampon de `stdout` avec lui.)

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-040/ex2.md)

---

**Partie :** [Partie 1 — la couche plateforme](../../index.md) ·
**Précédente :** [Leçon 039 — plongée dans la mémoire virtuelle](lesson-039-virtual-memory.md) ·
**Suivante :** [Leçon 041 — les arenas](lesson-041-arenas.md) ·
**Étiquette de code :** [`lesson-040`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-040)

*Page traduite de la version anglaise `book/lessons/part-1/lesson-040-reservations.md`,
révision `e9826f2`.*

<!-- translation-source: book/lessons/part-1/lesson-040-reservations.md @ e9826f2 -->
