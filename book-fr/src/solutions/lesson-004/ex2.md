# Solution : exercice 2 — Un octet à la fois

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — Un octet à la fois](../../lessons/part-0/lesson-004-heap-buffers.md) de la leçon 004.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-004/ex2.patch}}
```

## Visite guidée

Le changement de politique tient en une ligne — `buf->cap + 1` à la place de
l'expression de doublement — et les mesures sur une machine (gcc 13.3.0,
x86-64) sont :

```
grow-by-one, 100k:  real 0m0.001s
grow-by-one, 1M:    real 0m0.008s
doubling,   100k:   real 0m0.001s
doubling,   1M:     real 0m0.005s
```

À 100 000 caractères, les politiques sont indiscernables ; à 1 000 000,
l'alternative est environ 1,6× plus lente — quelques millisecondes, pas la
catastrophe que prédit la théorie des copies. Si chaque croissance copiait,
un-à-la-fois déplacerait environ N²/2 = 5×10¹¹ octets et tournerait pendant
des minutes ; visiblement elle n'a presque rien déplacé. Où est passée la
copie ? Dans la première clause du contrat de `realloc` : quand rien ne se
trouve au-dessus du bloc dans le tas, il grandit **sur place** et copie zéro
octet. L'instrument d'adresses de l'exercice 3 est la preuve — copiez sa
ligne d'impression dans cette variante (les deux patches touchent tous deux
`BufferGrow`, aussi une fusion à la main est-elle nécessaire). Sur le fichier
de 100 000, chaque croissance imprime la même adresse de bloc ; sur celui de
1 000 000, le bloc se déplace quelques fois dans la plupart des exécutions.
La croissance sur place est l'heuristique de l'allocation quand la place est
là, pas une garantie. Ce que les chronométrages prouvent, c'est le coût d'un
million d'appels à `realloc` ; ce qu'ils ne prouvent pas, c'est le pire cas,
qui apparaît dès que le tas est assez fragmenté pour que le bloc doive
bouger. La théorie borne ce cas ; la mesure a montré celui-ci. Mesurez
d'abord — y compris mesurer que la théorie ne vous fait pas de mal en ce
moment.

*Page traduite de la version anglaise `book/solutions/lesson-004/ex2.md`,
révision `bb8d4ab`.*

<!-- translation-source: book/solutions/lesson-004/ex2.md @ bb8d4ab -->
