# Solution : exercice 3 — L'image retournée

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 3 — L'image retournée](../../lessons/part-0/lesson-017-image-file.md) de la leçon 017.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-017/ex3.patch}}
```

## Visite guidée

Retourner la boucle fait stocker à l'écrivain la ligne d'image 0 en premier.
`file` ne s'en aperçoit pas — `PC bitmap ... 8 x 6 x 24` — parce que chaque
champ d'*en-tête* est toujours correct. Les pixels la trahissent : décodée
avec la règle standard bas en haut, l'échantillonneur montre la scène à
l'envers —

```
pixel (1,0): (32, 32, 32) pixel (7,5): (0, 255, 255)
pixel (1,5): (0, 255, 0) pixel (7,0): (255, 255, 0)
```

— le pixel vert a déménagé vers la ligne du bas et l'extrémité jaune de la
diagonale vers le coin supérieur droit. L'en-tête ment sur exactement un bit :
un `biHeight` positif *veut dire* des lignes stockées bas en haut, et les
nôtres ne le sont plus. Un BMP haut en haut légitime stocke le champ hauteur
comme un nombre *négatif* (ses octets en complément à deux via le même
`PutU32LE`), que les lecteurs interprètent comme « des lignes dans l'ordre
haut en bas ». La bizarrerie survit parce que le format précède quiconque se
pose la question — Windows écrivait les lignes de balayage bas en haut dans ses
framebuffers, aussi le fichier faisait-il de même, et quarante ans de lecteurs
le supposent désormais. Les règles de format font partie des données : les
bons octets dans le mauvais ordre sont les mauvais octets. Restaurez la boucle
ensuite.

*Page traduite de la version anglaise `book/solutions/lesson-017/ex3.md`,
révision `c6657c1`.*

<!-- translation-source: book/solutions/lesson-017/ex3.md @ c6657c1 -->
