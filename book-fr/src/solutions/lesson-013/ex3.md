# Solution : exercice 3 — Les décalages, à la main

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 3 — Les décalages, à la main](../../lessons/part-0/lesson-013-raw-bytes.md) de la leçon 013.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-013/ex3.patch}}
```

## Visite guidée

Avec le pas `8 * 3 = 24`, la formule `y * 24 + x * 3` donne
`(0, 0) → 0`, `(1, 0) → 3`, `(7, 5) → 141`, et `(3, 2) → 57`. L'exécution
instrumentée sur stderr est d'accord pour les trois pixels que la scène
dessine :

```
PutPixel(0,0) -> offset 0
PutPixel(1,0) -> offset 3
PutPixel(7,5) -> offset 141
```

La version du pas oublié calcule `y * w + x * 3`, aussi le pixel le plus à
gauche de la ligne 1 atterrit au décalage `1 * 8 = 8` au lieu de `24` — à
l'intérieur de la ligne 0. La ligne 0 se dessine correctement (son terme
`y * w` est zéro, ce qui est exactement pourquoi le bug se cache), et chaque
ligne suivante écrit dans les lignes au-dessus d'elle. Un rectangle dessiné
de cette façon plie vers le bas sur lui-même : l'image a l'air que le tampon
a été pressé. La leçon est qu'un pas n'est pas de la décoration — c'est ce qui
rend les octets de la ligne *y* disjoints de ceux de toute autre ligne.

*Page traduite de la version anglaise `book/solutions/lesson-013/ex3.md`,
révision `66eaafc`.*

<!-- translation-source: book/solutions/lesson-013/ex3.md @ 66eaafc -->
