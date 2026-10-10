# Solution : exercice 2 — L'escalier du poursuivant

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — L'escalier du poursuivant](../../lessons/part-5/lesson-089-enemy-ai.md) de la leçon 089.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-089/ex2.patch}}
```

## Visite guidée

**La prédiction.** Le bat se tient à `(560, 72)`, le héros à `(312, 232)` : le
delta est `(−248, +160)` — le bat est à 248 px à droite et 160 px au-dessus.
`AiChase` écrit le point cardinal de ce delta : `(−1, +1)` mis à l'échelle de
1/√2 — en bas à gauche à 45°. Le poursuivant marche sur cette diagonale
jusqu'à ce que l'un des deux écarts se referme. L'écart en y est le plus court
(160 < 248), donc il se referme en premier : après 160 px de diagonale, le bat
est à `(400, 232)` — à hauteur du héros — et le delta vaut maintenant
`(−88, 0)`, dont le point cardinal est `(−1, 0)`. Le dernier segment est tout
droit vers la gauche, 88 px, jusqu'au héros. Une diagonale, un segment droit —
voilà l'escalier : il tourne exactement une fois, et seulement à un alignement.

**Ce que l'exécution montre.** La sonde imprime la direction chaque fois que le
point cardinal d'un poursuivant tourne — et il tourne *beaucoup* :

```
ai: bat steers -0.707,0.707
ai: bat steers 0.707,0.707
ai: bat steers -0.707,0.707
ai: bat steers 0.707,0.707
```

Le virage propre de la prédiction est un **vacillement** en pratique. Quand le
bat est presque à hauteur du héros, la petite composante du delta change de
signe entre les frames (un pas de 6 px de part et d'autre de zéro), et le
point cardinal bascule avec elle — le bat zigzague sur le dernier segment au
lieu de courir droit. Le monde à huit voies n'a pas de « presque droit » : une
direction est l'une des huit, chaque frame. (Les zigzags sont petits — le bat
finit à `311,176` contre le héros à `312,232`, aligné à un pixel près.)
L'escalier n'est pas non plus de la géométrie pure : la trajectoire citée par
l'exécution plie là où la carte la plie — un pas dont le x est refusé bouge
quand même en y, parce que le mover résout **x d'abord** et que le glissement
est la règle d'un axe à la fois de la leçon 084. Ce qui amène la dernière
question :

**Un mur en travers de la diagonale.** Le mover essaie x, puis y. Un mur en
travers de la diagonale refuse d'abord le pas en x ; le pas en y est encore
libre, donc le bat glisse *le long* du mur en y — l'escalier s'aplatit contre
l'obstacle. Et dans cette exécution même, le bat finit coincé à `(311, 176)`,
retenu au bord supérieur du pilier sous le héros : son pas en y est refusé (la
colonne du pilier), et l'oscillation en x du vacillement ne le porte jamais
assez loin latéralement pour dégager la colonne du pilier et descendre. C'est
la limite honnête d'un poursuivant à points cardinaux sans recherche de
chemin — il glisse, il négocie les angles, et il peut se coincer — et c'est la
réponse de la carte et du mover, pas celle du comportement : le comportement
n'écrit que des requêtes.

*Page traduite de la version anglaise `book/solutions/lesson-089/ex2.md`,
révision `d371511`.*

<!-- translation-source: book/solutions/lesson-089/ex2.md @ d371511 -->
