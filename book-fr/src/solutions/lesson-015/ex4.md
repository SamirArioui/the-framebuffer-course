# Solution : exercice 4 — Pourquoi plier d'abord

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 4 — Pourquoi plier d'abord](../../lessons/part-0/lesson-015-fill-rect.md) de la leçon 015.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-015/ex4.patch}}
```

## Visite guidée

Les plis instrumentés impriment

```
fold left:   x=0 rw=3
fold top:    y=0 rh=2
fold right:  rw=2
fold right:  rw=3
fold bottom: rh=2
```

— quatre comparaisons bon marché par rectangle (seuls ceux qui se
déclenchent impriment), contre une vérification de limites pour chacun des
`rw * rh` pixels dans le schéma par pixel. À 10×10 c'est 4 tests contre 100 ;
à l'échelle d'un sprite le ratio ne fait que croître. Pour (b), faites
l'arithmétique avant de faire tourner le code : avec le pli `x < 0` supprimé,
le premier pixel du rectangle qui pend à gauche en `(x, y) = (-3, 1)` calcule
`off = 1 * 24 + (-3) * 3 = 15` — à l'intérieur du tampon, dans la queue de la
ligne 0. Le dump vérifié montre exactement cela : la ligne 0 gagne `FF 80 00`
aux octets 15-17, et chaque ligne saigne dans sa voisine — corruption
silencieuse, aucun plantage. Déplacez le rectangle à `y = 0` et les décalages
deviennent négatifs ; AddressSanitizer rapporte alors `heap-buffer-overflow
... WRITE of size 1` avant le tampon. Pour (c) : une vérification par pixel
gagne son salaire quand plusieurs écrivains de formes partagent un `PutPixel`
brut et que les maths de découpage de chaque forme sont complexes (sprites
pivotés, polygones) — vérifier en bas est alors l'assurance bon marché
qu'aucune forme ne peut corrompre la mémoire.

*Page traduite de la version anglaise `book/solutions/lesson-015/ex4.md`,
révision `b7d43c4`.*

<!-- translation-source: book/solutions/lesson-015/ex4.md @ b7d43c4 -->
