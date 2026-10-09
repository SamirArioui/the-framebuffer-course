# Solution : exercice 3 — La lettre minuscule

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 3 — La lettre minuscule](../../lessons/part-0/lesson-012-multi-file.md) de la leçon 012.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-012/ex3.patch}}
```

## Visite guidée

La prédiction : `nm` rapporte `HtHash` avec un `T` majuscule au lieu du `t`
minuscule, et rien d'autre ne change — le programme se construit et tourne
exactement comme avant. Les deux prédictions tiennent. Avant :

```
0000000000000000 t HtHash
```

Après suppression du seul mot-clé :

```
0000000000000000 T HtHash
```

La lettre est la vue du nom par l'éditeur de liens. `t` veut dire que le
symbole est défini dans cet objet mais gardé **local** — lien interne,
invisible pour l'appariement de l'éditeur de liens, exactement ce que
promet `static`. `T` veut dire que la définition est exportée — lien externe
— et l'éditeur de liens appariera désormais les promesses `U HtHash` des
autres unités de traduction contre elle. Le comportement du programme est
inchangé parce que la liaison porte sur la *visibilité des noms*, pas sur ce
que fait le code : le code machine de `HtHash` a toujours été là, toujours
appelé depuis ce fichier seulement.

Ce qui a changé est la surface de collision : toute unité de traduction du
programme pourrait désormais appeler `HtHash` — ou en définir un sien et
mourir à l'étape de liaison exactement comme le `ReportOOM` de l'exercice 1.
C'est l'argument pour `static`-par-défaut vu de l'autre côté : chaque nom
exporté est une promesse sur l'espace de noms de votre programme, et la
promesse la moins chère à tenir est celle qu'on n'a jamais faite.

*Page traduite de la version anglaise `book/solutions/lesson-012/ex3.md`,
révision `e5cc4fc`.*

<!-- translation-source: book/solutions/lesson-012/ex3.md @ e5cc4fc -->
