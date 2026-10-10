# Leçon 038 — écritures de fichier entier et aller-retour

{{#include ../../stability-horizon.md}}

## Prose

Les lectures font *entrer* les octets ; la moitié du contrat d'aujourd'hui les
fait *sortir* :

> **Une écriture crée le fichier ou le remplace avec exactement les octets
> donnés — ou c'est un échec typé.**

Et le scénario qui donne un sens au mot « entier » des deux côtés à la fois :
**écrire des octets vers un chemin, relire ce chemin, et les octets concordent.**
L'aller-retour est la plus petite preuve que les données du moteur survivent au
voyage à travers l'OS et au retour.

### Le contrat

```c++
FileError WriteFile(const char *path, const unsigned char *data, size_t size);
```

Le côté lecture renvoyait des octets ; le côté écriture les consomme et ne
renvoie que le verdict. Trois groupes de mots du contrat font tout le travail :

- **Crée ou remplace.** Un chemin qui n'existe pas devient un fichier ; un
  chemin qui existe revient avec exactement ces octets et rien de ce qui s'y
  trouvait avant. `O_CREAT | O_TRUNC` sur cet OS — les deux drapeaux *sont* ces
  deux mots.
- **Exactement les octets donnés.** Pas « la plupart d'entre eux ». `write()`
  sur cet OS a le droit de prendre moins d'octets qu'on ne lui en offre — une
  écriture partielle est un événement normal, pas une erreur — donc la boucle
  continue d'écrire jusqu'à ce que le compte dise que le fichier les a tous. La
  version déroulée « un seul appel à `write()` » est un bug qui ne se manifeste
  que sur certains fichiers, certains jours.
- **Ou un échec typé.** `FILE_UNWRITABLE` couvre le refus de l'OS — et l'OS dit
  non pour au moins trois raisons distinguables (l'exercice 2 les rend
  visibles). La couture les garde dans une seule réponse typée jusqu'à ce que
  le moteur ait une raison de vouloir savoir laquelle.

### L'aller-retour

Le moteur écrit un motif de 256 octets — l'octet `i` vaut `i * 7`, un motif
qu'aucune compression ni transcodage ne saurait préserver par accident —, relit
le chemin via `ReadFile` et compare, octet par octet :

```
$ DISPLAY=:99 ./build/game /tmp/opencode/xcheck/roundtrip.bin
engine: wrote /tmp/opencode/xcheck/roundtrip.bin: 256 bytes
engine: round-trip ok: 256 bytes match
```

256 octets à l'aller, 256 octets au retour, chacun d'eux l'octet qui est parti.
Vérifié aussi depuis l'extérieur du moteur — le fichier sur le disque contient
exactement la charge utile :

```
$ python3 -c "import sys; sys.stdout.buffer.write(bytes((i*7)&0xFF for i in range(256)))" > payload.bin
$ cmp payload.bin /tmp/opencode/xcheck/roundtrip.bin && echo "cmp: file bytes are exactly the payload"
cmp: file bytes are exactly the payload
```

Écrire le même chemin une seconde fois le remplace, exactement comme le contrat
le dit — un second lancement refait un `round-trip ok` sur le même fichier. Une
« sauvegarde », ce sera cela, cent fois de suite, dans le futur du jeu.

### Les échecs

Deux des façons dont une écriture échoue, toutes deux finissant sur la même
réponse typée et la même sortie propre (la fenêtre est fermée en sortant — la
règle de la leçon 029 tient sur chaque chemin) :

```
$ DISPLAY=:99 ./build/game /no-such-dir/out.bin
engine: /no-such-dir/out.bin: could not write
$ echo $?
1
$ DISPLAY=:99 ./build/game /dev/full
engine: /dev/full: could not write
$ echo $?
1
```

`/no-such-dir/out.bin` échoue parce que l'OS ne peut pas créer de fichier dans
un répertoire qui n'est pas là. `/dev/full` est plus étrange et plus utile :
c'est un fichier qui s'ouvre parfaitement et fait échouer chaque écriture avec
« no space left on device » — le disque toujours-défaillant de l'OS lui-même.
C'est le test qui prouve que le chemin d'échec de la boucle d'écriture ne fait
pas passer un fichier court pour un fichier entier.

Une note honnête pour les deux : une écriture qui échoue peut laisser un
fichier partiel sur le disque — l'OS a écrit quelques octets avant de refuser.
Le moteur ne s'y trompe pas (il a l'échec, pas les données), mais les octets
laissés derrière sont bien réels. Rendre les écritures tout-ou-rien — écrire un
fichier temporaire, puis le renommer par-dessus la cible — est une vraie
technique et un bon sujet d'exercice.

### Les deux moitiés, une seule règle

Lectures : des octets complets ou un échec typé. Écritures : exactement les
octets, ou un échec typé. La symétrie est le propos — les entrées-sorties de
fichiers de la couche plateforme ne renvoient jamais « la plupart d'un
fichier » dans un sens ou dans l'autre, et le moteur n'a jamais à deviner s'il
a obtenu ce qu'il demandait. Le chargement des ressources de la partie 4
s'appuiera exactement là-dessus quand il commencera à remplir les arenas depuis
les fichiers du disque.

## Étape de code

Un seul changement pour cette leçon : la couture gagne `WriteFile` et
`FILE_UNWRITABLE`, le côté OS implémente le créer-ou-remplacer avec une boucle
à l'épreuve des écritures partielles, et `main.cpp` fait tourner
l'aller-retour : un motif d'octets écrit vers le chemin passé en ligne de
commande, relu, comparé, rapporté. Son état final est étiqueté `lesson-038`.

```diff
diff --git a/src/main.cpp b/src/main.cpp
index 26aca64..fafec04 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -64,24 +64,42 @@ int Run(int argc, char **argv)
     std::printf("engine: clock %s over 100000 samples, finest step %.0f ns\n",
                 backwards ? "WENT BACKWARDS" : "never backwards", finest * 1e9);
 
-    /* Whole-file reads: the file's complete bytes, or a typed failure —
-       never partial data dressed as success. */
+    /* Whole-file writes and reads: bytes leave the engine, come back, and
+       had better be the same bytes — or the failure is the answer. */
     if (argc > 1) {
+        unsigned char payload[256];
+        for (int i = 0; i < (int)sizeof payload; ++i)
+            payload[i] = (unsigned char)(i * 7); /* a pattern, byte by byte */
+
+        platform::FileError wrote =
+            platform::WriteFile(argv[1], payload, sizeof payload);
+        if (wrote != platform::FILE_OK) {
+            std::printf("engine: %s: could not write\n", argv[1]);
+            platform::CloseWindow(opened.window);
+            return 1;
+        }
+        std::printf("engine: wrote %s: %zu bytes\n", argv[1], sizeof payload);
+
         platform::FileData file = platform::ReadFile(argv[1]);
         if (file.error != platform::FILE_OK) {
-            std::printf("engine: %s: %s\n", argv[1],
-                        file.error == platform::FILE_NOT_FOUND
-                            ? "file not found"
-                            : "unreadable");
+            std::printf("engine: %s: could not read back\n", argv[1]);
             platform::CloseWindow(opened.window);
             return 1;
         }
-        long lines = 0;
-        for (size_t i = 0; i < file.size; ++i)
-            if (file.data[i] == '\n')
-                ++lines;
-        std::printf("engine: read %s: %zu bytes, %ld lines\n", argv[1],
-                    file.size, lines);
+
+        long mismatch = -1;
+        size_t checked = file.size < sizeof payload ? file.size : sizeof payload;
+        for (size_t i = 0; i < checked; ++i)
+            if (file.data[i] != payload[i]) {
+                mismatch = (long)i;
+                break;
+            }
+        if (file.size == sizeof payload && mismatch < 0) {
+            std::printf("engine: round-trip ok: %zu bytes match\n", file.size);
+        } else {
+            std::printf("engine: round-trip FAILED: %zu bytes back (wanted %zu), first mismatch %ld\n",
+                        file.size, sizeof payload, mismatch);
+        }
         platform::ReleaseFile(file);
     }
 
diff --git a/src/platform.h b/src/platform.h
index 73ec7e5..ae9ddca 100644
--- a/src/platform.h
+++ b/src/platform.h
@@ -68,8 +68,9 @@ double Now(void);
    presented as success. */
 enum FileError {
     FILE_OK = 0,
-    FILE_NOT_FOUND,  /* nothing is there */
-    FILE_UNREADABLE, /* something is there and the OS says no */
+    FILE_NOT_FOUND,   /* nothing is there */
+    FILE_UNREADABLE,  /* something is there and the OS says no */
+    FILE_UNWRITABLE,  /* the OS refused to take the bytes */
 };
 
 struct FileData {
@@ -86,6 +87,11 @@ FileData ReadFile(const char *path);
 /* Gives the file's bytes back to the OS. */
 void ReleaseFile(FileData &file);
 
+/* Writes a whole file to the OS: the file is created, or replaced if it
+   already exists, with exactly the bytes given — or the write is a typed
+   failure. */
+FileError WriteFile(const char *path, const unsigned char *data, size_t size);
+
 /* Reads whatever news the OS has about this window and folds it into the
    platform layer's state. The engine never sees an event object — it polls
    state afterwards. Blocks until there is news or the run is interrupted. */
diff --git a/src/platform_x11.cpp b/src/platform_x11.cpp
index 3205c28..a569e4d 100644
--- a/src/platform_x11.cpp
+++ b/src/platform_x11.cpp
@@ -286,6 +286,31 @@ void ReleaseFile(FileData &file)
     file.size = 0;
 }
 
+FileError WriteFile(const char *path, const unsigned char *data, size_t size)
+{
+    /* Created, or replaced if it exists — exactly these bytes or nothing
+       the caller can mistake for success. */
+    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0666);
+    if (fd < 0)
+        return FILE_UNWRITABLE;
+
+    /* write() may take fewer bytes than offered — a short write is not a
+       finished file. Keep going until they are all in. */
+    size_t total = 0;
+    while (total < size) {
+        ssize_t n = write(fd, data + total, size - total);
+        if (n < 0) {
+            close(fd);
+            return FILE_UNWRITABLE;
+        }
+        total += (size_t)n;
+    }
+
+    if (close(fd) < 0) /* the OS can still refuse at the very end */
+        return FILE_UNWRITABLE;
+    return FILE_OK;
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

### Exercice 1 — La capture d'écran *(extend-the-code)*

La leçon 017 écrivait un fichier image à la main dans `paint` ; le moteur peut
désormais sauvegarder ses propres pixels de la même façon. Écrivez un
`SaveScreenshot` qui construit un fichier PPM en mémoire — l'en-tête de trois
lignes, puis un triplet RGB par pixel (attention à l'ordre des octets : le
framebuffer est en bleu-vert-rouge-x, le fichier en rouge-vert-bleu) — et le
confie à `WriteFile`. Prenez le chemin de sortie comme second argument de ligne
de commande, dessinez la scène d'abord, et vérifiez le fichier en relisant ses
pixels : la couleur du marqueur est-elle à la position rapportée du marqueur ?

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-038/ex1.md)

### Exercice 2 — Le périphérique qui est toujours plein *(predict-the-output)*

Trois chemins, trois prédictions — écrivez-les avant de lancer quoi que ce
soit : que répond `WriteFile` pour **`/dev/full`**, pour un chemin dans un
**répertoire qui n'existe pas**, et pour un chemin dans un **répertoire qui
vous refuse** (`/usr/out.bin` fait l'affaire) ? Faites ensuite raconter
l'implémentation — une ligne d'instrumentation à chaque refus, nommant l'erreur
de l'OS elle-même. Accordez les prédictions, et tranchez : la couture doit-elle
gagner des échecs typés pour « disque plein » contre « permission refusée », ou
un seul `FILE_UNWRITABLE` est-il le bon contrat ? Défendez la réponse.

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-038/ex2.md)

---

**Partie :** [Partie 1 — la couche plateforme](../../index.md) ·
**Précédente :** [Leçon 037 — lectures de fichier entier](lesson-037-file-read.md) ·
**Suivante :** [Leçon 039 — plongée dans la mémoire virtuelle](lesson-039-virtual-memory.md) ·
**Étiquette de code :** [`lesson-038`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-038)

*Page traduite de la version anglaise `book/lessons/part-1/lesson-038-file-write.md`,
révision `dabf156`.*

<!-- translation-source: book/lessons/part-1/lesson-038-file-write.md @ dabf156 -->
