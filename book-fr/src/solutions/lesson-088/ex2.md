# Solution : exercice 2 — Pourquoi le boss ne mérite pas de code

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — Pourquoi le boss ne mérite pas de code](../../lessons/part-5/lesson-088-enemy-tables.md) de la leçon 088.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-088/ex2.patch}}
```

## Visite guidée

La sonde rend la forme visible : une visite par entité vivante, une seule
forme de travail, chaque type à travers elle.

```
walk visit: hero (behavior none)
walk visit: slime (behavior none)
walk visit: bat (behavior chase)
walk visit: wisp (behavior flee)
walk visit: spitter (behavior keep)
walk visit: golem (behavior boss)
```

Six types, une boucle. Maintenant, l'argument.

**Ce que cinq fonctions d'update coûteraient à la marche.** La règle de la
leçon 075 est que le travail par entité a *une* forme : chaque entité vivante
exactement une fois, dans l'ordre des emplacements, aucun emplacement retiré
visité. Avec `UpdateBat`, `UpdateWisp`, `UpdateGolem`, … la marche reste une
boucle — mais son corps est un dispatch qui grandit à chaque type, et
l'invariant « chaque entité reçoit exactement le bon travail, une fois » n'est
plus garanti par la forme ; il se défend à nouveau, fonction par fonction.
Ajoutez un type et le dispatch grandit. Ajoutez un effet qui doit tourner pour
*certains* types et vous obtenez un second dispatch. La seule branche sur
`behavior` — un fait que la ligne porte — achète l'inverse : le travail est
exprimé une fois, et un type est une valeur.

**Ce que les vagues de la leçon 091 changent.** Les vagues font apparaître les
mêmes lignes encore et encore — des douzaines d'entités sur une partie, mais
jamais un nouveau *type* à l'exécution. Avec des lignes, l'apparition est
`EntityCreate(store, row)` et la marche ne s'en aperçoit pas. Avec du code par
type, la population de chaque vague est un ensemble de sites d'appel à garder
en phase — et la vague qui fait apparaître « trois types et le boss ensemble »
est celle qui a le plus besoin que le travail s'échelle avec le *nombre*, pas
avec le code.

**Ce que « les attributs par type sont de la donnée » protège.** Une constante
de vitesse dans le code est une valeur introuvable, incomparable et non
ajustable sans recompilation — et deux types ne peuvent pas en différer sans
*plus* de code. La valeur d'une ligne est diffable (les trois types de
l'exercice 1 étaient trois lignes de texte), relisible en revue, et l'exécution
l'imprime deux fois — une fois comme la définition, une fois comme l'entité
porteuse — si bien que personne n'a à croire le code sur parole pour savoir ce
qu'est un golem.

**L'argument le plus solide — la seule chose qu'une ligne ne peut pas
porter.** Le boss a réellement besoin de *temps* : des phases, une cadence,
« poursuis maintenant, crache trois fois, fuis, recommence ». Une ligne porte
des faits ; elle ne peut pas porter un planning. C'est exactement ce que la
leçon 090 lui donne — **son propre planning**, de l'état et du timing par
entité — et remarquablement *pas* sa propre machinerie de mouvement : le
planning choisit parmi les trois mêmes comportements que tout le monde
utilise. Le boss obtient ce dont il a besoin sans le code que la règle
d'itération interdit. (La sonde ci-dessus est une mesure jetable ; la vraie
marche n'imprime rien.)

*Page traduite de la version anglaise `book/solutions/lesson-088/ex2.md`,
révision `ac121ed`.*

<!-- translation-source: book/solutions/lesson-088/ex2.md @ ac121ed -->
