# Solution : exercice 3 — L'addition qui a mangé le découpage

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 3 — L'addition qui a mangé le découpage](../../lessons/part-0/lesson-015-fill-rect.md) de la leçon 015.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-015/ex3.patch}}
```

## Visite guidée

Avec l'appel fautif ajouté, le programme non corrigé meurt immédiatement —
`Segmentation fault (core dumped)`, code de sortie 139. `x = 2147483640` et
`rw = 100` font déborder `x + rw` l'`int` ; la somme retombée est négative, le
test `x + rw > w` sort faux, le pli ne fait rien, et la boucle d'écriture
pousse `PutPixel` vers des décalages loin au-delà du tampon. La correction
retire l'addition du test : `rw > w - x` pose la même question — « le
rectangle atteint-il au-delà de `w` ? » — en soustrayant à la place, et
`w - x` est sûr parce que `x` est déjà connu comme non négatif (le pli de
départ a tourné avant), aussi `w - x` tient-il bien dans `int`. Le même appel
calcule désormais `rw = 8 - 2147483640`, une largeur négative, et le cas vide
rentre sans une seule écriture. Gardez la forme de ce bug : « calculer
`x + n`, comparer, brider » est un appât à débordement dès qu'un appelant
contrôle `x`, et la leçon 018 montre ce que l'optimiseur fait des appâts à
débordement même quand ils ne plantent pas.

*Page traduite de la version anglaise `book/solutions/lesson-015/ex3.md`,
révision `b7d43c4`.*

<!-- translation-source: book/solutions/lesson-015/ex3.md @ b7d43c4 -->
