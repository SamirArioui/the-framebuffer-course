# Solution : exercice 1 — Le coin, prédit

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — Le coin, prédit](../../lessons/part-4/lesson-077-mover.md) de la leçon 077.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-077/ex1.patch}}
```

## Visite guidée

Le diff est un coin jetable : l'entité du héros copiée, garée en `736,464` —
l'intérieur en bas à droite de la carte — et trois pas forcés de 20 pixels par
axe à travers `MoveEntity` lui-même. Le coin est de la vraie donnée de carte :
la cellule 47 (à droite) est un `#` et la ligne 31 (en dessous) est un `#`,
tandis qu'en haut et à gauche se trouvent les lignes ouvertes que le héros
parcourt depuis le début.

Les prédictions, avant toute exécution. **Bas-Droite** (+20, +20) : le pas en x
mettrait le rectangle sur la cellule 47 — refusé ; le pas en y le mettrait sur
la ligne 31 — refusé. Rien ne bouge : `736,464`. **Bas-Gauche** (−20, +20) : le
pas en x aboutit (716 est ouvert), le pas en y est encore refusé à la ligne du
bas : `716,464`. **Haut-Droite** (+20, −20) : le pas en x est refusé à la
cellule 47, le pas en y aboutit : `736,444`.

Les exécutions, depuis l'état final de cette leçon plus le patch :

```
engine: corner: Down-Right -> 736,464
engine: corner: Down-Left  -> 716,464
engine: corner: Up-Right   -> 736,444
```

Exactement comme prédit — et notez ce que les trois réponses donnent ensemble :
le coin refuse des *composants*, pas des requêtes. Une diagonale vers un coin
ne rebondit pas et ne s'arrête pas net ; chaque axe prend ce que le monde
offre.

Maintenant la question de l'ordre. Le mover essaie **x d'abord, puis y** — et
le test en y s'exécute au x *mis à jour* quand le pas en x a abouti. Cela
compte dans un coin : dans le cas Bas-Gauche, le test en y s'est exécuté à
x 716, pas à 736. Si le pas en x avait fait glisser l'entité au-delà du bord du
coin — un mur qui se termine en pleine course — le pas en y aurait pu aboutir
au nouveau x là où il aurait été refusé à l'ancien. C'est le cas du
« glissement autour d'un coin » : l'ordre décide si le héros se glisse autour
de la lèvre du coin ou y reste collé. Y d'abord répondrait différemment aux
mêmes situations.

Aucun des deux ordres n'est *juste* dans l'abstrait ; le choix fait partie du
contrat du mover et celui de ce moteur est x d'abord, comme l'était le mover en
ligne de la leçon 056. Ce qui compte, c'est qu'il s'agit d'une seule fonction
avec un seul ordre — pour que chaque entité du jeu glisse autour des coins de
la même façon, et que le jour où le ressenti demandera l'autre ordre, il change
à un seul endroit.

Rien ici ne touche le mover, la marche ou la boucle du jeu : la sonde est une
entité copiée et trois appels à côté de ceux de l'exécution.

*Page traduite de la version anglaise `book/solutions/lesson-077/ex1.md`,
révision `c8d0b9c`.*

<!-- translation-source: book/solutions/lesson-077/ex1.md @ c8d0b9c -->
