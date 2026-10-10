# Leçon 042 — l'interface comme contrat

{{#include ../../stability-horizon.md}}

## Prose

Il y a seize leçons, la couture était deux fonctions et une promesse.
Aujourd'hui, elle est auditée : **ce qu'un second OS doit implémenter**,
vérifié contre les scénarios à frontière unique de la spécification plateforme
et contre le code tel qu'il est. C'est la leçon qui transforme « nous avons
écrit une couche plateforme » en « la couche plateforme est un contrat » — et
la différence, c'est qu'un contrat porte des affirmations que l'on peut
*vérifier*.

### Ce qu'un second OS doit implémenter

Tout ce que la couture déclare — rien de plus, rien de moins. Depuis
`platform.h`, la liste complète :

| Le contrat | Ce que cela signifie pour le moteur |
| ---------- | ----------------------------------- |
| `OpenWindow`, `CloseWindow` | une fenêtre de la taille demandée, ressources libérées |
| `PumpEvents`, `CloseRequested` | les nouvelles pliées dans l'état ; chaque fin se signale d'une même façon |
| `KeyDown`, `KeyPressed`, `HasFocus` | entrée scrutée : enfoncée maintenant, vient d'être enfoncée, focus |
| `Present` | nos octets dans la fenêtre, terminé quand l'appel rend la main |
| `Now`, `PageSize` | le temps monotone ; la page comme unité de mémoire |
| `ReserveMemory`, `ReleaseMemory` | des pages entières, mises à zéro, possédées jusqu'à leur libération |
| `ReadFile`, `ReleaseFile`, `WriteFile` | des fichiers entiers ou des échecs typés, dans les deux sens |

Quinze fonctions, cinq structures et énumérations simples (`Window`, `Key`,
`OpenError`/`WindowResult`, `MemoryError`/`Reservation`,
`FileError`/`FileData`), et une règle qui les gouverne toutes : **aucun type
d'OS n'apparaît dans l'interface**. `Window` est déclaré et jamais défini dans
l'en-tête ; les touches sont celles de la couture ; les erreurs sont celles de
la couture. Un second OS implémente les mêmes fonctions et définit
`struct Window` avec ce dont sa propre fenêtre est faite. L'exercice 1 fait
exactement cela avec un stub, et le moteur se lie contre lui sans une seule
ligne modifiée.

### Scénario un : le code du moteur est sans OS

> **WHEN** un fichier source hors de la couche plateforme est inspecté,
> **THEN** il ne contient aucun appel ni en-tête propre à un OS.

Vérifié mécaniquement, et désormais *gardé* vérifié — `tools/check-boundary.sh`
parcourt chaque fichier du moteur et échoue sur les en-têtes ou les appels
d'OS :

```
$ ./tools/check-boundary.sh
boundary: engine code must not name an OS
boundary: OK — OS headers and OS calls appear only in src/platform_x11.cpp
boundary: the contract a second OS implements is declared in src/platform.h
boundary: 14 single-line declarations there (multi-line ones are in the header)
```

Les objets s'accordent au niveau de l'édition de liens. Les symboles non
définis de `main.o` sont des fonctions du moteur, celles du langage lui-même et
des appels `platform::` — et `platform_x11.o` définit exactement les quinze du
contrat :

```
$ nm build/obj/platform_x11.o | grep " T " | c++filt | sed 's/^[0-9a-f]* T //' | sort
platform::CloseRequested(platform::Window const*)
platform::CloseWindow(platform::Window*)
platform::HasFocus(platform::Window const*)
platform::KeyDown(platform::Window const*, platform::Key)
platform::KeyPressed(platform::Window*, platform::Key)
platform::Now()
platform::OpenWindow(int, int)
platform::PageSize()
platform::Present(platform::Window*, unsigned char const*, int, int)
platform::PumpEvents(platform::Window*)
platform::ReadFile(char const*)
platform::ReleaseFile(platform::FileData&)
platform::ReleaseMemory(platform::Reservation&)
platform::ReserveMemory(unsigned long)
platform::WriteFile(char const*, unsigned char const*, unsigned long)
```

La frontière n'est pas un schéma ; c'est l'étape d'édition de liens.

### Scénario deux : un second OS se branche

> **WHEN** une implémentation pour un autre OS est ajoutée derrière
> l'interface, **THEN** seuls les fichiers de la couche plateforme changent.

La liste des fichiers modifiés en compte exactement un : le fichier
d'implémentation. L'exercice 1 le prouve — un OS stub dans son propre fichier,
le moteur compilé et lié contre lui sans modification, aucun X11 nulle part sur
la ligne :

```
$ ./build/game-stub
engine: no display to open a window on
```

L'audit a trouvé **un accroc**, et l'honnêteté exige de le consigner : le build
compile chaque source sous `src/`, donc deux implémentations ne peuvent pas y
coexister — chacune définit les mêmes quinze fonctions du contrat, et l'éditeur
de liens refuserait. Le contrat dit donc **une implémentation par build** : le
fichier du portage remplace celui du premier OS (ou, si les deux doivent vivre
dans l'arbre, la sélection de fichiers du build est la seule ligne au courant —
et ce n'est pas du code du moteur). Le scénario tient ; la formulation est
« seuls les fichiers de la couche plateforme changent », pas « seuls de
nouveaux fichiers apparaissent ».

### Les limites de l'audit

Le contrôle passe sur l'arbre d'aujourd'hui — et un contrôle est une *liste des
erreurs auxquelles quelqu'un a pensé*. L'exercice 2 ouvre cette porte
délibérément : une fuite de frontière dont le contrôle n'entend jamais parler,
et la question de ce que la revue doit faire que l'outillage ne peut pas. Les
conventions ont déjà donné la réponse pour la loi du langage — *imposé par la
revue, pas par l'outillage* — et la frontière est imposée de la même façon. Le
contrôle est le premier filtre, bon marché. La revue est le reste.

### Où en est la couture

L'interface est **fixe** — l'API a été fixée avant sa première implémentation
(design D1) et elle a grandi un contrat à la fois, chaque leçon ajoutant
exactement une promesse. La partie 2 y présente des pixels, la partie 3 mixe du
son à côté d'elle, la partie 4 y charge des assets, la partie 5 y mesure des
frames. Rien de tout cela ne touche le rapport entre le code du moteur et l'OS :
le code du moteur demande à la couture ; la couture répond pour la machine.

La dernière chose qui manque est la chose pour laquelle tout cela existe : une
exécution complète et mesurée qui utilise chaque partie du contrat à la fois.
C'est la leçon 043.

## Étape de code

Un seul changement pour cette leçon : `tools/check-boundary.sh` — le scénario à
frontière unique sous forme de contrôle répétable qui parcourt les fichiers
source du moteur et échoue sur les en-têtes ou les appels d'OS hors de
l'implémentation. Son état final est étiqueté `lesson-042`.

```diff
diff --git a/tools/check-boundary.sh b/tools/check-boundary.sh
new file mode 100755
index 0000000..39582f3
--- /dev/null
+++ b/tools/check-boundary.sh
@@ -0,0 +1,54 @@
+#!/usr/bin/env bash
+#
+# check-boundary.sh — the single-OS-boundary check.
+#
+# Lesson 042: the seam's contract, audited mechanically. Engine code may
+# include the platform interface and the language's own headers — nothing
+# that knows which OS it is on. The platform implementation files are the
+# only place OS headers and OS calls may appear; a second OS replaces those
+# files and nothing else.
+#
+# Usage:
+#   ./tools/check-boundary.sh
+#
+# Exits 0 when the boundary holds, 1 when it is breached.
+
+set -euo pipefail
+
+cd "$(dirname "$0")/.."
+
+# The files a second OS replaces. Everything else is engine code.
+IMPL="src/platform_x11.cpp"
+
+# Headers that only an OS has. The language's own headers (<cstdio>,
+# <cstring>, <stddef.h>, ...) are fine anywhere — they are not an OS.
+OS_HEADERS='<X11/|<sys/|<unistd\.h>|<fcntl\.h>|<poll\.h>|<signal\.h>|<errno\.h>|<time\.h>'
+
+# Calls only an OS answers. The list grows with the seam.
+OS_CALLS='(^|[^A-Za-z0-9_:])(X[A-Z][A-Za-z]+|mmap|munmap|mprotect|clock_gettime|nanosleep|sysconf|open|close|read|write|fstat|poll|signal)\s*\('
+
+status=0
+
+echo "boundary: engine code must not name an OS"
+for f in src/*.h src/*.cpp; do
+    [ "$f" = "$IMPL" ] && continue
+    if hits=$(grep -nE "$OS_HEADERS" "$f"); then
+        echo "BREACH: $f includes an OS header:"
+        echo "$hits"
+        status=1
+    fi
+    if hits=$(grep -nE "$OS_CALLS" "$f"); then
+        echo "BREACH: $f calls an OS function:"
+        echo "$hits"
+        status=1
+    fi
+done
+
+if [ "$status" -eq 0 ]; then
+    echo "boundary: OK — OS headers and OS calls appear only in $IMPL"
+fi
+
+echo "boundary: the contract a second OS implements is declared in src/platform.h"
+echo "boundary: $(grep -cE '^[A-Za-z].*\(.*\);' src/platform.h) single-line declarations there (multi-line ones are in the header)"
+
+exit "$status"
```

## Exercices

Deux extensions « faites-le vôtre ». Chacune se termine par sa solution — un
diff contre l'état final de cette leçon, plus une visite guidée — après
l'énoncé.

### Exercice 1 — Le second OS *(extend-the-code)*

La preuve d'un contrat est une seconde implémentation de celui-ci. Écrivez
`tools/platform_stub.cpp` : chaque fonction que la couture déclare, implémentée
par un refus typé (`OPEN_NO_DISPLAY`, `FILE_NOT_FOUND`, `MEMORY_NO_MEMORY`) —
un OS complet qui ne fait rien, honnêtement. Liez ensuite le moteur contre lui —
chaque fichier du moteur inchangé — et exécutez-le. Qu'est-ce que la ligne de
liaison prouve, que l'en-tête se contente de promettre ? Et où votre stub
a-t-il dû définir `struct Window`, et pourquoi là ?

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-042/ex1.md)

### Exercice 2 — Ce que le contrôle ne peut pas voir *(explain-in-prose)*

**Cet exercice franchit la frontière exprès** — un état pédagogique délibéré.
Trouvez un moyen d'atteindre l'OS depuis `main.cpp` que `check-boundary.sh`
n'attrape pas (`std::system` en est un ; trouvez le vôtre aussi), montrez le
contrôle passer sur l'arbre cassé, puis notez la *classe* de fuites qu'aucun
grep ne peut attraper — les appels par indirection, les fonctions de la libc
qui parlent au noyau sans le dire, le code qu'une macro développe. Que doit
comprendre un relecteur pour les attraper, et pourquoi le contrat d'écriture
dit-il « imposé par la revue, pas par l'outillage » ?

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-042/ex2.md)

---

**Partie :** [Partie 1 — la couche plateforme](../../index.md) ·
**Précédente :** [Leçon 041 — les arenas](lesson-041-arenas.md) ·
**Suivante :** [Leçon 043 — la démo de clôture : la couche plateforme terminée](lesson-043-demo.md) ·
**Étiquette de code :** [`lesson-042`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-042)

*Page traduite de la version anglaise `book/lessons/part-1/lesson-042-contract.md`,
révision `e4b35c3`.*

<!-- translation-source: book/lessons/part-1/lesson-042-contract.md @ e4b35c3 -->
