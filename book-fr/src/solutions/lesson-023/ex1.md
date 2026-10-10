# Solution : exercice 1 — Compter jusqu'au mur

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — Compter jusqu'au mur](../../lessons/part-0/lesson-023-state-machine.md) de la leçon 023.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-023/ex1.patch}}
```

## Visite guidée

La prédiction, à partir de la géométrie seule : le serpent naît avec trois
segments à la ligne `GRID_ROWS / 2` — la ligne 10 — avec la tête à la colonne
20, et une flèche du haut le tourne vers le mur d'en haut. Chaque tick grimpe
d'une ligne, et la mort arrive quand la *prochaine* tête atterrirait sur le
bord — l'intérieur s'arrête à la ligne 2, donc les lignes 9, 8, … 2 sont huit
mouvements légaux et le neuvième mouvement atteint la ligne 1 et meurt. La
trace d'état bascule de `state=play` à `state=dead` au **tick 9** — et la
dernière cellule légale, `at=2,20`, est là où le corps reste figé sur l'écran
de mort.

L'exécution instrumentée est d'accord exactement : `head 9,20` jusqu'à
`head 2,20` sont les huit mouvements qui ont lieu ; `head 1,20` est le
mouvement qui n'a pas lieu — le test de mur dans `AdvanceSnake` le rejette et
change d'état à la place. Notez que la barre d'espace dans `printf ' \033[A'`
était essentielle : la flèche du haut ne gouverne que quand `state == PLAY`,
aussi l'exécution doit-elle d'abord quitter l'écran titre. Le tick de mort est
une pure fonction de la géométrie et du pas fixe — le numéro de frame ne l'est
pas, puisque les frames ne portent aucun temps de jeu par elles-mêmes.

*Page traduite de la version anglaise `book/solutions/lesson-023/ex1.md`,
révision `94b8aa1`.*

<!-- translation-source: book/solutions/lesson-023/ex1.md @ 94b8aa1 -->
