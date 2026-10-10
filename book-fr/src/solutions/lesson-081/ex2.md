# Solution : exercice 2 — Ce que la ligne ne dit pas

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — Ce que la ligne ne dit pas](../../lessons/part-4/lesson-081-entities-row.md) de la leçon 081.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-081/ex2.patch}}
```

## Visite guidée

Le diff est le nombre propre de l'update imprimé à côté du travail d'entités
qu'il contient — la frontière de la ligne, rendue visible sous forme
d'arithmétique.

L'exécution, depuis l'état final de cette leçon plus le patch :

```
engine: frame budget — 73 frames, avg 1.750 ms, worst 2.415 ms (frame 16)
engine: update: 0.010 ms a frame of wall clock, 0.001 ms of it entity work — the rest is input, the camera, and the reports
```

L'update coûte 0.010 ms par frame et 0.001 ms de cela est la marche. Les
0.009 ms restants sont ce dont l'exercice parle.

**Ce qui se trouve dans `update` en dehors d'`entities`, dans la frame de ce
jeu** : la lecture de l'entrée par scrutation (quatre appels à `KeyDown` et
l'écriture de la requête du héros), l'arithmétique du score, le rapport d'état
du mover, le bornage de la caméra et son rapport de changement, le compte à
rebours de la secousse, et la comparaison du script d'échelle. Chacun de ces
éléments tourne *une fois par frame*, pas une fois par entité — aucun ne
coûterait plus si le magasin contenait cinquante entités au lieu de deux. C'est
la justification de la frontière en une ligne : le rôle de la ligne est de
répondre à « combien coûte le fait de contenir des entités ? », et le travail
qui ne croît pas avec le nombre d'entités n'appartient pas à la réponse.

**Le test de résistance.** Les tentations sont réelles, et elles se résolvent
toutes de la même façon. La recherche de chemin est du travail par entité — et
appartient au corps de la marche, chronométrée par cette ligne (son coût *est*
le coût de contenir des entités qui cherchent leur chemin). L'animation est par
entité — même réponse. Les déclencheurs de son actionnés par entité — pareil.
Et le travail par entité mais bon marché, comme la mise à jour du `facing`
déjà dans la marche ? Il est déjà à l'intérieur de la ligne, et c'est correct : la
ligne est le temps *de la marche*, pas « le temps des parties de la marche que
quelqu'un juge intéressantes ». Et le cas inverse — un système par *frame* qui
lit toutes les entités (un générateur de vagues, une grille de collisions) ?
Ce n'est pas la marche ; c'est un système de jeu, et s'il croît avec le nombre
d'entités, le correctif honnête est de *lui* donner un nom à lui à l'intérieur
d'`update` (le geste de la leçon 046, de nouveau disponible) plutôt que
d'étirer cette ligne au-delà de sa définition.

**Ce qu'une ligne de 0.001 ms ne peut pas vous dire.** Elle ne peut pas vous
dire ce que coûte le travail d'entités quand c'est du *vrai* travail — deux
entités qui avancent à travers un mover, ce n'est pas cinquante entités qui
font tourner une IA, et le nombre par entité ne se transfère pas de l'un à
l'autre. Elle ne peut pas vous dire quoi que ce soit des *distributions* — la
ligne moyenne cache la frame où toutes les entités ont touché un mur en même
temps. Et elle ne peut pas vous dire quoi que ce soit de la machine : 0.001 ms
ici est une mesure de ce CPU à `-O0` sous Xvfb, et le même jeu sur l'ordinateur
portable d'un joueur est un autre nombre. Ce qu'il faut mesurer à la place,
quand on veut ces réponses : la ligne de la *pire* frame (l'enregistrement
nomme déjà la pire frame — lisez sa ligne), la ligne aux nombres d'entités
réels du jeu sous la charge réelle du jeu, et tout cela sur les machines que le
jeu cible.

C'est la place de la ligne dans la discipline : pas un verdict, une *colonne* —
à lire à côté du nombre de frames, de la machine et du jeu qui l'a produite.

Rien ici ne touche l'enregistrement, la marche ni l'arithmétique du budget : le
patch est une ligne à côté de la table que la leçon imprime déjà.

*Page traduite de la version anglaise `book/solutions/lesson-081/ex2.md`,
révision `15b1166`.*

<!-- translation-source: book/solutions/lesson-081/ex2.md @ 15b1166 -->
