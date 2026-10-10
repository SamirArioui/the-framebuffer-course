# Solution : exercice 2 — Le redémarrage qui n'en était pas un

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — Le redémarrage qui n'en était pas un](../../lessons/part-0/lesson-023-state-machine.md) de la leçon 023.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-023/ex2.patch}}
```

## Visite guidée

Reproduisez avec un producteur qui presse espace, dirige vers le haut, et
presse à nouveau espace seulement après la mort — la seconde touche doit
arriver *pendant que le jeu est mort*, aussi a-t-elle besoin de vrai temps
entre les octets :

```
( printf ' \033[A'; sleep 3.5; printf ' ' ) | ./snek 150
```

Avec le bug, la trace montre la première mort au tick 9
(`dir=up at=2,20`), un redémarrage à la frame 106 — et le serpent ressuscité
grimpe immédiatement la même colonne et meurt au mur d'en haut à nouveau au
tick 43. `StartGame` remettait à zéro le corps, le score et la nourriture,
mais `dir` est un état ordinaire comme un autre et il portait encore la
direction de la *dernière vie*. La promesse de l'écran de mort, « press space
to play again », n'est vraie que si chaque champ est remis à zéro.

La correction ajoute `dir = DIR_RIGHT;` à côté des autres remises à zéro.
Relancez et le redémarrage à la frame 106 rapporte
`state=play … dir=right at=10,21`, et la frame 150 trouve le serpent toujours
vivant à `at=10,36`. La règle générale mérite une phrase dans le `StartGame`
de tout jeu : listez ce que le jeu *est* — position, direction, score,
minuteurs, mode — et mettez tout à jour, à chaque fois. Les machines à états
rendent les états explicites ; elles ne rendent pas l'état périmé impossible.

*Page traduite de la version anglaise `book/solutions/lesson-023/ex2.md`,
révision `94b8aa1`.*

<!-- translation-source: book/solutions/lesson-023/ex2.md @ 94b8aa1 -->
