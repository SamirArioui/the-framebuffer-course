# Solution : exercice 2 — Le tableau à moitié libéré

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — Le tableau à moitié libéré](../../lessons/part-0/lesson-008-dynarray.md) de la leçon 008.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-008/ex2.patch}}
```

## Visite guidée

La ligne fautive est `free(da->items)` associée à `da->len = 0` et *rien
d'autre* : c'est la moitié de `DaFree`. Le bloc a disparu, mais `cap` dit
toujours que des emplacements sont disponibles, aussi le `DaPush` suivant
voit-il `len < cap`, saute la croissance, et écrit 24 octets dans de la
mémoire libérée. Sous ASan l'écriture est attrapée là où elle se produit :

```
ERROR: AddressSanitizer: heap-use-after-free ... WRITE of size 24
    #0 ... in DaPush sandbox/ds-kit/ds-kit.c:38
freed by thread T0 here:
    #1 ... in DaClear sandbox/ds-kit/ds-kit.c:43
```

Construit simplement, le même programme ne plante jamais — il imprime un
mensonge (`first=date last=`) depuis de la mémoire périmée, ce qui est pire.

La correction choisit un sens pour *vider* : jeter le contenu, garder le bloc
— `da->len = 0` et rien de plus. Réutiliser le tableau ne demande plus
d'allocation, et la mémoire est toujours libérée plus tard par `DaFree`. Si
vous voulez une remise à zéro qui libère aussi la mémoire, c'est précisément
`DaFree`, pas une troisième chose. Un autre dégât de la remise à zéro : la
lecture finale du pilote de `da.items[9]` est hors limites dès que cinq
éléments seulement sont présents — ASan l'a signalée comme un
heap-buffer-overflow. Indexer `da.items[da.len - 1]` au lieu d'un littéral
garde les lectures à l'intérieur de la longueur. Avec les deux corrections,
ASan sort propre et la sortie lit `first=date last=lemon`.

*Page traduite de la version anglaise `book/solutions/lesson-008/ex2.md`,
révision `f4168c6`.*

<!-- translation-source: book/solutions/lesson-008/ex2.md @ f4168c6 -->
