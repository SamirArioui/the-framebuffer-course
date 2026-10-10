# Solution : exercice 1 — La ligne verticale à travers tout

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — La ligne verticale à travers tout](../../lessons/part-0/lesson-016-lines.md) de la leçon 016.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-016/ex1.patch}}
```

## Visite guidée

La ligne va de `y = -2` à `y = 9` en `x = 4` ; le découpage vertical coupe les
deux extrémités sur le tampon et l'exécution instrumentée confirme les
extrémités :

```
DrawLine clipped -> (4,0)-(4,5)
```

(les deux premières lignes sont les deux autres lignes de la scène). Bresenham
allume alors `(4, 0)` à `(4, 5)` — un pixel par ligne — aussi les octets de
chaque ligne aux décalages 12-14 deviennent `FF 00 FF`, le triplet du canal
magenta. Les pixels volés : `(4, 2)` était blanc (le rectangle intérieur) et
`(4, 3)` était à la fois blanc *et* celui de la diagonale jaune — la ligne
tourne en dernier et gagne les six. Notez ce que le découpage a acheté : la
boucle a fait exactement six pas, pas douze, et pas un seul n'avait besoin
d'une vérification de limites. Une ligne verticale est aussi le cas sur lequel
la pente flottante naïve meurt ; Bresenham la traite comme juste une autre
ligne (`dx = 0`, seule la branche `e2 <= dx` se déclenche jamais).

*Page traduite de la version anglaise `book/solutions/lesson-016/ex1.md`,
révision `df71716`.*

<!-- translation-source: book/solutions/lesson-016/ex1.md @ df71716 -->
