# Leçon 037 — lectures de fichier entier

{{#include ../../stability-horizon.md}}

## Prose

Le moteur sait voir, se déplacer et mesurer — et il ne sait pas encore
*charger*. Chaque ressource dont le jeu aura jamais besoin est un fichier, et
le contrat pour en faire entrer un en mémoire est la promesse la plus étroite
de la couche plateforme :

> **Une lecture renvoie les octets complets d'un fichier ou un échec typé —
> jamais des données partielles présentées comme un succès.**

La partie 0 lisait des fichiers avec `fopen`/`fread` dans les programmes du
bac à sable (la première leçon de `wordcount`). Le moteur, lui, lit **contre
l'OS** : les appels open-read-close de l'OS lui-même, derrière la couture, là
où un second OS substitue les siens.

### Le contrat

```c++
enum FileError {
    FILE_OK = 0,
    FILE_NOT_FOUND,
    FILE_UNREADABLE,
};

struct FileData {
    const unsigned char *data;
    size_t size;
    FileError error;
};

FileData ReadFile(const char *path);
void ReleaseFile(FileData &file);
```

Trois choses à y lire :

- **Complet ou échoué.** Il n'y a pas de troisième issue. Une lecture qui
  s'arrête tôt, un fichier qui change pendant la lecture, un chemin qui n'est
  pas un fichier — tout cela est un échec, et rien de tout cela ne laisse
  traîner des octets partiels qui prétendent être un fichier.
- **L'échec est typé par ce sur quoi l'appelant peut agir.** `FILE_NOT_FOUND`
  (« il n'y a rien ») et `FILE_UNREADABLE` (« il y a quelque chose et l'OS dit
  non ») sont les deux cas auxquels un moteur réagit différemment — essayer un
  autre chemin, ou signaler une installation cassée. L'OS a des dizaines de
  numéros d'erreur ; la couture a ceux qui ont un sens.
- **Les octets appartiennent à l'appelant.** En cas de succès, la plateforme a
  pris de la mémoire à l'OS pour contenir le fichier et vous la remet ;
  `ReleaseFile` la rend. La règle de propriété de la leçon 004 dans sa forme
  la plus simple : vous libérez ce que vous avez pris, par la même porte que
  celle par où vous l'avez pris.

### Le côté OS

L'implémentation est l'API de fichiers de l'OS — `open`, `fstat`, `read`,
`close` — et la forme du code est dictée par le contrat :

1. **Ouvrir.** Un chemin manquant est `FILE_NOT_FOUND` (la seule erreur de
   l'OS qui vaille la peine d'être nommée) ; tout le reste que l'OS refuse est
   `FILE_UNREADABLE`.
2. **Prendre la taille d'abord.** `fstat` dit quelle taille fait le fichier
   *avant* que le premier octet ne soit lu — et quel genre de chose est le
   chemin. Un répertoire s'ouvre allègrement sur cet OS et ne lit rien ; un
   périphérique caractère comme `/dev/zero` n'a pas de taille du tout. Les
   deux échouent au contrôle « est-ce un fichier régulier » et deviennent
   `FILE_UNREADABLE` — avant que la moindre mémoire ne soit prise.
3. **Lire exactement cela, et vérifier.** La boucle remplit le tampon jusqu'à
   atteindre la taille ; puis on tente un octet de plus, qui doit ne rien
   trouver. Un fichier qui se termine tôt échoue. Un fichier qui a *grossi*
   pendant la lecture échoue. Le tampon ne contient jamais « la majeure partie
   d'un fichier ».

La démo du moteur prend un chemin sur la ligne de commande — l'`argv` de la
leçon 001, désormais via `main(int argc, char **argv)` — et rapporte ce qu'elle
a obtenu :

```
$ DISPLAY=:99 ./build/game README.md
engine: read README.md: 6113 bytes, 143 lines
```

Les octets complets : 6 113, comptés par le moteur depuis le tampon qu'on lui a
remis, avec en plus le nombre de lignes que `wordcount` reconnaîtrait.

### Les échecs, tels qu'écrits

```
$ DISPLAY=:99 ./build/game no-such-file.txt
engine: no-such-file.txt: file not found
$ echo $?
1
$ DISPLAY=:99 ./build/game src
engine: src: unreadable
$ DISPLAY=:99 ./build/game /dev/zero
engine: /dev/zero: unreadable
```

Chacun sort avec un code non nul, et chacun ne laisse **rien** derrière lui :
le chemin d'erreur ferme la fenêtre qu'il a ouverte (la règle de la leçon 029
— chaque sortie libère ce qu'elle a pris, et une sortie avec un échec typé est
une sortie) et aucune donnée partielle n'a atteint le moteur.

`/dev/zero` mérite qu'on s'y arrête. C'est « un fichier » qui se lit avec
succès pour toujours — la boucle naïve de fichier entier n'en revient jamais.
Le contrat ne le poursuit pas : la taille vient d'abord, la taille est zéro,
le contrôle de fichier régulier échoue, et la réponse est un échec typé en
temps constant. Fichier entier veut dire la taille *du fichier*, pas « jusqu'à
ce que le flux s'arrête ».

Un fichier vide, pour être complet, est un succès — 0 octet complet :

```
$ DISPLAY=:99 ./build/game /tmp/opencode/xcheck/empty.txt
engine: read /tmp/opencode/xcheck/empty.txt: 0 bytes, 0 lines
```

## Étape de code

Un seul changement pour cette leçon : la couture gagne `FileError`, `FileData`,
`ReadFile` et `ReleaseFile`, le côté OS implémente les lectures de fichiers
entiers avec open/fstat/read/close et les deux raisons d'échec typées, et
`main.cpp` prend un chemin dans `argv`, le lit en entier, et rapporte — ou
rapporte l'échec typé et sort proprement. Son état final est étiqueté
`lesson-037`.

```diff
diff --git a/src/main.cpp b/src/main.cpp
index 942d825..26aca64 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -25,7 +25,7 @@ static void DrawMarker(Framebuffer &fb, int x, int y)
             PutPixel(fb, x + i, y + j, 240, 220, 80);
 }
 
-int Run(void)
+int Run(int argc, char **argv)
 {
     platform::WindowResult opened =
         platform::OpenWindow(FRAME_WIDTH, FRAME_HEIGHT);
@@ -64,6 +64,27 @@ int Run(void)
     std::printf("engine: clock %s over 100000 samples, finest step %.0f ns\n",
                 backwards ? "WENT BACKWARDS" : "never backwards", finest * 1e9);
 
+    /* Whole-file reads: the file's complete bytes, or a typed failure —
+       never partial data dressed as success. */
+    if (argc > 1) {
+        platform::FileData file = platform::ReadFile(argv[1]);
+        if (file.error != platform::FILE_OK) {
+            std::printf("engine: %s: %s\n", argv[1],
+                        file.error == platform::FILE_NOT_FOUND
+                            ? "file not found"
+                            : "unreadable");
+            platform::CloseWindow(opened.window);
+            return 1;
+        }
+        long lines = 0;
+        for (size_t i = 0; i < file.size; ++i)
+            if (file.data[i] == '\n')
+                ++lines;
+        std::printf("engine: read %s: %zu bytes, %ld lines\n", argv[1],
+                    file.size, lines);
+        platform::ReleaseFile(file);
+    }
+
     /* The scene: a marker the arrow keys move. The report below is its
        position and the time it moved — the interactive frame makes itself
        observable. */
@@ -177,7 +198,7 @@ int Run(void)
 
 } /* namespace engine */
 
-int main(void)
+int main(int argc, char **argv)
 {
-    return engine::Run();
+    return engine::Run(argc, argv);
 }
diff --git a/src/platform.h b/src/platform.h
index cea5fc6..73ec7e5 100644
--- a/src/platform.h
+++ b/src/platform.h
@@ -7,6 +7,8 @@
 #ifndef PLATFORM_H
 #define PLATFORM_H
 
+#include <stddef.h> /* size_t */
+
 namespace platform {
 
 /* What a window is made of is the OS implementation's business. The engine
@@ -62,6 +64,28 @@ bool HasFocus(const Window *window);
    clock Part 2's frame timing and Part 5's frame-budget report stand on. */
 double Now(void);
 
+/* A file's complete bytes — or a typed failure. Never partial data
+   presented as success. */
+enum FileError {
+    FILE_OK = 0,
+    FILE_NOT_FOUND,  /* nothing is there */
+    FILE_UNREADABLE, /* something is there and the OS says no */
+};
+
+struct FileData {
+    const unsigned char *data; /* the file's complete bytes, or 0 */
+    size_t size;
+    FileError error;
+};
+
+/* Reads a whole file from the OS. On success the bytes are the file —
+   all of it — and they belong to the caller: give them back with
+   ReleaseFile. */
+FileData ReadFile(const char *path);
+
+/* Gives the file's bytes back to the OS. */
+void ReleaseFile(FileData &file);
+
 /* Reads whatever news the OS has about this window and folds it into the
    platform layer's state. The engine never sees an event object — it polls
    state afterwards. Blocks until there is news or the run is interrupted. */
diff --git a/src/platform_x11.cpp b/src/platform_x11.cpp
index d35cf08..3205c28 100644
--- a/src/platform_x11.cpp
+++ b/src/platform_x11.cpp
@@ -11,6 +11,10 @@
 // Lesson 035: the platform clock. POSIX, not ISO C — clock_gettime is the
 // OS's clock interface (the one lesson 020 taught inside snek, now behind
 // the seam), so the feature-test macro goes before the includes.
+//
+// Lesson 037: whole-file reads. File I/O is OS surface too — POSIX here,
+// a second OS's own calls there. Everything in this file is one
+// implementation behind the seam.
 #define _POSIX_C_SOURCE 200809L
 
 #include "platform.h"
@@ -20,9 +24,14 @@
 #include <X11/Xutil.h>
 #include <X11/keysym.h>
 
+#include <errno.h>
+#include <fcntl.h>
 #include <poll.h>
 #include <signal.h>
+#include <stdlib.h>
+#include <sys/stat.h>
 #include <time.h>
+#include <unistd.h>
 
 namespace platform {
 
@@ -212,6 +221,71 @@ double Now(void)
     return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
 }
 
+/* File I/O is the OS side too — POSIX here, Win32's own calls in a second
+   implementation. The bytes the OS reads for us live in memory the OS
+   gives us (its allocator) and leave through ReleaseFile. */
+FileData ReadFile(const char *path)
+{
+    FileData file = { 0, 0, FILE_UNREADABLE };
+
+    int fd = open(path, O_RDONLY);
+    if (fd < 0) {
+        file.error = (errno == ENOENT) ? FILE_NOT_FOUND : FILE_UNREADABLE;
+        return file;
+    }
+
+    /* Whole file means whole file: the size is known before the first
+       byte, and a read that ends early is a failure, not a smaller file. */
+    struct stat st;
+    if (fstat(fd, &st) < 0 || !S_ISREG(st.st_mode)) {
+        close(fd);
+        return file; /* still FILE_UNREADABLE */
+    }
+
+    size_t capacity = (size_t)st.st_size;
+    unsigned char *bytes =
+        (unsigned char *)malloc(capacity ? capacity : 1);
+    if (!bytes) {
+        close(fd);
+        return file;
+    }
+
+    size_t total = 0;
+    while (total < capacity) {
+        ssize_t n = read(fd, bytes + total, capacity - total);
+        if (n < 0) {
+            free(bytes);
+            close(fd);
+            return file;
+        }
+        if (n == 0)
+            break; /* the file ended early — checked below */
+        total += (size_t)n;
+    }
+
+    /* One byte past what the size promised must find nothing, or the file
+       changed under the read — and a moving file is not a whole file. */
+    unsigned char extra;
+    if (total != capacity || read(fd, &extra, 1) != 0) {
+        free(bytes);
+        close(fd);
+        return file;
+    }
+
+    close(fd);
+    file.data = bytes;
+    file.size = total;
+    file.error = FILE_OK;
+    return file;
+}
+
+void ReleaseFile(FileData &file)
+{
+    free((void *)file.data);
+    file.data = 0;
+    file.size = 0;
+}
+
 void PumpEvents(Window *window)
 {
     if (!window || !window->display)
```

## Exercices

Deux extensions « faites-le vôtre ». Chacune se termine par sa solution — un
diff contre l'état final de cette leçon, plus une visite guidée — après
l'énoncé.

### Exercice 1 — Lire dans votre propre mémoire *(extend-the-code)*

`ReadFile` distribue des octets que la plateforme a alloués et que vous devez
libérer. Quand le moteur sait déjà d'où sa mémoire doit venir — un arena, un
tampon statique, un tampon brouillon —, il veut les octets *là*. Étoffez la
couture avec `ReadFileInto(path, into, capacity)` : les octets complets du
fichier dans le tampon de l'appelant, ou un échec typé quand ils n'y tiennent
pas. Rien n'est alloué ; rien n'est libéré. Montrez les deux chemins à l'œuvre
dans une même exécution — et montrez l'échec « ne tient pas » avec un fichier
plus gros que votre tampon.

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-037/ex1.md)

### Exercice 2 — Le fichier qui ne finit jamais *(predict-the-output)*

Trois chemins, trois prédictions — écrivez-les avant de lancer quoi que ce
soit : que répond `ReadFile` pour un **répertoire**, pour **`/dev/zero`**, et
pour un **chemin qui n'existe pas** ? Faites ensuite raconter
l'implémentation — une ligne d'instrumentation à chaque refus, nommant
l'erreur de l'OS elle-même — et lancez les trois. Réconciliez les prédictions
avec ce que l'OS a dit, et expliquez pourquoi `/dev/zero` est le cas qui vous
dit ce que « fichier entier » signifie vraiment.

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-037/ex2.md)

---

**Partie :** [Partie 1 — la couche plateforme](../../index.md) ·
**Précédente :** [Leçon 036 — le temps de frame comme donnée mesurée](lesson-036-frame-time.md) ·
**Suivante :** [Leçon 038 — écritures de fichier entier et aller-retour](lesson-038-file-write.md) ·
**Étiquette de code :** [`lesson-037`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-037)

*Page traduite de la version anglaise `book/lessons/part-1/lesson-037-file-read.md`,
révision `c9b4169`.*

<!-- translation-source: book/lessons/part-1/lesson-037-file-read.md @ c9b4169 -->
