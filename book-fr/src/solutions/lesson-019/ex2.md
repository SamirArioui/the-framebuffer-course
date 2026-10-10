# Solution : exercice 2 — La chaîne qui ne finit jamais

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — La chaîne qui ne finit jamais](../../lessons/part-0/lesson-019-game-loop.md) de la leçon 019.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-019/ex2.patch}}
```

## Visite guidée

Les deux chaînes échouent pour deux raisons différentes, et aucune n'est
visible pour `*end != '\0'`. `strtoul` est documenté comme acceptant un signe
optionnel, aussi `-5` est-il *analysé*, pas rejeté : il revient comme
`18446744073709551611` — le retournement non signé de −5 — avec `end` sur le
terminateur et `errno` intact. La chaîne en débordement se convertit en
`18446744073709551615` (`ULONG_MAX`) et est le seul cas qui met `errno` à
`ERANGE`. Les deux chaînes sont entièrement consommées, aussi le test du
pointeur de fin passe-t-il et `Update` compte-t-il vers un nombre que la
machine n'atteindra jamais — c'est le « blocage ».

La correction ajoute deux vérifications : un `-` de tête est refusé d'emblée,
et `errno == ERANGE` attrape le débordement. Les deux sont nécessaires —
l'exécution `-5` laisse `errno` à zéro — et `errno = 0` avant l'appel est ce
qui rend la seconde vérification saine, puisque `errno` est collant et peut
porter une vieille erreur. Après la correction, les deux exécutions impriment
le message habituel `FRAMES must be a positive integer` et sortent 1, tandis
que `./snek 3` se comporte exactement comme avant.

*Page traduite de la version anglaise `book/solutions/lesson-019/ex2.md`,
révision `6f5430b`.*

<!-- translation-source: book/solutions/lesson-019/ex2.md @ 6f5430b -->
