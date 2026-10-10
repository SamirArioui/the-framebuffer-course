# Solution : exercice 3 — Emballez l'état

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 3 — Emballez l'état](../../lessons/part-0/lesson-019-game-loop.md) de la leçon 019.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-019/ex3.patch}}
```

## Visite guidée

Les trois variables de portée fichier deviennent les trois champs de
`struct Game`, les phases gagnent un paramètre `struct Game *g` et atteignent
l'état à travers `g->`, et `main` possède l'instance unique — `&game` aux
sites d'appel est le pointeur `this` de la leçon C++ à venir. Le comportement
est censé être inchangé, et la première exécution après la refactorisation ne
le sera *pas* : `frame` commence comme de la camelote du genre
`133509038754545` et la boucle compte vers nulle part. C'est la seule vraie
leçon cachée dans cet exercice. Les variables de portée fichier sont
initialisées à zéro par défaut ; une locale comme `game` ne l'est pas — ses
champs sont ce que la pile contenait. `struct Game game = {0};` initialise
chaque champ à zéro (`frame` compris), ce qui est l'idiome C à mémoriser. Le
`(void)g;` de `ProcessInput` tient tranquille l'avertissement de paramètre
inutilisé de `-Wextra` jusqu'à ce que la leçon 021 remplisse la fonction.
Avec cela, `./snek 3` imprime les mêmes trois lignes de frame et le même
résumé de sortie qu'avant la refactorisation.

*Page traduite de la version anglaise `book/solutions/lesson-019/ex3.md`,
révision `6f5430b`.*

<!-- translation-source: book/solutions/lesson-019/ex3.md @ 6f5430b -->
