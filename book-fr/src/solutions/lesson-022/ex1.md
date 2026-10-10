# Solution : exercice 1 — Compter le vidage

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — Compter le vidage](../../lessons/part-0/lesson-022-double-buffer.md) de la leçon 022.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-022/ex1.patch}}
```

## Visite guidée

La prédiction : le premier vidage réécrit le monde — 20 lignes × 40 colonnes =
800 cellules. Chaque vidage après n'écrit que ce qui a *changé*, et avec le
marqueur comme seule chose qui bouge, la réponse est soit 2 (son ancienne
cellule vidée, sa nouvelle remplie) soit 0 (une frame tombée entre deux ticks
et n'ayant rien changé du tout). L'exécution instrumentée de `./snek 10` le
confirme exactement : `flush cells=800`, puis un motif de `0, 0, 2` à mesure
que les ticks tombent toutes les trois frames.

Les comptes sont l'invariant du double tampon rendu visible : `front` est une
affirmation sur le terminal, `back` est la scène, et le vidage ne paie que la
différence. Notez l'échappatoire `front_valid` — avant le premier dessin il
n'y a *pas* d'affirmation, aussi chaque cellule compte-t-elle comme changée
(et l'écran est nettoyé une fois). Le compteur est un instrument jetable ; ce
qui reste est l'habitude de se demander « combien de travail cela a-t-il
réellement fait ? » — qui est aussi la question derrière les arguments de
scintillement et de bande passante pour le double tampon en premier lieu.

*Page traduite de la version anglaise `book/solutions/lesson-022/ex1.md`,
révision `eea0461`.*

<!-- translation-source: book/solutions/lesson-022/ex1.md @ eea0461 -->
