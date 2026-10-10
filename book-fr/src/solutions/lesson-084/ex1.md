# Solution : exercice 1 — Le coin et le mur

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — Le coin et le mur](../../lessons/part-5/lesson-084-collision.md) de la leçon 084.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-084/ex1.patch}}
```

## Visite guidée

La sonde note où chaque entité se trouvait, la déplace à travers `MoveEntity`,
puis rapporte **quel axe a réellement bougé** — `x moved/stopped, y moved/stopped`.
Cette seule ligne est toute la réponse rendue visible : la règle d'un axe à la
fois se montre comme un axe qui bouge et l'autre qui s'arrête.

**Les deux prédictions.** `MoveEntity` prend le pas en x d'abord (vérifié contre
le y *d'origine* de l'entité), puis le pas en y (vérifié contre le x *mis à
jour*). Donc :

- **(a) En diagonale contre un mur plat qui ne bloque que x** (disons, en haut à
  gauche contre un mur à gauche) : x est refusé (sa destination est solide),
  mais y est toujours libre — donc **il glisse** le long du mur, sur l'axe
  **y**. La sonde affiche `x stopped, y moved`.
- **(b) En diagonale dans un coin où les deux axes sont solides** : x est refusé
  et y est refusé — donc **il s'arrête complètement**. La sonde affiche
  `x stopped, y stopped`. Aucun axe ne l'emporte ; il n'y a nulle part où aller.

L'ordre compte dans le cas intermédiaire : parce que x est résolu contre le y
*d'origine* et y contre le x *mis à jour*, x a le premier refus. Si les deux pas
étaient individuellement légaux mais pas ensemble, x bouge quand même en premier
et y est ensuite jugé contre le nouveau x. Donc dans un passage diagonal serré,
**c'est l'axe x qui l'emporte** — il est résolu en premier.

L'exécution derrière la leçon montre déjà les deux formes dans la trace de
position du héros lui-même — le glissement d'abord :

```
engine: hero at 312,232
engine: hero at 210,232 (t=3.447)     <- x moved 312->210, y frozen: slid along a wall
engine: hero blocked at 206,228 (t=6.477)   <- both axes refused: stopped in the corner
```

`312 → 210` à un `y = 232` constant, c'est le glissement : un axe refusé, l'autre
libre. `blocked at 206,228`, c'est le coin : les deux refusés. Sonde appliquée,
la même exécution l'épelle pas à pas — une série de `x stopped, y moved` (ou
`x moved, y stopped`, selon le mur) pendant que le héros glisse le long du mur,
puis `x stopped, y stopped` à l'instant où il est coincé dans le coin.

La leçon à emporter : il n'y a pas de comportement « glissement » séparé ni de
comportement « arrêt » séparé. Il y a une règle — résoudre chaque axe pour
lui-même — et la différence entre un mur et un coin n'est que le nombre d'axes,
sur les deux, qu'il refuse. C'est aussi pourquoi l'ordre n'est pas arbitraire :
échanger les deux pas ne changerait pas le glissement, mais changerait quel axe
l'emporte quand un seul des deux peut être honoré.

*Page traduite de la version anglaise `book/solutions/lesson-084/ex1.md`,
révision `84d7ea6`.*

<!-- translation-source: book/solutions/lesson-084/ex1.md @ 84d7ea6 -->
