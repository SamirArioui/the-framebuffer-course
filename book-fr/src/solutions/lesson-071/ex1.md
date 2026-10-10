# Solution : exercice 1 — L'en-tête, réordonné

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — L'en-tête, réordonné](../../lessons/part-4/lesson-071-table.md) de la leçon 071.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-071/ex1.patch}}
```

## Visite guidée

Le diff, ce sont deux fichiers et une refactorisation : `assets/reordered.txt`
(les deux définitions sous un en-tête dans un ordre différent) et
`assets/swapped.txt` (les mêmes, avec le `3` et le `240` de la ligne du héros
échangés), à côté de la table propre du cours. Le rapport sort de `Run` pour
aller dans `PrintTable` — même ligne, mêmes champs — si bien qu'une seule
exécution imprime les trois tables et que la différence est une comparaison de
trois blocs de sortie au lieu de trois exécutions.

La prédiction, avant toute exécution. Le chargeur ne lit pas « la première
valeur est le nom » : il lit l'en-tête, trouve quel champ chaque colonne nomme,
et remplit les champs dans l'ordre de l'en-tête. Donc dans
`assets/reordered.txt` la ligne du héros `240 hero assets/sprite.ppm 3 232 0 312`
atterrit exactement là où atterrit la ligne d'origine — `240` est sous `speed`,
`3` est sous `health`, `232` est sous `y` — et le rapport doit être *identique*,
ligne pour ligne, au rapport de `assets/entities.txt`. Mêmes champs, mêmes
valeurs, rien de réordonné dans la sortie parce que la sortie imprime des
champs, pas des colonnes.

La ligne échangée est celle à laquelle penser deux fois.
`3 hero assets/sprite.ppm 240 232 0 312` est une ligne bien formée : sept
valeurs, `3` et `240` sont tous deux des nombres entiers, le `facing` est
toujours 0, le nom est toujours unique dans le fichier. Le chargeur n'a pas
d'opinion sur la question de savoir si une vie de 240 est *plausible* — c'est
l'affaire du jeu, pas celle du format. Donc rien n'échoue, l'exécution rapporte
`speed 3 health 240`, et le héros du rapport est plus lent et bien plus
résistant.

Les exécutions, de l'état final de cette leçon plus le patch :

```
engine: table: 2 definitions
engine: def hero: x 312 y 232 facing 0 speed 240 health 3 sprite assets/sprite.ppm
engine: def slime: x 400 y 320 facing 2 speed 96 health 1 sprite assets/sprite.ppm
engine: reordered: 2 definitions
engine: def hero: x 312 y 232 facing 0 speed 240 health 3 sprite assets/sprite.ppm
engine: def slime: x 400 y 320 facing 2 speed 96 health 1 sprite assets/sprite.ppm
engine: swapped: 2 definitions
engine: def hero: x 312 y 232 facing 0 speed 3 health 240 sprite assets/sprite.ppm
engine: def slime: x 400 y 320 facing 2 speed 96 health 1 sprite assets/sprite.ppm
```

La première prédiction tient exactement : le rapport de la table réordonnée est
identique caractère pour caractère. La seconde tient comme prévu : les valeurs
échangées sont portées par leurs colonnes, et la seule chose que le chargeur
garantit est que chaque valeur est ce que sa colonne *requiert* — un nombre
entier là où un nombre est requis — pas ce que le jeu voudrait y voir.

Cette distinction est le point de l'exercice. Les vérifications du format sont
syntaxiques et bon marché — types, comptes, unicité, bornes — et ce sont
exactement les vérifications qu'une analyse peut faire sans savoir ce qu'est un
jeu. Une valeur bien typée mais fausse pour le jeu (une vie de 240, une vitesse
de 3) passe sans encombre, et c'est pourquoi le rapport imprime chaque champ :
la vérification au niveau de l'octet contre le fichier est la vôtre à faire, et
c'est la seule vérification qui attrape le mensonge bien typé. Quand une table
grandit d'une colonne que vous n'attendiez pas à régler — la vie du boss, la
vitesse d'un projectile — le fichier est le premier endroit où regarder, et le
rapport de l'exécution est votre façon de regarder.

Rien ici ne touche le chargeur, l'analyse ou le format : la sonde est trois
chargements et trois rapports à côté de ceux de la leçon, et les deux fichiers
qu'elle lit sont à vous de garder ou de supprimer.

*Page traduite de la version anglaise `book/solutions/lesson-071/ex1.md`,
révision `84af294`.*

<!-- translation-source: book/solutions/lesson-071/ex1.md @ 84af294 -->
