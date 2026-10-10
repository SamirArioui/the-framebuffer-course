# Solution : exercice 1 — Deux héros, une ligne

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — Deux héros, une ligne](../../lessons/part-4/lesson-073-rows.md) de la leçon 073.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-073/ex1.patch}}
```

## Visite guidée

Le diff, c'est une seconde création et un acte de vandalisme : `twin` à partir
de la même définition `hero`, puis la vie de la première entité mise à 0 et son
`x` mis à 0, et un rapport nommant les trois sources d'un coup — les champs du
jumeau, les champs du héros, et les champs de la ligne.

La prédiction, avant toute exécution. Le jumeau a été créé *avant* le
vandalisme, et `EntityFromDef` copie chaque attribut — donc le jumeau garde ce
que la ligne énonçait à sa création : `x 312`, `health 3`. Le héros est celui
qui a été écrit : `x 0`, `health 0`. Et la définition — la ligne de la table —
dit toujours `x 312`, `health 3`, parce que rien dans cette leçon n'écrit *dans*
une définition. Trois réponses, deux d'entre elles identiques, et une seule a
bougé.

L'exécution, de l'état final de cette leçon plus le patch :

```
engine: entity hero: x 312 y 232 facing 0 speed 240 health 3 sprite 16x16
engine: twin hero: x 312 y 232 health 3 (hero at x 0 health 0; the row says x 312 health 3)
engine: table: "dragon" -> unknown
```

Exactement comme prédit. La ligne du jumeau est toute la réponse en une seule
chaîne : deux entités issues d'une ligne, l'une d'elles changée, et la ligne
intacte.

Ce qui fait de cela le design et non un accident mérite d'être redit une fois,
car c'est la raison pour laquelle `EntityFromDef` copie au lieu de pointer. Le
jeu *écrit* des champs d'entité — la position du héros à chaque frame où son
joueur maintient une touche, sa vie chaque fois que quelque chose l'atteint —
et ces écritures doivent atterrir sur la vie d'une entité, pas sur la définition
depuis laquelle toute entité de ce type est créée. Une vue sur la ligne ferait
que déplacer un héros déplacerait les jumeaux, et réécrirait en silence les
valeurs énoncées par le fichier pendant que l'exécution se déroule ; la table
cesserait d'être de la donnée pour devenir une variable partagée. Avec des
copies, la table est le passé — ce que le fichier a dit — et les lignes du
magasin sont le présent. La leçon 074 rend « le magasin » réel ; les deux
locales de cette leçon en sont tout le contenu.

Un champ n'est *pas* copié par valeur et c'est délibéré aussi : le pointeur
`sprite`. Chaque héros partage l'art, parce que l'art est chargé une fois et en
lecture seule — la définition le porte (l'exécution l'a chargé au démarrage) et
l'entité pointe dessus. Des copies là où le jeu écrit, du partage là où il
n'écrit pas.

Rien ici ne touche le chargeur, l'analyse de la table ou la boucle du jeu : la
sonde est une création de plus et un rapport à côté de ceux de la leçon.

*Page traduite de la version anglaise `book/solutions/lesson-073/ex1.md`,
révision `df91188`.*

<!-- translation-source: book/solutions/lesson-073/ex1.md @ df91188 -->
