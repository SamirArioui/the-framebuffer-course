# Solution : exercice 1 — Le rectangle hors du bord gauche

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — Le rectangle hors du bord gauche](../../lessons/part-0/lesson-015-fill-rect.md) de la leçon 015.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-015/ex1.patch}}
```

## Visite guidée

Le rectangle `(-3, 1, 6, 3)` fait pendre trois colonnes hors du bord gauche.
Le pli gauche retire exactement ces trois colonnes : `rw += x` fait passer la
largeur de 6 à 3, et `x` bouge à 0. Les trois autres plis ne trouvent rien à
faire (le bord droit atterrit à 3 ≤ 8, le bas à 4 ≤ 6), aussi la partie visible
est `x=0 y=1 w=3 h=3` — et l'exécution instrumentée le confirme :

```
FillRect visible part: x=0 y=1 w=3 h=3
```

La ligne 1 commence donc par l'orange `(255, 128, 0)` trois fois :

```
row 1: FF 80 00 FF 80 00 FF 80 00 00 00 00 ...
```

Le rectangle perdant est le pixel bleu en `(7, 5)` : le rectangle inférieur
droit `(5, 4, 10, 10)` se découpe en `x=5 y=4 w=3 h=2`, ce qui couvre `(7, 5)`,
et il tourne après l'appel à `PutPixel` qui a planté le bleu — les écritures
récentes écrasent les précédentes, ce qu'est toute « superposition » dans un
framebuffer.

*Page traduite de la version anglaise `book/solutions/lesson-015/ex1.md`,
révision `b7d43c4`.*

<!-- translation-source: book/solutions/lesson-015/ex1.md @ b7d43c4 -->
