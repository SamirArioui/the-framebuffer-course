# Solution : exercice 1 — Le tir en éventail

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — Le tir en éventail](../../lessons/part-5/lesson-087-projectiles-weapons.md) de la leçon 087.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-087/ex1.patch}}
```

## Visite guidée

Une arme est une ligne, donc une arme qui se comporte différemment est une ligne
qui le dit. L'arme à dispersion est une ligne de plus (`scatter 1 120 bolt 3`)
et une colonne nommée de plus dans le format : **`burst`** — combien de tirs une
pression de gâchette déclenche. La croissance est le geste propre de la leçon :
la colonne est nommée, additive et avec défaut (`burst` vaut `1` par défaut),
donc chaque fichier livré continue de charger à l'octet près — leurs lignes
portent simplement un tir par pression — et le rapport d'armement montre la
valeur de la ligne là où elle est portée :

```
engine: arm: hero arms scatter (damage 1, rate 120, burst 3, fires bolt)
engine: fire: hero -> bolt (damage 1, range 160)
engine: fire: hero -> bolt (damage 1, range 160)
engine: fire: hero -> bolt (damage 1, range 160)
engine: shot bolt retired — wall
engine: shot bolt retired — wall
engine: shot bolt retired — range
```

Une pression de gâchette, trois lignes `fire` — puis trois sorts différents :
deux des voies de la dispersion ont rencontré des murs, la troisième a volé
toute sa portée. La dispersion est réelle : les tirs partent selon trois
directions, pas une.

Les directions sont la partie intéressante. Le tir du milieu vole la visée ; de
chaque côté, c'est la visée **tournée d'un pas de 45 degrés**, et une rotation
d'une direction unitaire de 45 degrés — `((x − y)/√2, (x + y)/√2)` — conserve
exactement sa longueur. Donc chaque tir de la dispersion vole à la vitesse propre
du projectile (aucun rattrapage d'échelle), et tourner une direction de façon
répétée la garde unitaire. Aucune racine carrée n'est calculée à l'exécution :
`AIM_DIAG` est le même 1/√2 que l'intention diagonale du héros utilise depuis la
leçon 085.

La troisième touche numérique arme la troisième ligne — le bloc d'armement de
`HeroFire` grandit d'une ligne, exactement la forme que la leçon a promise : une
arme est une ligne, et les armes grandissent en lignes.

*Page traduite de la version anglaise `book/solutions/lesson-087/ex1.md`,
révision `3fd99b7`.*

<!-- translation-source: book/solutions/lesson-087/ex1.md @ 3fd99b7 -->
