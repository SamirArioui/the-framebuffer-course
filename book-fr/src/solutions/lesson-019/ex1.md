# Solution : exercice 1 — La vie de `strtoul`

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — La vie de `strtoul`](../../lessons/part-0/lesson-019-game-loop.md) de la leçon 019.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-019/ex1.patch}}
```

## Visite guidée

Avec la ligne d'instrumentation en place, les cinq exécutions rapportent :

```
./snek 3    → parsed=3 end=''           then three frame lines
./snek 0    → parsed=0 end=''           then the error message
./snek abc  → parsed=0 end='abc'        then the error message
./snek 12x  → parsed=12 end='x'         then the error message
./snek      → usage line (the parser is never reached)
```

`strtoul` convertit la plus longue suite de chiffres de tête et met `end` sur
le premier caractère qu'il n'a pas consommé. Pour `3` et `0` toute la chaîne
est consommée, aussi `end` pointe-t-il sur le terminateur et `*end` vaut-il
`'\0'` — le test passe et la différence de résultat tient entièrement au
contrôle séparé `max_frames == 0`. Pour `abc` aucun chiffre n'est consommé du
tout : `0` revient et `end` pointe toujours sur la chaîne entière. Pour `12x`
la conversion réussit avec `12`, et `end` pointant sur le `x` est la seule
preuve de la saleté — exactement ce que `*end != '\0'` attrape et qu'`atoi`
aurait manqué. L'exécution sans argument n'atteint jamais le parseur : la
garde d'`argc` dans `main` sort d'abord. Prédire celle-là correctement tient
surtout à se souvenir de la position de la garde dans `main`.

*Page traduite de la version anglaise `book/solutions/lesson-019/ex1.md`,
révision `6f5430b`.*

<!-- translation-source: book/solutions/lesson-019/ex1.md @ 6f5430b -->
