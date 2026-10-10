# Solution : exercice 4 — La copie qui ne peut pas exister

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 4 — La copie qui ne peut pas exister](../../lessons/part-0/lesson-025-cpp-subset.md) de la leçon 025.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-025/ex4.patch}}
```

## Visite guidée

Le compilateur ne refuse pas la copie par pédanterie — `Drawable` est une
interface, et le refus est la règle des types abstraits qui fait son travail :

```
error: cannot allocate an object of abstract type ‘snek::Drawable’
note:   because the following virtual functions are pure within ‘snek::Drawable’:
note:     ‘virtual void snek::Drawable::Draw(snek::Grid&) const’
```

Si `Drawable` n'avait pas été abstraite, la même ligne aurait compilé et été
silencieusement fausse : la copie ne garde que la partie de base de l'objet —
le classique *tranchement (slice)* — et `saved.Draw(grid)` aurait appelé la
fonction de la base au lieu de `StatusView::Draw`. C++ a transformé un piège
à l'exécution en erreur de compilation. La correction est
`Drawable &saved = status;` : une référence est un pointeur que le compilateur
déréférence pour vous, donc rien n'est copié et le vptr reste où il
appartient. Preuve que la répartition atterrit toujours dans
`StatusView::Draw` : l'exécution est identique octet par octet à l'état final
de la leçon — même texte de titre, même mort au tick 9 (`frame=28 tick=9
state=dead score=0 dir=up at=2,20`).

*Page traduite de la version anglaise `book/solutions/lesson-025/ex4.md`,
révision `975e844`.*

<!-- translation-source: book/solutions/lesson-025/ex4.md @ 975e844 -->
