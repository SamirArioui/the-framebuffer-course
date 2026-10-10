# Solution : exercice 1 — La courbe, prédite

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — La courbe, prédite](../../lessons/part-5/lesson-085-hero-movement.md) de la leçon 085.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-085/ex1.patch}}
```

## Visite guidée

La sonde imprime `hero.move_x` une fois par frame — la fraction lissée de la
pleine vitesse — si bien que la courbe est une colonne de nombres.

**La prédiction.** Depuis le repos (`move = 0`), maintenir une direction donne
`move += (1 − move) × dt/HERO_TIME` à chaque frame. À 60 fps,
`dt = 1/60 ≈ 0.016667`, et avec `HERO_TIME = 0.12` l'easing par frame vaut
`k = dt/HERO_TIME ≈ 0.13889`. Donc :

| frame | `HERO_TIME = 0.12` | `HERO_TIME = 0.24` |
| ----- | ------------------ | ------------------ |
| 1 | `0 + 1×0.1389 = 0.139` | `0 + 1×0.0694 = 0.069` |
| 2 | `0.139 + 0.861×0.1389 = 0.258` | `0.069 + 0.931×0.0694 = 0.134` |
| 3 | `0.258 + 0.742×0.1389 = 0.361` | `0.134 + 0.866×0.0694 = 0.194` |

**Atteint-elle la pleine vitesse en trois frames ?** Non — loin de là. Après
trois frames, le héros est à 14 %, 26 %, 36 % de la pleine vitesse (et 7 %,
13 %, 19 % avec la constante doublée). L'easing est une approche géométrique —
`move` comble une *fraction* fixe de l'écart restant à chaque frame — donc il
s'approche asymptotiquement de la pleine vitesse sans jamais tout à fait y
arriver en un nombre fixe de frames. En pratique, il se lit comme « à pleine
vitesse » vers 5-7 frames (`HERO_TIME = 0.12`) ou 10-14 (`HERO_TIME = 0.24`).

**Laquelle atteint la pleine vitesse plus tôt, et de combien ?** `HERO_TIME =
0.12` — environ deux fois plus vite que `0.24`, frame pour frame, parce que
l'easing par frame `k = dt/HERO_TIME` est deux fois plus grand. La constante
doublée divise par deux la vitesse de toute la courbe : après n'importe quelle
frame `n`, `0.24` est à peu près à mi-chemin de ce que `0.12` avait atteint.

Sur une machine qui tourne à 60 fps, la colonne de la sonde reproduit ces six
nombres à trois décimales près. (Sur la boucle sans écran de la machine de
l'auteur — environ une frame par seconde — `dt` est énorme, `k` est borné à 1,
et la sonde imprime `0.000` puis `1.000` : la même courbe, échantillonnée en un
seul pas. C'est pourquoi l'exercice dit de la vérifier à 60 fps.) La forme est
le propos : `HERO_TIME` est le nombre de secondes que la courbe met à compter,
et c'est le bouton dans lequel vit le « poids » du héros.

*Page traduite de la version anglaise `book/solutions/lesson-085/ex1.md`,
révision `82879a4`.*

<!-- translation-source: book/solutions/lesson-085/ex1.md @ 82879a4 -->
