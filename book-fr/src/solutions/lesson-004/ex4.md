# Solution : exercice 4 — L'entrée standard, revue

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 4 — L'entrée standard, revue](../../lessons/part-0/lesson-004-heap-buffers.md) de la leçon 004.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-004/ex4.patch}}
```

## Visite guidée

Le changement est une branche de cinq lignes là où la leçon 001 avait une
boucle de comptage : appeler `CountStream` avec `stdin` — le `FILE *` déjà
ouvert quand le programme démarre — accumuler dans un `struct Counts` mis à
zéro, et imprimer la ligne sans nom de fichier :

```
$ ./wordcount < story.txt
2 4 21 11
$ ./wordcount story.txt
2 4 21 11 story.txt
```

Cela fonctionne à travers un tube aussi : `ls | ./wordcount`. C'est la
promesse du premier exercice de la leçon 001 tenue : à l'époque la boucle de
comptage vivait dans `main` et le support de stdin voulait dire réécrire la
boucle ; maintenant la boucle est une fonction et l'appelant est un appel
plus un `printf`. Les règles de cet exercice se reportent inchangées — en
présence d'arguments de fichiers, `stdin` est entièrement ignoré, ce que fait
`wc` et ce qui garde le programme prévisible dans les pipelines. Une
subtilité : la ligne stdin perd la dernière colonne parce qu'il n'y a pas de
nom à imprimer, et l'erreur `usage` disparaît avec les arguments — pas de
fichiers ne veut plus dire « pas d'entrée », cela veut dire « ce que le tube
a ».

*Page traduite de la version anglaise `book/solutions/lesson-004/ex4.md`,
révision `354f350`.*

<!-- translation-source: book/solutions/lesson-004/ex4.md @ 354f350 -->
