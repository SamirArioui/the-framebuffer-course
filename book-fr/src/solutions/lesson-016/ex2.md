# Solution : exercice 2 — Des rectangles en contour

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — Des rectangles en contour](../../lessons/part-0/lesson-016-lines.md) de la leçon 016.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-016/ex2.patch}}
```

## Visite guidée

`DrawRect` est quatre appels à `DrawLine` — haut, bas, gauche, droit — et le
contour `(-2, 1, 6, 4)` exerce chaque cas de découpage à la fois : les bords
haut et bas pendent hors du bord gauche, le bord droit est entièrement à
l'intérieur, et le bord gauche (`x = -2`, de `(−2, 1)` à `(−2, 4)`) est
entièrement hors du tampon. Le dump montre exactement cela :

```
row 1: 60 60 60 60 60 60 60 60 60 60 60 60 FF 00 FF ...
row 2: FF 80 00 FF 80 00 FF FF FF 60 60 60 FF 00 FF ...
```

— le bord haut découpé en `(0, 1)`–`(3, 1)`, le bas en `(0, 4)`–`(3, 4)`, le
bord droit à `x = 3` lignes 1-4 (ici écrasant le `(3, 2)` de la diagonale
jaune, la dernière écriture gagnant à nouveau), et le bord gauche n'a rien
écrit : ses deux extrémités portent le même code de sortie, aussi `ClipLine`
rejette-t-il le segment d'emblée. Ce rejet est le découpage qui fait son
travail — un bord invisible coûte quatre appels à `OutCode`, zéro pixel, et
zéro vérification par pixel. Composer des formes à partir de primitives
découpées veut dire qu'aucune forme composite n'a jamais besoin de maths de
découpage à elle.

*Page traduite de la version anglaise `book/solutions/lesson-016/ex2.md`,
révision `df71716`.*

<!-- translation-source: book/solutions/lesson-016/ex2.md @ df71716 -->
