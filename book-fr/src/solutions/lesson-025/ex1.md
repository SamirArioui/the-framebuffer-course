# Solution : exercice 1 — Une troisième vue

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — Une troisième vue](../../lessons/part-0/lesson-025-cpp-subset.md) de la leçon 025.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-025/ex1.patch}}
```

## Visite guidée

Une nouvelle dessinable, c'est une classe et une ligne. `TallyView` déclare la
même fonction unique que les autres vues, et son `Draw` formate les compteurs
avec `snprintf` et les peint sur la ligne de bord inférieure à travers la même
surcharge de `Grid::Put` qu'utilise `GridView`. L'enregistrer a coûté un objet
et un `&tally` dans `views` — `Render` n'a jamais changé, parce que la boucle
n'a jamais su ce qu'elle dessinait. C'est tout le but de l'interface.

L'exécution tracée finit `done after 30 frames, 9 ticks`, et la ligne du bas
de l'écran sort comme :

```
19|+-f=30 t=9-----------------------------+
```

L'ordre de dessin est l'ordre de la table, et ici il a mordu : le tally se
trouve *après* `&playfield` dans `views`, aussi peint-il sur les tirets de
bord que `GridView::Draw` vient de placer — le même ordre de superposition que
le `Render` de la leçon 024 maintenait à la main. Une ligne de tableau plus
haut ou plus bas et les compteurs seraient redevenus des tirets.

*Page traduite de la version anglaise `book/solutions/lesson-025/ex1.md`,
révision `975e844`.*

<!-- translation-source: book/solutions/lesson-025/ex1.md @ 975e844 -->
