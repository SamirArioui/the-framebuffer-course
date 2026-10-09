# Solution : exercice 3 — Les conditions ne voient qu'une trame

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 3 — Les conditions ne voient qu'une trame](../../lessons/part-0/lesson-002-gdb.md) de la leçon 002.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-002/ex3.patch}}
```

## Visite guidée

La première condition meurt au moment de sa définition :

```
(gdb) break CountBytes if i == 2
No symbol "i" in current context.
```

Une condition est résolue contre la portée de l'endroit où le point d'arrêt se
trouve. À l'intérieur de `CountBytes`, ces noms sont son paramètre et ses
variables locales — `f`, `bytes`, `c` — plus les noms de portée fichier ; `i`
est une locale de la boucle de `main` et appartient à une trame qui
n'existera même pas quand la condition sera testée.
`break CountBytes if bytes == 0` *est* accepté (`bytes` est un nom que
`CountBytes` connaît), mais à l'entrée de la fonction l'initialiseur n'a pas
encore tourné — `bytes` contient la camelote que l'emplacement de pile
portait, presque jamais zéro — aussi le point d'arrêt passe devant les deux
appels sans s'arrêter.

`$hits` est différent en nature : une *variable de commodité* de gdb. Elle
vit dans le débogueur, pas dans votre programme ; rien de ce que vous
compilez ne peut la voir. gdb évalue la condition à chaque arrêt, `++$hits`
les compte, et le point d'arrêt se déclenche exactement au deuxième appel —
confirmé par l'impression d'instrumentation, qui montre `file 2: b.txt` sur
`stderr` juste avant l'arrêt.

*Page traduite de la version anglaise `book/solutions/lesson-002/ex3.md`,
révision `a3b4a6a`.*

<!-- translation-source: book/solutions/lesson-002/ex3.md @ a3b4a6a -->
