# Solution : exercice 1 — Le moteur dit son nom

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — Le moteur dit son nom](../../lessons/part-1/lesson-026-birth.md) de la leçon 026.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-026/ex1.patch}}
```

## Visite guidée

Trois ajouts, et chacun d'eux est dans la loi. `<cstdio>` est la bibliothèque
standard du langage lui-même — le stdio de C avec ses noms dans `std`, qui
n'est pas la STL et pas une bibliothèque externe. Les deux valeurs `constexpr`
sont du genre de la leçon 025 : le compilateur les plie avant que le programme
existe, si bien que la version atteint `printf` comme argument littéral plutôt
qu'une valeur récupérée depuis la mémoire. `Run` et les constantes vivent dans
`namespace engine`, et `main` reste l'unique global — il transmet un seul
appel dans l'espace de noms, exactement la forme sur laquelle la leçon 025
s'est achevée avec `snek::Run`.

La construction et l'exécution, en entier :

```
$ ./build.sh
build: compiling 1 source(s) from src/
  CC  src/main.cpp
  LD  build/game
build: OK (1 source(s) compiled -> build/game)
$ ./build/game
the framebuffer engine v0
```

Une ligne de sortie est tout l'enjeu de l'exercice : la base de code fait
maintenant quelque chose que vous pouvez voir, elle le fait depuis l'intérieur
de l'espace de noms que la loi nomme, et l'ajouter n'a coûté aucune nouvelle
caractéristique de langage au-delà des cinq admises. Quand la leçon 027
déplace le premier vrai code du moteur dans `namespace engine`, c'est le motif
dans lequel il atterrit.

*Page traduite de la version anglaise `book/solutions/lesson-026/ex1.md`, révision `100464c`.*

<!-- translation-source: book/solutions/lesson-026/ex1.md @ 100464c -->
