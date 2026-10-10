# Solution : exercice 2 — Chaque définition, une entité

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — Chaque définition, une entité](../../lessons/part-4/lesson-073-rows.md) de la leçon 073.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-073/ex2.patch}}
```

## Visite guidée

À la requête de `hero` par son nom, le diff substitue une marche sur la table :
un `EntityFromDef` par ligne, une ligne de rapport par entité. La vérification
`TableFind(table, "dragon")` de l'exécution reste — l'échec typé ne porte pas
sur *combien* de définitions le jeu veut, il porte sur la demande d'une
définition que la table ne contient pas.

L'exécution, de l'état final de cette leçon plus le patch :

```
engine: table: 2 definitions
engine: def hero: x 312 y 232 facing 0 speed 240 health 3 sprite assets/sprite.ppm
engine: def slime: x 400 y 320 facing 2 speed 96 health 1 sprite assets/sprite.ppm
engine: entity hero: x 312 y 232 facing 0 speed 240 health 3 sprite 16x16
engine: entity slime: x 400 y 320 facing 2 speed 96 health 1 sprite 16x16
engine: table: "dragon" -> unknown
```

Maintenant la partie qui rend l'exercice digne d'être fait. Ajoutez une ligne à
`assets/entities.txt` —

```
bat 96 320 1 120 2 assets/sprite.ppm
```

— et relancez **sans recompiler** :

```
engine: entity hero: x 312 y 232 facing 0 speed 240 health 3 sprite 16x16
engine: entity slime: x 400 y 320 facing 2 speed 96 health 1 sprite 16x16
engine: entity bat: x 96 y 320 facing 1 speed 120 health 2 sprite 16x16
engine: table: "dragon" -> unknown
```

Qu'est-ce qui, dans le code de l'exécution, a changé pour faire apparaître la
chauve-souris ? **Rien.** La source du moteur n'a été ni touchée ni recompilée ;
les faits de la troisième entité — sa position, son `facing`, sa vitesse, sa
vie — venaient d'un fichier. C'est l'exigence pour laquelle toute cette partie
existe, rendue visible en deux exécutions : *changer les valeurs d'une table
change le comportement du jeu sans recompiler le moteur*, et le code du moteur
lui-même ne contient aucune copie par type de ces valeurs qui pourrait se
périmer. Cherchez la vitesse de la chauve-souris dans `src/` : elle n'y est pas.

Deux choses que cet exercice ne supprime **pas**. `TableFind` a toujours sa
place — le jeu veut la ligne `hero` en particulier quand le joueur reçoit une
entité à contrôler, et « une de chaque » n'est pas cette requête. Et le chemin
d'échec de la requête par nom vaut toujours la peine d'être dans l'exécution :
une table dont la ligne `hero` manque est un échec nommé, pas une exécution qui
se fabrique son propre héros. La boucle et la recherche sont deux questions
différentes sur la même donnée.

Où cela va ensuite, c'est la leçon 074 : deux locales et une boucle sont très
bien pour un rapport et inutiles pour un jeu. Les entités que la boucle crée
tombent par terre — il n'y a nulle part où les *garder* — et le magasin fixe est
exactement la réponse à « la table liste les types ; où vivent les vivantes ? »

Rien ici ne touche le chargeur ni l'analyse : la sonde est une boucle qui
remplace une recherche.

*Page traduite de la version anglaise `book/solutions/lesson-073/ex2.md`,
révision `df91188`.*

<!-- translation-source: book/solutions/lesson-073/ex2.md @ df91188 -->
