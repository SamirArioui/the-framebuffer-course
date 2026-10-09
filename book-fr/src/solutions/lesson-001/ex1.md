# Solution : exercice 1 — L'entrée standard

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — L'entrée standard](../../lessons/part-0/lesson-001-first-program.md) de la leçon 001.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-001/ex1.patch}}
```

## Visite guidée

L'entrée standard n'est pas un concept nouveau en C — c'est un `FILE *` déjà
ouvert quand le programme démarre. La correction remplace la branche
d'utilisation par la même boucle de comptage que le chemin des fichiers, en
lisant depuis `stdin` au lieu d'une poignée de `fopen`, et en affichant le
compteur nu puisqu'il n'y a pas de nom de fichier pour l'étiqueter.
`./wordcount < notes.txt` fonctionne désormais, et de même un tube :
`ls | ./wordcount`.

La duplication entre cette boucle et celle de la boucle des fichiers est
délibérée à ce stade du programme : toutes deux font cinq lignes que vous
pouvez lire d'un bout à l'autre. La leçon 002 tire la boucle des fichiers
dans une fonction à elle — pour une raison de débogueur, de toutes les
choses. Quand vous y serez, le chemin `stdin` que vous avez écrit ici peut
devenir un appelant d'une ligne de cette même fonction.

Une décision mérite d'être nommée : quand un argument de fichier *est*
donné, `stdin` est entièrement ignoré. C'est ce que fait `wc`, et le
reproduire garde le programme prévisible dans les pipelines.
*Page traduite de la version anglaise `book/solutions/lesson-001/ex1.md`, révision `bb8d4ab`.*

<!-- translation-source: book/solutions/lesson-001/ex1.md @ bb8d4ab -->
