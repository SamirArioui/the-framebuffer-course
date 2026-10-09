# Solution : exercice 2 — Le remplissage est de la vraie mémoire

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — Le remplissage est de la vraie mémoire](../../lessons/part-0/lesson-007-struct-layout.md) de la leçon 007.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-007/ex2.patch}}
```

## Visite guidée

`DumpBytes` parcourt l'objet en octets `unsigned char` et imprime chacun en
hexadécimal — une exécution ici :

```
41 00 00 00 00 00 00 00 2a 00 00 00 00 00 00 00 5a 71 27 5a fc 7f 00 00
41 00 00 00 00 00 00 00 2a 00 00 00 00 00 00 00 5a 00 00 00 00 00 00 00
```

La première ligne est la structure affectée à la main, la seconde celle mise
à zéro par `memset`. `41` est `tag` = 'A', `2a 00 00 00 00 00 00 00` est
`score` = 42 (l'octet faible d'abord — le boutisme est le sujet de la leçon
014), et `5a` est `flag` = 'Z'. Les octets entre eux sont du remplissage, et
la première ligne les montre contenant ce que la pile y a laissé : le trou de
queue changeait à chaque exécution de ce programme (`5a 71 27 5a fc 7f 00 00`,
puis `5a 32 ed ac fc 7f 00 00`, …), et le trou intérieur se lisait comme zéro
— par chance, pas par règle.

`memset` est la seule façon fiable de mettre à zéro le remplissage d'une
structure, et la seconde ligne le prouve. Cela compte chaque fois que des
octets sont comparés ou hachés en tant qu'ensemble : un `memcmp` sur deux
structures aux champs égaux peut quand même rapporter « différent » parce que
leurs remplissages diffèrent.

*Page traduite de la version anglaise `book/solutions/lesson-007/ex2.md`,
révision `fa0fc1a`.*

<!-- translation-source: book/solutions/lesson-007/ex2.md @ fa0fc1a -->
