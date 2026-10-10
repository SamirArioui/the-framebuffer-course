# Solution : exercice 1 — La tuile sous le sprite

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — La tuile sous le sprite](../../lessons/part-2/lesson-053-tiles.md) de la leçon 053.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-053/ex1.patch}}
```

## Visite guidée

La correspondance inverse est une division dans chaque sens : la marche
calcule la position monde à partir de la cellule (`x + cx × 16`), la sonde
calcule la cellule à partir de la position monde (`(int)sprite_x / TILE_SIZE`).
Le rapport de démarrage la vérifie sur la position de départ du sprite —
312,232 — qui est la cellule `312 / 16 = 19`, `232 / 16 = 14` :

```
engine: tile under the sprite: cell 19,14 is kind '.' (solid 0)
```

La cellule 19,14 dans le monde de la démo est du sol — non solide, comme le
dit la table des types. (Si l'arithmétique avait besoin d'une preuve qu'elle
fonctionne : placez le sprite sur la nappe d'eau autour de la cellule 33,26 et
le rapport nomme `w`.)

Le contour est la même correspondance, dessinée : le rectangle de la cellule
va de `(cell_x × 16, cell_y × 16)` à `+15`, et un `PutPixel` jaune par bord le
marque. Pilotez le sprite avec une entrée scriptée et le contour suit —
vérifié côté fenêtre au coin du contour après une flèche droite maintenue,
sprite à `596,232` (cellule 37,14) :

```
$ DISPLAY=:99 ./winread "the framebuffer engine" 592 224
winread: 592,224 -> r=255 g=255 b=0
```

Du jaune exactement à `592 = 37 × 16`, `224 = 14 × 16` — le coin de cellule
que la correspondance prédisait.

Ce que cela offre gratuitement à la leçon 055 mérite d'être explicité, car
c'est tout le système de collision en germe : **la collision, c'est cette
correspondance plus la table des types**. « Ce rectangle est-il dans un mur ? »
devient : trouvez les cellules que le rectangle couvre (`world_x / TILE_SIZE`
sur son étendue), lisez le type de chaque cellule (`TileAt`), interrogez
l'indicateur `solid` du type. L'arithmétique monde → cellule, le `TileAt` sûr
vis-à-vis des limites et les données de solidité sont déjà écrits et vérifiés
— la leçon 055 les assemble en les deux requêtes que la logique de jeu
exécute. Rien dans la collision n'a besoin de toucher aux pixels, au blitter
ou à l'écran : ce sont des données sur des données, et c'est exactement
pourquoi la spécification voulait une carte testable sans fenêtre.

*Page traduite de la version anglaise `book/solutions/lesson-053/ex1.md`,
révision `2ac62b8`.*

<!-- translation-source: book/solutions/lesson-053/ex1.md @ 2ac62b8 -->
