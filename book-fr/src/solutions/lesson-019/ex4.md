# Solution : exercice 4 — Pourquoi trois phases

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 4 — Pourquoi trois phases](../../lessons/part-0/lesson-019-game-loop.md) de la leçon 019.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-019/ex4.patch}}
```

## Visite guidée

L'exécution instrumentée de `./snek 2` imprime, dans l'ordre : `ProcessInput`,
`Update`, `Render`, `frame=1`, puis les trois mêmes phases à nouveau et
`frame=2`. Chaque itération parcourt le cycle exactement une fois, l'entrée
d'abord et le dessin en dernier.

Cet ordre est l'argument. `ProcessInput` tourne d'abord pour que le pas de
mise à jour voie chaque touche que le joueur a pressée *cette* frame — lire
puis simuler, jamais simuler puis lire. `Update` possède l'état : s'il
dessinait quoi que ce soit, la simulation dépendrait de la fréquence du
rendu, et le jeu s'accélérerait sur un écran rapide. `Render` observe et met
en forme — s'il avançait l'état, la trace le trahirait : une boucle faisant N
itérations rapporterait plus de N frames, et l'expérience du double `Render`
montre la forme de ce bug immédiatement, avec deux lignes `frame=` par
itération tandis qu'`Update` tourne toujours une fois. Entrée, simulation,
présentation : un propriétaire par responsabilité, et un compteur de frames
qui compte les itérations, pas le travail.

La duplication des appels `fprintf` dans le patch est de l'instrumentation
jetable — supprimez-la une fois que la trace a fait son point.

*Page traduite de la version anglaise `book/solutions/lesson-019/ex4.md`,
révision `6f5430b`.*

<!-- translation-source: book/solutions/lesson-019/ex4.md @ 6f5430b -->
