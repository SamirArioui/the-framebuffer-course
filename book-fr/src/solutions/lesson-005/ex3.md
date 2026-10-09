# Solution : exercice 3 — Comment le sanitizer sait

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 3 — Comment le sanitizer sait](../../lessons/part-0/lesson-005-leaks.md) de la leçon 005.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-005/ex3.patch}}
```

## Visite guidée

L'exécution instrumentée imprime la vérité dont le rapport est bâti
(capturée avec la sortie redirigée : à travers un seul tube les lignes
`freeing` non mises en tampon arrivent en premier ; sur un terminal les
lignes s'entremêlent — la ligne de `story.txt` après le premier `freeing`,
celle de `long.txt` après le second. Les adresses sont celles de
l'allocation du sanitizer et varient selon les versions et les machines) :

```
freeing 64 bytes at 0x506000000020
freeing 512 bytes at 0x515000000580
2 4 21 11 story.txt
1 1 301 300 long.txt
```

Le mécanisme, dans l'ordre où il se produit. La construction a remplacé
`malloc` et `free` par les versions de l'exécution du sanitizer — pas par
convention mais par édition de liens : chaque site d'appel, le nôtre comme
ceux de la bibliothèque C, se résout vers l'exécution. Chaque allocation est
enregistrée dans une table avec la pile d'appels qui l'a demandée, ce que le
rapport imprime plus tard : la pile a été capturée au moment de
l'*allocation*, pas au moment de l'échec, aussi nomme-t-elle `BufferGrow` et
ses semblables quel que soit la façon dont le bloc est perdu. À la sortie,
l'audit de LeakSanitizer tourne : il fige le processus et balaie chaque
endroit où un pointeur pourrait vivre — registres, piles, globales — pour des
adresses de blocs encore alloués. Tout ce qui est alloué et non référencé est
rapporté comme une fuite ; tout ce qui est encore référencé (comme les
entrées de `stdin`) ne l'est pas.

Sous la construction simple, rien de tout cela n'existe : `free` est celui de
la bibliothèque C, rien n'est enregistré, rien n'audite, et la fuite n'a pas
de symptôme visible à part l'empreinte mémoire du processus lui-même. C'est
toute la proposition de valeur du sanitizer — il convertit une classe de bug
silencieuse, dépendante de l'ordre et de la taille, en un rapport
déterministe à la sortie.

*Page traduite de la version anglaise `book/solutions/lesson-005/ex3.md`,
révision `bb8d4ab`.*

<!-- translation-source: book/solutions/lesson-005/ex3.md @ bb8d4ab -->
