# Solution : exercice 2 — Le coût par tuile

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — Le coût par tuile](../../lessons/part-5/lesson-099-map-draw.md) de la leçon 099.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-099/ex2.patch}}
```

## Visite guidée

La sonde se place à la fin de `DrawTileMap`, après la marche : la fenêtre de
cellules qu'elle a calculée, multipliée, imprimée chaque fois que le compte
change — pour qu'une exécution montre chaque forme distincte que prend la marche
à mesure que la caméra se borne le long de la carte. D'une vraie traversée de
cette carte :

```
engine: probe: map draw 1230 of 1536 cells at offset -8,0
engine: probe: map draw 1200 of 1536 cells at offset -128,0
engine: probe: map draw 1230 of 1536 cells at offset -123,0
engine: probe: map draw 1200 of 1536 cells at offset -133,0
```

Le compte oscille entre 1 200 et 1 230 à mesure que l'offset en pixels de la
caméra décale la fenêtre d'une demi-cellule — jamais près des 1 536
d'autrefois. (L'offset passe 128 parce que l'offset additif de la secousse
rejoint la base bornée de la caméra — le hook de la leçon 092, visible ici
aussi.)

Maintenant la paire, et l'arithmétique par tuile. La ligne `tilemap` du journal
de frames sur les 641 frames de jeu de la même exécution est `0.549 ms` ; la
sonde dit que la marche a fait environ 1 215 cellules par dessin. Donc :

```
before (lesson-098):  0.981 ms / 1536 cells = 639 ns per tile
after  (this run):    0.549 ms / ~1215 cells = 450 ns per tile
```

La décomposition que l'exercice demande — quel levier possède quel nombre :

- **Le compte : 1 536 → ~1 215 cellules (−21 %)** est celui du *culling* —
  `DrawTileMap` qui ne parcourt que la fenêtre visible. C'est une coupe nette,
  quel que soit le coût de la boucle.
- **Le coût par tuile : 639 → 450 ns (−30 %)** est celui de la *boucle* — les
  pointeurs de ligne hissés et l'expansion directe. Les tuiles ici portent zéro
  pixel clé, donc chaque tuile prend le chemin sans test.

Ensemble, ils sont la baisse mesurée de 43 % de la ligne `tilemap` (`0.981 →
0.559 ms` sur l'exécution de la leçon), et les deux nombres se vérifient
indépendamment : un changement du compte sans changement du coût par tuile
aurait voulu dire que la réécriture de la boucle n'a rien fait ; l'inverse
aurait voulu dire que c'est le culling. Mesurez la paire et aucun des deux
leviers ne peut se cacher derrière l'autre.

Une note de bas de page pour vos propres sondes : celle-ci imprime au
*changement*, pas à chaque frame — une impression par frame atterrirait dans la
phase `tilemap` qu'elle cherche à mesurer (la règle de la leçon 079 : la mesure
ne doit pas gonfler la chose mesurée). Même ainsi, l'impression de la sonde est
du vrai travail dans la phase render sur les frames où elle se déclenche ; quand
vous chassez des microsecondes à un chiffre, soustrayez votre sonde ou garez-la
en dehors de la phase chronométrée.

*Page traduite de la version anglaise `book/solutions/lesson-099/ex2.md`,
révision `20f49d8`.*

<!-- translation-source: book/solutions/lesson-099/ex2.md @ 20f49d8 -->
