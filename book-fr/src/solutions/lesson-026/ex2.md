# Solution : exercice 2 — Deux unités de traduction

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — Deux unités de traduction](../../lessons/part-1/lesson-026-birth.md) de la leçon 026.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-026/ex2.patch}}
```

## Visite guidée

La séparation, c'est un en-tête, une implémentation, et un `main.cpp` réduit à
son unique travail. `engine.h` est un contrat entre unités de traduction au
sens où la leçon 012 l'a enseigné : il déclare `engine::Run` et rien d'autre,
enveloppé dans la garde d'inclusion `ENGINE_H` pour qu'une seconde inclusion
ne change rien. Le `#include "engine.h"` des deux fichiers source le trouve
parce que la forme entre guillemets cherche d'abord dans le répertoire du
fichier qui inclut — aucun drapeau `-I`, aucun chemin.

Le code du moteur se compile maintenant tout seul, et la définition de `Run`
est une fonction ordinaire dans `namespace engine` : le nom qualifié que voit
l'éditeur de liens est le même genre de nom manglé préfixé par la longueur que
la leçon 025 lisait dans `nm`.

La construction prouve la forme :

```
$ ./build.sh
build: compiling 2 source(s) from src/
  CC  src/engine.cpp
  CC  src/main.cpp
  LD  build/game
build: OK (2 source(s) compiled -> build/game)
$ ls build/obj
engine.o
main.o
$ ./build/game
$ echo $?
0
```

Pourquoi le script de construction n'a jamais changé : `build.sh` ne nomme
jamais de fichier source. Il *trouve* chaque fichier C et C++ sous `src/`,
compile chacun vers son propre objet sous `build/obj/`, et lie tout ce qu'il a
compilé. Ajouter `engine.cpp` et `engine.h` à la base de code a suffi — la
construction en une commande fait exactement ce pour quoi elle a été écrite,
et la base de code peut désormais grandir d'un fichier par leçon sans que
personne ne touche à la construction. (Le `.h` n'est jamais compilé
directement ; c'est du texte inclus dans les deux unités de traduction qui en
ont besoin.)

*Page traduite de la version anglaise `book/solutions/lesson-026/ex2.md`, révision `100464c`.*

<!-- translation-source: book/solutions/lesson-026/ex2.md @ 100464c -->
