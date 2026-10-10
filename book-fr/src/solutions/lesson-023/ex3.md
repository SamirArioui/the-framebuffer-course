# Solution : exercice 3 — Pause

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 3 — Pause](../../lessons/part-0/lesson-023-state-machine.md) de la leçon 023.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-023/ex3.patch}}
```

## Visite guidée

La pause est un état, pas un drapeau — ce qui est exactement le sujet de cette
leçon. Le patch ajoute `PAUSE` à `enum GameState` et son nom à `state_names`,
deux lignes `p` dans la gestion des touches (une pour chaque sens de la
bascule, chacune gardée par l'état qu'elle quitte), et une ligne de dessin.
`Update` n'a eu besoin d'aucun changement : la boucle de ticks n'avance déjà
le serpent que `if (state == PLAY)`, aussi `PAUSE` gèle-t-il la simulation
gratuitement — le gain d'acheminer tout le mouvement par une seule
vérification d'état.

Une exécution le prouve : `( printf ' p'; sleep 1.5; printf 'p' ) | ./snek 75`
montre `state=pause … at=10,20` de la frame 1 à environ la frame 45 — la tête
ne bouge jamais — puis `state=play` et la position avance à nouveau après le
second `p`. Notez ce que la trace montre aussi : `tick` continue de compter
pendant la pause. Le pas fixe mesure le temps réel et le temps réel ne
s'arrête pas ; ce que la pause gèle est la *réponse du jeu* à ce temps. Geler
aussi l'accumulateur rendrait la pause exacte à l'horloge murale au prix d'un
`Update` plus subtil — les deux conceptions se défendent, et savoir quelle
horloge s'arrête est la partie qui compte.

*Page traduite de la version anglaise `book/solutions/lesson-023/ex3.md`,
révision `94b8aa1`.*

<!-- translation-source: book/solutions/lesson-023/ex3.md @ 94b8aa1 -->
