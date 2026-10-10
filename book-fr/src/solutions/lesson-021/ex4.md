# Solution : exercice 4 — WASD

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 4 — WASD](../../lessons/part-0/lesson-021-terminal-input.md) de la leçon 021.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-021/ex4.patch}}
```

## Visite guidée

Quatre lignes `else if` dans la branche des touches ordinaires — `w`, `a`,
`s`, `d` — assignant chacune la même énumération de direction que les
flèches. Rien d'autre ne change, ce qui est la partie plaisante : le parseur
de séquences d'échappement n'est pas touché parce que les touches de mots sont
des octets uniques qui n'y entrent jamais. `printf 'wasdq' | ./snek 30` tourne
une frame et la trace finit `dir=right` — les cinq octets ont été analysés en
une seule lecture, la dernière des quatre touches laissant sa marque avant que
`q` ne quitte. Mélanger les deux orthographes fonctionne aussi :
`printf 'w\033[Csq' | ./snek 30` va en haut, à droite, en bas, quitter.

Cela vaut la peine de remarquer combien de code cela a pris comparé au parseur
d'échappement. Ajouter une touche ordinaire est une comparaison contre un
octet ; ajouter une touche *nommée* — une touche de fonction, disons, qui
envoie une séquence plus longue — veut dire étendre la machine à états. Cette
asymétrie entre « les touches sont des octets » et « les touches sont des
séquences » est exactement ce que la table de commandes de la leçon 024
absorbera : là-bas les flèches arrivent comme des codes symboliques comme
n'importe quelle autre touche, et chaque liaison — touche de mot ou flèche —
devient une ligne de table.

*Page traduite de la version anglaise `book/solutions/lesson-021/ex4.md`,
révision `2369864`.*

<!-- translation-source: book/solutions/lesson-021/ex4.md @ 2369864 -->
