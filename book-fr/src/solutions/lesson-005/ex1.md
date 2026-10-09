# Solution : exercice 1 — Trois verdicts

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — Trois verdicts](../../lessons/part-0/lesson-005-leaks.md) de la leçon 005.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-005/ex1.patch}}
```

## Visite guidée

Les trois verdicts, vérifiés contre la réalité. `./wordcount nope.txt`
imprime `./wordcount: cannot open nope.txt` et le sanitizer ne dit rien,
sortie 0 — `CountStream` n'a jamais tourné, donc aucun bloc n'a jamais été
alloué. `./wordcount empty.txt` imprime `0 0 0 0 empty.txt` et est
pareillement silencieux, sortie 0 — celui-ci *a* fait tourner `CountStream`,
mais un fichier vide n'envoie jamais un octet, `BufferGrow` ne se déclenche
jamais, et il n'y a rien à fuir. `./wordcount story.txt` est l'endroit où le
bloc existe, et avec le `free` commenté le verdict est :

```
Direct leak of 64 byte(s) in 1 object(s) allocated from:
    ...
SUMMARY: AddressSanitizer: 64 byte(s) leaked in 1 allocation(s).
```

avec le statut de sortie 1 — une croissance, un bloc de 64 octets, une
fuite. Deux fichiers donnent `576 byte(s) in 2 object(s)`, en accord avec le
rapport de la leçon elle-même.

Deux détails à garder. Commenter l'appel fait se plaindre le compilateur —
`BufferFree` défini mais non utilisé — ce qui est le
« les avertissements sont du cours » qui fait son travail : la construction
n'aime déjà pas un propriétaire mort. Et la forme de l'exercice est toute la
méthode de la leçon sur les fuites : prédire, puis utiliser l'outil comme un
oracle, et réconcilier les deux.

*Page traduite de la version anglaise `book/solutions/lesson-005/ex1.md`,
révision `bb8d4ab`.*

<!-- translation-source: book/solutions/lesson-005/ex1.md @ bb8d4ab -->
