# Solution : exercice 1 — La diagonale est trop rapide

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — La diagonale est trop rapide](../../lessons/part-1/lesson-035-clock.md) de la leçon 035.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-035/ex1.patch}}
```

## Visite guidée

Le bug : quatre `if` indépendants ajoutent `MARKER_SPEED * dt` à *chaque* axe
actif, si bien qu'une frame en diagonale déplace le marqueur de `speed × dt`
en x **et** d'autant en y — soit `speed × √2 × dt` par seconde au total. La
diagonale est 41 pour cent plus rapide que toutes les autres directions, et le
chemin le plus rapide à travers l'écran passe toujours par un coin.

La correction traite le vecteur de direction comme une seule direction :
lisez les axes dans des composantes, et divisez par la longueur du vecteur
avant d'appliquer la vitesse. Les quatre touches ne peuvent produire qu'un
déplacement aligné sur un axe ou à 45 degrés, donc la longueur vaut 1 ou √2 —
pas besoin de `sqrt`, et la constante est écrite en toutes lettres pour ce
qu'elle est.

Maintien déterministe, une seconde dans chaque sens (un auxiliaire qui presse,
attend exactement 1000 ms, relâche) :

```
axis:     engine: marker at 548,228 (t=1.001)   moved 240 px
diagonal: engine: marker at 478,398 (t=1.002)   moved 170 px on each axis
```

Même temps, même vitesse : le déplacement de la course en diagonale vaut
`√(170² + 170²) ≈ 240` pixels — exactement les 240 de la course sur l'axe. Le
marqueur avance désormais à `MARKER_SPEED` dans *toutes* les directions, ce
qui est bien ce qu'une constante de vitesse est censée vouloir dire.

*Page traduite de la version anglaise `book/solutions/lesson-035/ex1.md`,
révision `9b3ec3d`.*

<!-- translation-source: book/solutions/lesson-035/ex1.md @ 9b3ec3d -->
