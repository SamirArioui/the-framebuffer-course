# Solution : exercice 2 — La ligne vide qui lit à l'envers

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — La ligne vide qui lit à l'envers](../../lessons/part-0/lesson-006-undefined-behavior.md) de la leçon 006.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-006/ex2.patch}}
```

## Visite guidée

Pour une ligne vide, `line.len` vaut 0, et `line.len - 1` est une
arithmétique `size_t` : elle retombe sur le plus grand indice possible, et
la lecture atterrit exactement un octet *avant* l'allocation du tampon. Le
sanitizer nomme la géographie avec précision :

```
==105278==ERROR: AddressSanitizer: heap-buffer-overflow on address 0x50600000001f ...
READ of size 1 at 0x50600000001f thread T0
    #0 ... in CountStream .../sandbox/wordcount/wordcount.c:106
    ...
0x50600000001f is located 1 bytes before 64-byte region [0x506000000020,0x506000000060)
allocated by thread T0 here:
    #0 ... in realloc ...
    #1 ... in BufferGrow ...
```

Statut de sortie 1. Notez la récursion avec l'histoire du retournement de la
prose : le retournement non signé *défini* a produit l'accès hors limites
*indéfini*. La construction simple est l'autre moitié de la leçon — elle a
lu un octet périmé et imprimé `last char: ' '` (quel que soit l'octet placé
avant le bloc lors de votre exécution), sortie 0 : le comportement indéfini
n'est pas tenu de planter.

La correction est trois disciplines. La garde `if (line.len > 0)` fait que la
ligne vide veut dire « pas de caractère à rapporter », aussi la
fonctionnalité est-elle honnête sur son propre domaine. La même lecture
gardée tourne à la fin du fichier, où une dernière ligne sans retour à la
ligne final est quand même une ligne et son dernier caractère compte aussi.
Et la lecture passe par `BufferAt`, l'accesseur vérifié — si la garde avait
encore tort une fois, le programme nommerait le mauvais indice et sortirait
proprement au lieu de lire à l'envers. Vérifié sur les deux constructions :
`last char: 'e'` pour `story.txt`, `last char: 'o'` pour un fichier dont la
ligne vide est sautée au-dessus du `hello` précédent, `last char: 'o'` pour
un `hello` nu sans retour à la ligne final, et `last char: 'd'` pour un
fichier finissant par `hello\n\nworld`.

*Page traduite de la version anglaise `book/solutions/lesson-006/ex2.md`,
révision `bb8d4ab`.*

<!-- translation-source: book/solutions/lesson-006/ex2.md @ bb8d4ab -->
