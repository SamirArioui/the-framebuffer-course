# Solution : exercice 4 — Un compteur dans deux portées

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 4 — Un compteur dans deux portées](../../lessons/part-0/lesson-002-gdb.md) de la leçon 002.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-002/ex4.patch}}
```

## Visite guidée

Le compteur est une variable de portée fichier, incrémentée en haut de
`CountBytes` et rapportée par `main` après le dernier fichier. Sur un
terminal, les lignes de compteur s'affichent au fur et à mesure que chaque
fichier est traité et le compteur vient en dernier ; capturé à travers un
seul tube, `stderr` arrive avant la `stdout` mise en tampon par blocs et
l'ordre s'inverse :

```
CountBytes called 2 times
3 a.txt
6 b.txt
```

Le test de portée dans gdb est l'enjeu. Arrêté à l'intérieur de `CountBytes` :

```
(gdb) print calls
$1 = 0
(gdb) print i
No symbol "i" in current context.
(gdb) up
#1  0x00005555555552e4 in main (argc=3, argv=0x7fffffffdab8) at wordcount.c:32
(gdb) print calls
$2 = 0
(gdb) print i
$3 = 1
```

Une locale comme `i` appartient à une trame : elle n'existe que pendant que
cette trame est vivante et n'est nommable que depuis son intérieur. `calls` a
un seul emplacement de stockage pour toute l'exécution, aussi chaque trame
peut-elle la nommer. (Elle lit `0` au premier arrêt parce que le point
d'arrête se déclenche avant que `++calls` ne tourne.) Notez que `static`
limite la visibilité du nom *au moment de la compilation* à ce fichier — il
ne cache pas la variable au débogueur. Cette différence entre locales et
globales est exactement ce à quoi l'exercice 3 s'est heurté par l'autre bout.
(Les adresses dans les transcriptions sont spécifiques à la machine ; les
trames et les valeurs, non.)

*Page traduite de la version anglaise `book/solutions/lesson-002/ex4.md`,
révision `bb8d4ab`.*

<!-- translation-source: book/solutions/lesson-002/ex4.md @ bb8d4ab -->
