# Solution : exercice 3 — Flottant contre entier

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 3 — Flottant contre entier](../../lessons/part-0/lesson-016-lines.md) de la leçon 016.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-016/ex3.patch}}
```

## Visite guidée

La référence trace une diagonale de 512 pixels 20 000 fois par rastériseur
dans un tampon brouillon de 512×512 et chronomètre chaque boucle avec
`clock()`. Sur cette machine, à `-O0` :

```
bresenham: 2.03 us/line
float:     1.60 us/line
pixels that disagree: 6
```

Les exécutions répétées atterrissent dans les mêmes bandes (Bresenham ≈ 2,0
µs, flottant ≈ 1,6 µs) — le folklore selon lequel la ligne flottante est plus
lente ne survit pas au contact d'un CPU moderne : les deux boucles sont des
stockages bornés par la mémoire plus une arithmétique triviale. Mais la
seconde mesure est celle qui tranche : pour la diagonale `(0, 0)`–`(7, 5)` les
deux rastériseurs ne sont pas d'accord sur six des pixels qu'ils touchent — la
ligne flottante tronque `(1, 0)`, `(4, 2)`, `(5, 3)` là où Bresenham, exact,
allume `(1, 1)`, `(4, 3)`, `(5, 4)`. Des pixels différents à reculons à
nouveau, et une ligne verticale divise par zéro. Le chronométrage dit « l'un
ou l'autre » ; la géométrie dit Bresenham — le produit d'un rastériseur de
lignes *est* ses pixels, et seul l'entier calcule les bons. Gardez cet
exercice dans votre poche comme leçon de mesure : le folklore sur la vitesse
est une hypothèse, et `clock()` ne coûte rien.

*Page traduite de la version anglaise `book/solutions/lesson-016/ex3.md`,
révision `df71716`.*

<!-- translation-source: book/solutions/lesson-016/ex3.md @ df71716 -->
