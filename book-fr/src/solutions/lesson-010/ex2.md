# Solution : exercice 2 — La mauvaise taille

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — La mauvaise taille](../../lessons/part-0/lesson-010-void-pointer.md) de la leçon 010.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-010/ex2.patch}}
```

## Visite guidée

Le sanitizer nomme le crime immédiatement — une lecture bien plus grande que
la variable d'où elle lit :

```
ERROR: AddressSanitizer: stack-buffer-overflow ...
READ of size 32 ...
    #0 ... in memcpy ...
    #1 ... in DaPush sandbox/ds-kit/ds-kit.c:43
    #2 ... in DemoLongs sandbox/ds-kit/ds-kit.c:107
```

`sizeof nums` est la taille de `struct DynArray` — 32 octets sur cette
machine — aussi le tableau a-t-il déclaré son pas d'élément à 32, et le tout
premier `DaPush` a copié 32 octets depuis le `long v` de huit octets, lisant
24 octets de ce qui le suivait sur la pile. Construit simplement, le même
code a souvent l'*air* de fonctionner : la lecture en excès trouve des
octets de pile plausibles et le programme imprime de la camelote en silence.

La correction est une expression : `DaInit(&nums, sizeof(long))` — ou,
mieux, au style `sizeof v` ou `sizeof *p`, qui suivent le vrai type de
l'élément. Et le compilateur n'a jamais été consulté parce qu'il n'y avait
rien à consulter : `DaInit` prend un `size_t`, `sizeof nums` *est* un
`size_t`, et le tableau générique n'a pas de type d'élément dans sa signature
pour que le désaccord y contrevienne. Le pas est un nombre à l'exécution, que
rien ne vérifie. C'est le mode d'échec `elem_size` de la prose de la leçon :
silencieux, bon marché à produire, et visible seulement sous un sanitizer ou
un très mauvais jour.

*Page traduite de la version anglaise `book/solutions/lesson-010/ex2.md`,
révision `60d447b`.*

<!-- translation-source: book/solutions/lesson-010/ex2.md @ 60d447b -->
