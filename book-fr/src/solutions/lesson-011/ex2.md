# Solution : exercice 2 — Le classement

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — Le classement](../../lessons/part-0/lesson-011-hashtable.md) de la leçon 011.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-011/ex2.patch}}
```

## Visite guidée

Le mode ajoute une seconde politique d'ordre et un point d'arrêt. Le nouveau
comparateur, `CmpByCountThenKey`, ordonne par compteur — le plus fréquent en
premier — et retombe sur `strcmp` sur les clés dès que les compteurs sont à
égalité. Ce repli est ce qui rend la sortie reproductible : deux entrées ne
peuvent jamais être égales sur *les deux* clés, aussi le comparateur ne
renvoie-t-il jamais zéro et la permutation triée est-elle unique —
l'instabilité de `qsort` ne peut pas fuir au travers. De vraies exécutions :

```
$ ./ds-kit --top 3 shakespeare.txt
to 3
be 2
the 2
```

`be` bat `the` parce que les deux ont un compteur de 2 et que `be` vient en
premier dans l'ordre alphabétique. Sur un fichier où tout est à égalité à 1,
le classement est alphabétique :

```
$ ./ds-kit --top 3 two-lines.txt
the 3
and 1
bird 1
```

L'arrêt de l'impression est une simple boucle sur `DaAt` jusqu'à `N`
(bornée à `len`), parce que `DaEach` parcourt tout — des hooks d'itération
avec sortie anticipée auraient besoin d'un retour « continuer ? », une autre
idée en forme de callback. Le mode simple n'est pas touché : même
comparateur, même `DaEach`, mêmes lignes. Une note de parsing : `strtol`
gagne son salaire face à `atoi` en pouvant rejeter une mauvaise entrée —
l'exercice garde la forme simple, mais tout ce qui durerait plus longtemps
devrait vérifier où le nombre s'est arrêté.

*Page traduite de la version anglaise `book/solutions/lesson-011/ex2.md`,
révision `11ce1ee`.*

<!-- translation-source: book/solutions/lesson-011/ex2.md @ 11ce1ee -->
