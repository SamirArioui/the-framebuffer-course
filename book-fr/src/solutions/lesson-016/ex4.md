# Solution : exercice 4 — Le terme d'erreur, observé

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 4 — Le terme d'erreur, observé](../../lessons/part-0/lesson-016-lines.md) de la leçon 016.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-016/ex4.patch}}
```

## Visite guidée

La trace de la diagonale jaune `(0, 0)`–`(7, 5)` lit

```
step (0,0) err=2
step (1,1) err=4
step (2,1) err=-1
step (3,2) err=1
...
```

À chaque tour, `e2 = 2 * err` est comparé à `dy` et `dx` : la première
comparaison décide si `x` avance et si l'erreur est corrigée par `dy` ; la
seconde si `y` avance et si l'erreur est corrigée par `dx`. Un tour est donc
« avancer à droite, et décider monter ou non » pour les lignes peu pentues —
deux additions entières et deux comparaisons. `err` est un entier tout le long
parce qu'il commence à `dx + dy` (la distance mise à l'échelle du *premier*
pixel, choisie pour que la toute première comparaison n'ait pas besoin de cas
spécial) et n'est jamais ajusté que par les entiers `dx` et `dy` — il n'y a ni
division ni arrondi nulle part dans la boucle, ce qui est exactement pourquoi
il n'y a pas de dérive à aucune taille de coordonnée. Où la version flottante
divergerait-elle ? À `(1, 1)` : la ligne idéale se situe à `y ≈ 0,71` quand
`x = 1`, et la troncation jette cela à `0` — tandis que la trace montre
Bresenham arrondir à la ligne 1 là, le choix exact du pixel le plus proche.

*Page traduite de la version anglaise `book/solutions/lesson-016/ex4.md`,
révision `df71716`.*

<!-- translation-source: book/solutions/lesson-016/ex4.md @ df71716 -->
