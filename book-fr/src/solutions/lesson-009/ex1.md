# Solution : exercice 1 — L'ordre inversé

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — L'ordre inversé](../../lessons/part-0/lesson-009-function-pointers.md) de la leçon 009.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-009/ex1.patch}}
```

## Visite guidée

La prédiction : `sorted by key:` sort dans l'ordre alphabétique inverse — et
c'est exactement ce que la machine imprime :

```
sorted by key:
pear 0
lemon 9
kiwi 8
grape 7
fig 2
elder 6
date 5
cherry 4
banana 3
apple 1
```

La subtilité demandée dans l'énoncé : c'est *précisément* le bloc inversé,
parce que les dix clés sont distinctes. Échanger les arguments transforme le
comparateur en son image miroir, ce qui reste un ordre total cohérent —
seulement un ordre en miroir — aussi le contrat de `qsort` est-il satisfait et
y a-t-il exactement une permutation triée valide à produire. Le bloc
`sorted by value:` s'inverse aussi, pour la même raison : le pont est partagé
par chaque tri.

Si deux éléments avaient partagé une clé, « simplement inversé » ne serait
plus garanti : `qsort` n'est pas un tri stable, et parmi les éléments égaux
tout ordre qui satisfait le comparateur est légal. La leçon à l'intérieur de
la prédiction : un comparateur ne définit totalement la réponse que lorsque
ses clés sont uniques.

*Page traduite de la version anglaise `book/solutions/lesson-009/ex1.md`,
révision `333e81a`.*

<!-- translation-source: book/solutions/lesson-009/ex1.md @ 333e81a -->
