# Solution : exercice 2 — Le répertoire silencieux

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — Le répertoire silencieux](../../lessons/part-0/lesson-001-first-program.md) de la leçon 001.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-001/ex2.patch}}
```

## Visite guidée

Ouvrir un répertoire avec `fopen` réussit sur ce système — l'échec arrive plus
tard, à la première lecture. La boucle de lecture ne peut pas le voir :
`fgetc` renvoie `EOF` à la fois pour « le fichier est terminé » et pour « la
lecture a échoué », et ces deux fins se ressemblent dans la condition de
boucle. Le flux connaît la différence, et `ferror(f)` est la façon de le
demander : il rapporte si une erreur de lecture ou d'écriture est survenue sur
le flux.

La correction teste `ferror` après la boucle, avant que quoi que ce soit soit
affiché. Un fichier qui s'est terminé normalement obtient sa ligne de compteur
comme avant ; un fichier dont la lecture a échoué obtient un message
`cannot read` sur `stderr` à la place, et la boucle continue avec le fichier
suivant. Le `fclose(f)` avant `continue` est la même discipline de ressources
que partout ailleurs : un fichier dont la lecture a échoué est toujours un
fichier ouvert.

Exécutez `./wordcount .` avec la correction et la plainte sur le répertoire
apparaît ; exécutez `./wordcount wordcount.c .` et une ligne de compteur et
une ligne de plainte sortent ensemble. La fonction compagne `feof(f)` répond à
l'autre question — « la boucle s'est-elle arrêtée parce que le fichier était
terminé ? » — qui est celle que vous voulez quand une lecture tronquée serait
un bug.
*Page traduite de la version anglaise `book/solutions/lesson-001/ex2.md`, révision `57b9b3e`.*

<!-- translation-source: book/solutions/lesson-001/ex2.md @ 57b9b3e -->
