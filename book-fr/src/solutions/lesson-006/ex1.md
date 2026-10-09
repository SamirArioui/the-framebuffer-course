# Solution : exercice 1 — La valeur qui n'est pas promise

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — La valeur qui n'est pas promise](../../lessons/part-0/lesson-006-undefined-behavior.md) de la leçon 006.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-006/ex1.patch}}
```

## Visite guidée

Les prédictions : le vérificateur dit quelque chose ; les deux constructions
impriment le même nombre. La réalité :

```
$ ./wordcount-san story.txt
wordcount.c:136:5: runtime error: signed integer overflow: 2147483647 + 1 cannot be represented in type 'int'
big = -2147483648
2 4 21 11 story.txt
$ ./wordcount-plain story.txt
big = -2147483648
2 4 21 11 story.txt
```

`-fsanitize=undefined` nomme l'opération et le type qui ne peut pas contenir
le résultat — cette ligne est le langage qui parle, pas le matériel. La
valeur imprimée est ce que *cette* construction fait : l'addition de la
machine retombe, et `-O0` vous remet les bits retombés. Le silence de la
construction simple est la vraie leçon : rien n'a planté, rien n'a averti,
et si vous livriez un programme qui comptait sur `big` valant
`−2147483648`, il compterait sur une observation sans contrat derrière. Le
comportement indéfini n'est pas une sorte de mauvaise réponse — c'est
l'absence de réponse, et un compilateur a le droit de supposer que `++big`
ne déborde jamais (aucun programme valide ne fait cela) et de transformer le
code en conséquence. La correction pour du vrai code n'est jamais « compter
sur le retournement » : utilisez un type plus large, vérifiez avant
d'additionner, ou rendez la limite explicite.

*Page traduite de la version anglaise `book/solutions/lesson-006/ex1.md`,
révision `51e4e75`.*

<!-- translation-source: book/solutions/lesson-006/ex1.md @ 51e4e75 -->
