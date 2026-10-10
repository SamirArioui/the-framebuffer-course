# Solution : exercice 2 — Compter ce qui a survécu

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — Compter ce qui a survécu](../../lessons/part-0/lesson-015-fill-rect.md) de la leçon 015.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-015/ex2.patch}}
```

## Visite guidée

Les rectangles découpés sont `3×3`, `2×2`, `3×2`, et `3×2`, aussi les
compteurs calculés à la main sont 9, 4, 6, et 6 — et le programme est
d'accord :

```
rect off the left:        9 pixels
rect off the top-right:   4 pixels
rect off the bottom-right: 6 pixels
rect fully inside:        6 pixels
rect entirely outside:    0 pixels
pixels written: 25
```

Le changement lui-même est petit : `FillRect` renvoie `rw * rh` après les
boucles d'écriture, et le chemin vide renvoie 0. La valeur de retour est
calculée *après* le pli, ce qui fait qu'elle compte ce qui a atterri sur le
tampon et pas ce qui a été demandé — `(5, 4, 10, 10)` a demandé 100 pixels et
en a écrit 6. Cette distinction est exactement ce qui rend le retour utile :
c'est l'aire visible, et le rectangle entièrement extérieur rapporte 0 sans
toucher un seul octet. Compter après le découpage est aussi la façon dont un
jeu budgète ses appels de dessin — les pixels qui ont survécu sont les pixels
que la machine doit payer.

*Page traduite de la version anglaise `book/solutions/lesson-015/ex2.md`,
révision `b7d43c4`.*

<!-- translation-source: book/solutions/lesson-015/ex2.md @ b7d43c4 -->
