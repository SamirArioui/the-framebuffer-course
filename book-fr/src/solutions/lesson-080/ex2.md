# Solution : exercice 2 — Réponse à la porte

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — Réponse à la porte](../../lessons/part-4/lesson-080-slice.md) de la leçon 080.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-080/ex2.patch}}
```

## Visite guidée

Le diff est la colonne de gauche de la porte : l'exécution nomme les services
qu'elle compose, une ligne par groupe — le même geste que la démo de la leçon
069 quand la partie 3 voulait sa table d'acceptation. Les lignes ne sont pas de
la décoration ; elles sont une affirmation par ligne, et chacune nomme le
groupe de services terminés qu'utilise la tranche.

L'exécution, depuis l'état final de cette leçon plus le patch :

```
engine: data: the archetype table loaded whole — definitions carrying their rows' values
engine: entities: one fixed store, created from the table, walked once per frame
engine: mover: the map's collision queries, one axis at a time
engine: view: the camera's clamped follow, the blit at each entity's position
engine: time: the game-time scale on the update's step, the record wall-clock at any scale
engine: sound: samples on channels through the one mixer
```

Maintenant la première moitié de la réponse : **nommez une ligne de la frame de
la tranche qui n'est pas un service terminé.** Parcourez la frame avec ces six
groupes ouverts. Le démarrage charge la table et l'art (071-073) et demande son
héros par son nom (`TableFind`, 073). L'update lit l'entrée par scrutation
(032), écrit la requête du héros (076), parcourt le magasin (075) et fait
avancer chaque entité par `MoveEntity` (077) à `request × speed × dt` où `dt`
est `GameTimeStep` (078). Le bornage de la caméra est l'arithmétique de la
leçon 054. Le render blitte chaque entité à travers la caméra (045, 076).
L'enregistrement mesure les phases et le pas (036, 079). Aucune ligne n'est
*nouvelle*, et c'est une preuve plutôt qu'une politesse à cause de la façon
dont c'est vérifiable : chaque ligne correspond à un numéro de leçon, et un
numéro de leçon correspond à un état étiqueté — vous pouvez faire `git checkout
lesson-077 -- src/` et voir le mover arriver. Une affirmation de la forme
« ceci était déjà terminé » est falsifiable ; une affirmation de la forme
« ceci est du code propre » ne l'est pas.

La seconde moitié : **si la tranche avait eu besoin d'une nouvelle pièce de
code moteur**, cette pièce serait le service qui se signale. Les modes de défaillance
sont reconnaissables. Une fonction auxiliaire `CameraFollow(camera, hero)`
signifie que la règle de la caméra n'a jamais vraiment été réglée (c'est de
l'arithmétique de jeu aujourd'hui, et c'est la bonne place — mais si le jeu en
avait besoin deux fois, c'est un service). Un cas particulier dans la marche
pour l'entrée du héros signifie que le modèle d'entité est faux : le travail
par entité doit être *exprimé une fois*, et une branche sur « est-ce le héros »
est le code par type que l'exigence d'itération existe pour empêcher (la partie
5 grandirait d'une branche par type d'ennemi). Un nouveau champ sur `Entity`
que la table ne déclare pas signifie que le modèle de données dérive du format
— le correctif est une colonne, pas un champ.

Ce que vous feriez à ce sujet avant que la partie 5 ne commence : **le mettre
dans le service, pas dans le jeu** — étendre le contrat de l'entité, le format
de la table ou le résultat du mover (l'exercice de la leçon 077 a fait
exactement cela), et revérifier la tranche avant de bâtir dessus. La porte
existe pour attraper cette lacune *maintenant*, quand son correctif coûte une
leçon, plutôt qu'en L12 quand douze leçons reposeront sur elle. La revue de
clôture (`plan/part4-review.md`) consigne la réponse propre de ce changement :
la tranche n'a rien inventé, et la seule chose qu'elle a *retirée*, c'est
l'échafaudage de la démo — la vérification de mise à mort et les scripts de
durée de vie dont les leçons 074-075 avaient besoin et dont le jeu n'a pas
besoin.

Une chose de plus que les six lignes rendent visible : les groupes ne sont pas
les fichiers. `entities` couvre `table.*`, `entity.*` et la marche dans le jeu ;
`time` couvre `gametime.*` et l'enregistrement dans `frame.*`. Un service est
ce sur quoi le *jeu* peut compter, et la liste d'acceptation est écrite dans
ces termes à dessein — c'est la liste que les leçons de la partie 5 citeront.

Rien ici ne touche la tranche, les services ni la boucle du jeu : le patch est
six lignes à côté de la ligne d'identité que la leçon imprime déjà.

*Page traduite de la version anglaise `book/solutions/lesson-080/ex2.md`,
révision `7d3e8c3`.*

<!-- translation-source: book/solutions/lesson-080/ex2.md @ 7d3e8c3 -->
