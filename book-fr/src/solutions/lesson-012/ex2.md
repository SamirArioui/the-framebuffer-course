# Solution : exercice 2 — Statistiques de table

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — Statistiques de table](../../lessons/part-0/lesson-012-multi-file.md) de la leçon 012.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-012/ex2.patch}}
```

## Visite guidée

Le changement traverse les trois couches du contrat de l'en-tête : la
déclaration dans `hashtable.h`, la définition dans `hashtable.c` (qui marche
les chaînes — son unité de traduction est celle qui inclut `dynarray.h` et
peut toucher `ht->chains[b].len`), et un appel dans `main.c`. Le compilateur
vérifie l'appel contre la déclaration et la définition contre elle aussi ; un
désaccord dans l'un ou l'autre sens est un diagnostic avant que l'éditeur de
liens ne soit jamais impliqué. De vraies exécutions sur deux fichiers :

```
stats: entries=14 buckets=1024 empty=1010 longest=1
stats: entries=10000 buckets=1024 empty=0 longest=21
```

La première ligne est le texte de l'exercice en deux lignes : 14 clés dans
1024 seaux veut dire 1010 seaux vides et des chaînes d'au plus un — un
facteur de charge de 0,014. La seconde est le texte de référence de 300 000
mots de l'exercice 4 de la leçon 011 : 10 000 entrées, plus aucun seau vide,
chaîne la plus longue 21 contre une moyenne proche de 10 — la dispersion que
vous attendez quand les clés hachent inégalement mais pas de façon
pathologique. Ces quatre nombres sont l'histoire du facteur de charge de la
leçon 011 réduite à une ligne, et ce sont exactement ceux que vous vérifiez
avant de blâmer la fonction de hachage. Le rapport va sur `stderr`, aussi les
compteurs triés sur `stdout` restent-ils propres — même habitude que chaque
diagnostic depuis la leçon 001.

*Page traduite de la version anglaise `book/solutions/lesson-012/ex2.md`,
révision `e5cc4fc`.*

<!-- translation-source: book/solutions/lesson-012/ex2.md @ e5cc4fc -->
