# Solution : exercice 2 — Pas de tunneling

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — Pas de tunneling](../../lessons/part-5/lesson-084-collision.md) de la leçon 084.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-084/ex2.patch}}
```

## Visite guidée

Le bug est réel et il tient à l'*échantillonnage*. `MoveEntity` ne vérifie que
la destination du pas entier : `next_x = entity.x + dx`, puis « le rectangle à
`next_x` est-il solide ? ». Sur une frame courte, `dx` fait quelques pixels et
tout va bien. Sur une frame longue — la machine saccade, le débogueur met en
pause, la fenêtre est traînée — `dx = speed × dt` peut faire des dizaines de
pixels. Si un mur mince (une tuile, comme la bordure `#` de la carte) se trouve
dans l'écart entre `entity.x` et `entity.x + dx`, la destination est du sol libre
de l'*autre* côté, le test passe, et l'entité est téléportée à travers le mur.
Elle n'a jamais « atterri sur » la tuile solide, donc la collision ne s'est
jamais déclenchée. C'est le tunneling.

Le correctif est d'arrêter d'échantillonner les extrémités et de commencer à
échantillonner le chemin. Découpez le déplacement en sous-pas assez petits pour
que l'entité ne puisse pas franchir tout un mur mince en un seul, et appliquez
la même règle d'un axe à la fois dans chacun :

```cpp
int steps = (int)(big / MAX_STEP) + 1;   // big = the larger of |dx|,|dy|
for (int i = 0; i < steps; ++i) { /* one-axis rule on sx, sy */ }
```

`MAX_STEP` (4 pixels ici) doit être plus petit que le mur le plus mince — une
tuile fait 16 pixels, donc 4 laisse de la marge. La règle d'un axe à la fois est
gardée *à l'intérieur* de chaque sous-pas, donc le glissement marche encore
exactement comme avant : dans chaque sous-pas, un axe peut être refusé pendant
que l'autre bouge.

Pour le voir, faites ce que l'exercice dit : augmentez la `speed` du héros dans
`assets/entities.txt` (disons, `240` → `2400` — de la donnée, aucune
recompilation), recompilez, et conduisez le héros droit sur la bordure `#` d'une
tuile. Sans le patch, une frame assez longue envoie `dx` au-delà de la bordure en
un pas et le héros ressort de l'autre côté (ou hors de la carte). Avec le patch,
le héros est refusé à la face de la bordure à chaque fois — les sous-pas le
mènent jusqu'au mur et s'arrêtent, parce que chaque saut de 4 pixels est testé
et que le saut qui atterrirait sur `#` est refusé.

Deux choses à garder en tête. Le découpage en sous-pas corrige le *dépassement*,
pas la règle d'un axe à la fois — c'est une boucle autour de la même résolution,
donc le glissement et le comportement dans les coins sont inchangés. Et il coûte
un peu : un pas qui était un appel à `TileRectSolid` par axe devient `steps`
appels. Sur une frame normale, `steps` vaut 1 et le coût est identique à avant ;
il ne grandit que sur les frames longues, où il fait exactement le travail
nécessaire pour ne pas passer à travers. Si vous voulez le nombre honnête, la
ligne `entities` de l'enregistrement de frame (leçon 081) montrera le coût de la
marche avec le patch en place — mesurez-le plutôt que de le deviner.

*Page traduite de la version anglaise `book/solutions/lesson-084/ex2.md`,
révision `84d7ea6`.*

<!-- translation-source: book/solutions/lesson-084/ex2.md @ 84d7ea6 -->
