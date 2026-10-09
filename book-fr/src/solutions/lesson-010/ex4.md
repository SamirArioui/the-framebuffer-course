# Solution : exercice 4 — Le contrat, par écrit

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 4 — Le contrat, par écrit](../../lessons/part-0/lesson-010-void-pointer.md) de la leçon 010.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-010/ex4.patch}}
```

## Visite guidée

(a) Via `elem_size` : passez `sizeof nums` — la taille de la structure — et
chaque `DaPush` copie plus d'octets que n'en possède l'élément (le
débordement de l'exercice 2). Via un mauvais transtypage : lisez un
emplacement `long` comme `const int *` et les mêmes octets répondent à une
question différente (le `7 0` de l'exercice 1). Les deux échappent au
compilateur parce que le `size_t` de `DaInit` et le `void *` de `DaAt` ont
effacé le type de l'élément — il ne reste pas de type déclaré pour qu'un
désaccord y contrevienne, aussi le diagnostic qui se déclencherait sur du
code typé n'a-t-il rien sur quoi se déclencher. (b) Python et Ruby rejettent
les deux erreurs à l'exécution — `TypeError`, jamais de mauvais résultats
produits à moitié silencieusement — parce que chaque valeur porte son type et
que chaque opération le vérifie. Le coût est du travail par opération pour
toujours. Le marché du C est l'inverse : zéro coût à l'exécution, zéro
vérification à l'exécution. (c) `DaAt` et `DaEach` remettent des pointeurs
*à l'intérieur* du bloc de données, pas des copies — c'est ce qui rend le
`memcpy` de `DaPush` et l'appel à `qsort` possibles sans copier des
conteneurs entiers. L'implication : toute croissance qui déclenche `realloc`
peut déplacer le bloc et invalider silencieusement chaque pointeur remis
auparavant, et un pointeur de `DaAt` n'est bon que jusqu'au prochain envoi
qui fait grandir — le contrat `realloc` de la leçon 008, qui fuit désormais
dans celui de l'API.

La vérification confirmante imprime la machinerie au moment où elle tourne —
de vraies lignes d'une exécution :

```
push elem_size=24 at offset 0
push elem_size=24 at offset 24
push elem_size=24 at offset 48
...
push elem_size=8 at offset 0
push elem_size=8 at offset 8
```

Deux tailles d'élément, un seul chemin de code : pas 24 pour `struct Item`,
pas 8 pour `long`, chaque envoi atterrissant à `len * elem_size`.

*Page traduite de la version anglaise `book/solutions/lesson-010/ex4.md`,
révision `60d447b`.*

<!-- translation-source: book/solutions/lesson-010/ex4.md @ 60d447b -->
