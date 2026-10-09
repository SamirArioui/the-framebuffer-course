# Solution : exercice 1 — Du remplissage dans une structure neuve

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — Du remplissage dans une structure neuve](../../lessons/part-0/lesson-007-struct-layout.md) de la leçon 007.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-007/ex1.patch}}
```

## Visite guidée

La prédiction que font la plupart des gens est `sizeof(struct Triple) == 6` —
un octet, quatre octets, un octet. La machine dit 12 :

```
== struct Triple ==
sizeof(struct Triple) = 12
  a offset 0
  b offset 4
  c offset 8
```

Parcourez la règle d'alignement et les nombres suivent. `a` prend le décalage
0. `b` est un `int` d'alignement 4, aussi ne peut-il pas se placer au décalage
1 : le compilateur remplit trois octets et le place à 4. `c` suit à 8. Les
membres finissent au décalage 9, mais l'alignement de la structure est 4 — le
plus grand de ceux de ses membres — aussi la taille est-elle arrondie à 12.
Ces trois octets de queue sont du remplissage aussi : sans eux, un tableau de
`Triple` placerait le second `a` au décalage 9 et son `b` serait à nouveau
mal aligné. C'est pourquoi `sizeof(struct Triple[2])` fait 24 et pas 18.

Refaites l'arithmétique contre chaque structure que vous rencontrez et les
nombres cessent de vous surprendre : des membres à des multiples de leur
alignement, une taille arrondie à l'alignement de la structure.

*Page traduite de la version anglaise `book/solutions/lesson-007/ex1.md`,
révision `fa0fc1a`.*

<!-- translation-source: book/solutions/lesson-007/ex1.md @ fa0fc1a -->
