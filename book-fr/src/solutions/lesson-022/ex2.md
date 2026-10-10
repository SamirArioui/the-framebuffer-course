# Solution : exercice 2 — Un de trop, deux fois

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — Un de trop, deux fois](../../lessons/part-0/lesson-022-double-buffer.md) de la leçon 022.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-022/ex2.patch}}
```

## Visite guidée

C compte à partir de zéro ; l'adressage de curseur ANSI non. `ESC [ r ; c H`
utilise des coordonnées en base 1 — la ligne 1 est la ligne du haut, ligne
1;colonne 1 est le coin supérieur gauche — aussi le vidage peignait-il toute
la grille une cellule plus bas et à droite de ses coordonnées, avec
`ESC [ 0 ; 0 H` comme aveu : zéro n'est pas une position légale du tout, et la
plupart des terminaux le brident à 1;1. La dernière ligne de la grille
échappait à l'attention seulement parce que le terminal bride aussi le
débordement.

La correction est la paire classique de `+ 1` à la frontière où le monde en
base zéro du C se traduit dans celui en base 1 du terminal — `row + 1, col + 1`
— et rien à l'intérieur des coordonnées propres du programme ne change. Après
la correction, la première cellule écrit `ESC [ 1 ; 1 H`, le coin inférieur
droit écrit `ESC [ 20 ; 40 H` pour une grille de 20×40, et le marqueur en
(10, 20) interne écrit `ESC [ 11 ; 21 H`. La leçon se généralise : chaque
interface avec sa propre convention de coordonnées ou d'indexation a besoin
d'un point de traduction, gardé évident — les bugs d'un-de-trop se cachent
exactement à cette sorte de couture.

*Page traduite de la version anglaise `book/solutions/lesson-022/ex2.md`,
révision `eea0461`.*

<!-- translation-source: book/solutions/lesson-022/ex2.md @ eea0461 -->
