# Solution : exercice 1 — Les étincelles héritent du coup

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — Les étincelles héritent du coup](../../lessons/part-5/lesson-093-bursts-easing.md) de la leçon 093.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-093/ex1.patch}}
```

## Visite guidée

La rafale gagne une direction : `FeelBurst` prend le trajet du coup
(`dir_x, dir_y`) à côté de son point et de son nombre. Une rafale radiale — le
`(0, 0)` de la mort — garde la dispersion régulière de la leçon, une voie par
particule. Une rafale directionnelle choisit ses voies par **produit scalaire** :
chaque étincelle prend la voie inutilisée qui fait le plus directement face au
coup, si bien qu'avec quatre étincelles et un tir volant vers l'est, l'éventail
est l'est, les deux diagonales qui l'entourent, puis ce qui lui fait face le
moins mal :

```cpp
double dot = LANE_X[l] * dir_x + LANE_Y[l] * dir_y;
if (dot > best_dot) { best_dot = dot; best = l; }
```

Le coup transmet le mouvement propre du tir (une voie unitaire, ou le `1/√2` de
la diagonale — la même direction que celle utilisée par le vol) ; la mort
transmet des zéros et reste une rafale radiale.

L'exécution — le coup fatal de l'extrait de la leçon, le tir volant vers l'est
du héros jusqu'au bag — pose les douze étincelles ainsi :

```
engine: hit: bolt hits bag — damage 1, health 1 -> 0
engine: burst: spark x4 at 320,232 — 4 made, 0 dropped
engine: burst: spark x8 at 344,240 — 8 made, 0 dropped
engine: spark settled at 384,232 — 64 px out, its row's range 64 (exact)
engine: spark settled at 365,277 — 64 px out, its row's range 64 (exact)
engine: spark settled at 365,186 — 64 px out, its row's range 64 (exact)
engine: spark settled at 320,296 — 64 px out, its row's range 64 (exact)
engine: spark settled at 344,304 — 64 px out, its row's range 64 (exact)
engine: spark settled at 298,285 — 64 px out, its row's range 64 (exact)
engine: spark settled at 280,240 — 64 px out, its row's range 64 (exact)
engine: spark settled at 298,194 — 64 px out, its row's range 64 (exact)
engine: spark settled at 344,176 — 64 px out, its row's range 64 (exact)
engine: spark settled at 389,194 — 64 px out, its row's range 64 (exact)
engine: spark settled at 408,240 — 64 px out, its row's range 64 (exact)
engine: spark settled at 389,285 — 64 px out, its row's range 64 (exact)
```

Les quatre du coup (depuis l'impact à `320,232`) sont `384,232` (plein est),
`365,277` (sud-est), `365,186` (nord-est), `320,296` (sud) — trois des quatre
atterrissent **en avant de l'impact**, du côté du tir, et la quatrième est
l'égalité nord/sud (les deux voies font également face au coup vers l'est à un
produit de `0` ; le balayage prend le sud en premier). Les huit de la mort
(depuis le centre à `344,240`) sont de nouveau toute la boussole : `408,240`
est, `280,240` ouest, les diagonales à `45` px, les cardinales à `64` — ce qui
est tombé se disperse dans toutes les directions, comme une mort le doit.

Les deux rafales se lisent différemment maintenant — un coup *arrose*, une mort
*éclate* — et la différence tient à un nombre : la direction que l'événement
transmet. Les voies, la politique du magasin et la pose sont exactement telles
que la leçon les a livrées ; les quatre effets sont toujours les quatre.

*Page traduite de la version anglaise `book/solutions/lesson-093/ex1.md`,
révision `3c9d3c2`.*

<!-- translation-source: book/solutions/lesson-093/ex1.md @ 3c9d3c2 -->
