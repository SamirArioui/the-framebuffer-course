# Solution : exercice 1 — Le deuxième arrêt ressemble au premier

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — Le deuxième arrêt ressemble au premier](../../lessons/part-0/lesson-002-gdb.md) de la leçon 002.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-002/ex1.patch}}
```

## Visite guidée

La prédiction : deux trames — `CountBytes` en haut, `main` en dessous — avec
`argc=3` dans l'appelant (deux fichiers plus le nom du programme), et la
surprise : `f` contient la *même* adresse aux deux arrêts. Sortie réelle au
deuxième arrêt :

```
Breakpoint 1, CountBytes (f=0x5555555592a0) at wordcount.c:8
8	    unsigned long bytes = 0;
returning 3

Breakpoint 1, CountBytes (f=0x5555555592a0) at wordcount.c:8
8	    unsigned long bytes = 0;
#0  CountBytes (f=0x5555555592a0) at wordcount.c:8
#1  0x00005555555552f4 in main (argc=3, argv=0x7fffffffdab8) at wordcount.c:30
```

Pourquoi la même adresse : `main` a fermé le premier fichier avant d'ouvrir le
second, et la bibliothèque C a remis le même morceau de tas libéré au nouveau
`fopen` — une réutilisation que vous verrez proprement dans la leçon 004. La
leçon pour le débogage : la backtrace porte les noms de fonctions et les
*valeurs* des arguments, et les deux appels se ressemblent dans la forme. Les
valeurs des trames ne peuvent pas vous dire sur quel fichier vous êtes ;
l'impression d'instrumentation le peut — `returning 3` sur `stderr` prouve que
le premier appel était déjà fini avant le deuxième arrêt.

*Page traduite de la version anglaise `book/solutions/lesson-002/ex1.md`,
révision `a3b4a6a`.*

<!-- translation-source: book/solutions/lesson-002/ex1.md @ a3b4a6a -->
