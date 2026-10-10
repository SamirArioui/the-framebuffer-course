# Solution : exercice 4 — Une chose qui bouge

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 4 — Une chose qui bouge](../../lessons/part-0/lesson-020-timing.md) de la leçon 020.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-020/ex4.patch}}
```

## Visite guidée

Le patch ajoute une variable d'état, une ligne d'intégration dans la boucle de
ticks, et un champ dans la trace. `pos` avance de `speed * TICK_LEN` — une
cellule par seconde de temps de *jeu* — et l'exécution montre qu'il atterrit
exactement : `pos=0.10` au premier tick, `0.90` au neuvième tick après 30
frames, `1.00` au dixième tick. Parce que l'intégration a lieu par tick et que
le pas est fixe, la position est la même fonction du temps de jeu sur chaque
machine et à chaque fréquence de frames : deux millions de frames sans plafond
et trente avec plafond placent tous deux `pos` à 0,90 après neuf ticks.

C'est le motif que le serpent utilisera. Comparez avec l'intégration de
`speed * dt` par *frame* à la place : la position dépendrait alors de la façon
dont les temps de frame sont tombés, la gigue s'accumulerait en oscillation,
et un blocage de deux secondes téléporterait le serpent à travers un mur.
L'intégration par tick sur un pas fixe échange un peu de fluidité — le
mouvement est quantifié à 100 ms — contre une physique qui ne peut pas
s'emballer par rapport à l'horloge. La leçon 022 transforme ce `pos` en
cellule à l'écran.

*Page traduite de la version anglaise `book/solutions/lesson-020/ex4.md`,
révision `203c219`.*

<!-- translation-source: book/solutions/lesson-020/ex4.md @ 203c219 -->
