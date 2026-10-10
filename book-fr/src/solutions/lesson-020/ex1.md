# Solution : exercice 1 — Prédire les ticks

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — Prédire les ticks](../../lessons/part-0/lesson-020-timing.md) de la leçon 020.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-020/ex1.patch}}
```

## Visite guidée

La prédiction : 30 frames au plafond de 30 fps prennent environ une seconde
d'horloge murale, et une seconde au pas fixe de 100 ms font dix ticks.
L'exécution réelle prend `real 0m1.004s` et finit sur
`done after 30 frames, 9 ticks` — un tick de moins. L'accumulateur
instrumenté explique le tick manquant et l'image frame par frame. Chaque
frame draine son `dt` (environ 0,0335 s) dans `tick_accum`, et dès que
l'accumulateur atteint `TICK_LEN` (0,1 s) il est soustrait et `tick` avance :
la trace montre un tick toutes les trois frames, laissant chaque fois `accum`
près de zéro. La frame 1 rapporte `dt=0.0000`, parce que `prev` a été
échantillonné juste avant la boucle — cela fait environ une frame de temps
jamais donnée à l'accumulateur, et 30 frames de temps réel atterrissent juste
sous dix ticks. Lancez 60 ou 90 frames et la loi des grands nombres lisse
cela : les comptes de ticks atterrissent à un près de `frames * 10 / 30`.

*Page traduite de la version anglaise `book/solutions/lesson-020/ex1.md`,
révision `203c219`.*

<!-- translation-source: book/solutions/lesson-020/ex1.md @ 203c219 -->
