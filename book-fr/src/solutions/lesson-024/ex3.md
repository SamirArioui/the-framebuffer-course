# Solution : exercice 3 — Deux lignes, une touche

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 3 — Deux lignes, une touche](../../lessons/part-0/lesson-024-command-table.md) de la leçon 024.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-024/ex3.patch}}
```

## Visite guidée

La prédiction : le message ne s'imprime jamais. `RunCommand` balaie la table
depuis le haut et `return` après la première correspondance de touche, aussi
la ligne originale `{'q', CmdQuit}` — près du haut — avale-t-elle chaque `q`,
et la nouvelle ligne `CmdQuitSecond` en bas est des données mortes.
L'exécution le confirme : `printf 'q' | ./snek 30` finit
`done after 1 frames` sans aucun `second q row ran` nulle part dans la sortie.
Supprimez la *première* ligne `q` et le message apparaît instantanément —
preuve que c'est le balayage, pas la touche, qui a décidé du résultat.

Première-correspondance-gagne est le contrat, et c'est le même contrat que
`switch` — sauf qu'ici les « cas » sont des données que vous pouvez
réordonner, étendre, et même charger d'ailleurs à l'exécution. Le contrat a
des dents : une ligne masquée est silencieusement inatteignable, et aucun
compilateur ne vous avertit, parce qu'un `int` dupliqué dans un tableau est du
C parfaitement légal. C'est l'échange de la répartition pilotée par les
données — vous échangez des branches vérifiées par le compilateur contre une
table dont les règles sont à vous de tenir : une ligne par touche,
première-correspondance-gagne, et le `return` du balayage est ce qui le rend
ainsi. Les lignes WASD de l'exercice 1 sont à l'abri du masquage précisément
parce que chaque ligne y lie une touche distincte.

*Page traduite de la version anglaise `book/solutions/lesson-024/ex3.md`,
révision `bd716a9`.*

<!-- translation-source: book/solutions/lesson-024/ex3.md @ bd716a9 -->
