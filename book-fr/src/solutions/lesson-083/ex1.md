# Solution : exercice 1 — La caméra qui anticipe

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — La caméra qui anticipe](../../lessons/part-5/lesson-083-tilemap-camera.md) de la leçon 083.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-083/ex1.patch}}
```

## Visite guidée

La caméra vise déjà à centrer le héros : `base = hero + half sprite − half frame`.
L'anticipation est un terme de plus — poussez la visée un peu dans la direction
où le héros se déplace :

```cpp
int base_x = (int)hero.x + hero.sprite->width / 2 - FRAME_WIDTH / 2 +
             (int)(hero.move_x * GAME_LOOKAHEAD);
```

`hero.move_x` / `hero.move_y` est la requête de déplacement du héros, la même
direction scrutée que la marche utilise — `-1`, `0` ou `1` par axe. Multipliée
par `GAME_LOOKAHEAD` (48 pixels), elle décale la visée de la caméra de 48 pixels
vers là où va le héros. Un déplacement diagonal anticipe en diagonale. Au repos,
la requête est zéro, donc la caméra se recentre — elle n'anticipe que pendant
que le héros se déplace réellement.

Le bornage n'est pas touché et s'applique toujours à la base *anticipée*, donc
la caméra anticipe et ne montre toujours jamais au-delà du bord de la carte. Et
le décalage additif reste au repos — l'anticipation déplace la base, pas le hook
du juice.

L'exécution, depuis l'état final de cette leçon plus le patch — le héros
maintient Right, puis relâche :

```
engine: hero at 312,232
engine: hero at 389,232 (t=3.693)
engine: camera base 125,0
engine: camera base 77,0
```

Lisez-la contre l'arithmétique. En allant à droite au héros `389`, la base
centrée serait `389 + 8 − 320 = 77`. La base anticipée affiche `125` — c'est
`77 + 48`, l'anticipation poussant la vue devant le héros. Le héros est à gauche
du centre pendant qu'il court à droite : le joueur voit plus de là où il va.
Puis le héros s'arrête (`move_x` revient à 0) et la base retombe à `77` —
recentrée. Anticipation en mouvement, centrage au repos.

Un jugement que l'exercice vous laisse : jusqu'où anticiper. `GAME_LOOKAHEAD = 48`
fait trois tuiles — assez pour voir une tuile en avant sans que la caméra donne
l'impression de courir après le héros. Trop grand, et le héros dérive vers le
bord de l'écran ; trop petit, et l'anticipation est invisible. Exécutez, observez
la place du héros à l'écran, et choisissez le nombre avec lequel votre jeu sonne
juste — c'est une constante, et elle est à vous.

*Page traduite de la version anglaise `book/solutions/lesson-083/ex1.md`,
révision `7f209d4`.*

<!-- translation-source: book/solutions/lesson-083/ex1.md @ 7f209d4 -->
