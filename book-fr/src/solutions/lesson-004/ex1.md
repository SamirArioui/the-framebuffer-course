# Solution : exercice 1 — La suite des doublons

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — La suite des doublons](../../lessons/part-0/lesson-004-heap-buffers.md) de la leçon 004.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-004/ex1.patch}}
```

## Visite guidée

La prédiction : huit croissances, capacités 64, 128, 256, 512, 1024, 2048,
4096, 8192. L'instrument le confirme exactement :

```
$ ./wordcount huge.txt
cap 64
cap 128
cap 256
cap 512
cap 1024
cap 2048
cap 4096
cap 8192
1 1 5001 5000 huge.txt
```

Trois points de réconciliation. Le premier envoi fait croître de `cap = 0` à
64 — ce `realloc(NULL, 64)` est un `malloc` déguisé, aussi le premier bloc et
les suivants sont-ils le même mécanisme. Le compte est de huit parce que la
ligne a besoin de 5001 octets stockés (5000 caractères plus le NUL que
`BufferPush` ajoute avant de mesurer) et que la suite ne fait que passer par
4096 en route vers 8192. Et la capacité finale n'est pas 5000 parce que la
capacité est de la marge, pas de l'ajustement : doubler dépasse d'un facteur
jusqu'à deux, le tampon se termine à 8192 avec 3191 octets de jeu, et ce jeu
est exactement ce qui achète le compte de croissance O(log N). Une politique
de croissance-exacte se terminerait à 5001 et coûterait une réallocation pour
presque chaque octet — l'exercice 2 mesure cet échange.

*Page traduite de la version anglaise `book/solutions/lesson-004/ex1.md`,
révision `354f350`.*

<!-- translation-source: book/solutions/lesson-004/ex1.md @ 354f350 -->
