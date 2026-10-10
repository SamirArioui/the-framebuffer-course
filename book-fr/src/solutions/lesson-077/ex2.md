# Solution : exercice 2 — Quel mur a dit non

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — Quel mur a dit non](../../lessons/part-4/lesson-077-mover.md) de la leçon 077.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-077/ex2.patch}}
```

## Visite guidée

Le diff change la réponse du mover, pas son comportement : `MoveEntity` renvoie
un `MoveResult` — `landed_x`, `landed_y` — rempli à mesure que chaque axe est
essayé, et le rapport d'état de l'exécution imprime les deux composants aux
transitions au lieu d'un unique drapeau `blocked`. La marche garde le résultat
du héros (elle sait quel emplacement est le sien) et le rapport en est le seul
lecteur.

Les exécutions, depuis l'état final de cette leçon plus le patch. En poussant
Gauche contre le mur de gauche de la carte, puis Bas contre la ligne du bas :

```
engine: hero: x refused, y landed (t=2.005)
engine: hero: x landed, y landed (t=2.030)
...
engine: hero: x refused, y landed (t=5.303)
```

et en poussant Bas contre la ligne du bas tout seul (l'exécution précédente) :

```
engine: hero: x landed, y refused (t=2.003)
engine: hero: x landed, y landed (t=2.029)
```

Les deux refus se lisent différemment — `x refused` au mur de gauche, `y refused`
au sol — et c'est toute la réponse à la question de l'exercice. La
comparaison de positions qu'utilisait l'exécution avant (`hero.x == was_x &&
hero.y == was_y`) ne connaît que le fait que le héros n'a pas bougé : elle ne
peut pas distinguer un mur à gauche d'un sol en dessous, elle ne peut pas
distinguer un refus d'une frame sans aucune requête (c'est pourquoi l'ancien
rapport portait une garde `move_x != 0 || move_y != 0`), et elle ne peut pas
voir le cas où un axe a abouti et l'autre non — le *glissement* — sans
davantage de comparaisons. Chacune de ces distinctions est exactement ce que le
mover savait au moment où il les a faites.

C'est la valeur de la réponse typée en une ligne : **le producteur d'une
décision la rapporte, au lieu que le consommateur la redérive**. Le résultat du
mover est le fait ; la position est une preuve. Quand la leçon 081 attribuera
le coût de l'update, ou que le héros de la partie 5 n'animera que tant que
`landed_x` est faux contre un mur, ils liront le même champ — aucune seconde
implémentation de « est-ce que ça a touché quelque chose » n'apparaît nulle
part.

Deux détails à garder. Le résultat est une simple struct de deux bool — les
valeurs typées de ce moteur sont des champs nommés, pas des exceptions ni des
codes d'erreur, et elle compose avec la forme d'`EntityResult` (une valeur qui
dit ce qui est arrivé). Et le *comportement* du mover n'a pas changé du tout :
même forme à un axe, mêmes requêtes, même glissement — l'exercice a élargi la
réponse, jamais la règle.

Une note honnête sur les exécutions ci-dessus : les appuis scriptés réveillent
deux frames chacun, donc le rapport oscille entre `refused` et `landed` à
mesure que les appuis alternent avec leurs relâchements. Sur une touche
maintenue, les transitions sont celles que produit la main d'un joueur :
`x landed, y landed` jusqu'au mur, puis `y refused` tant qu'il pousse.

*Page traduite de la version anglaise `book/solutions/lesson-077/ex2.md`,
révision `c8d0b9c`.*

<!-- translation-source: book/solutions/lesson-077/ex2.md @ c8d0b9c -->
