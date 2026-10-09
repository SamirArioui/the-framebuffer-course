# Solution : exercice 1 — Trois cents caractères

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — Trois cents caractères](../../lessons/part-0/lesson-003-char-buffers.md) de la leçon 003.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-003/ex1.patch}}
```

## Visite guidée

La prédiction qui fait trébucher : `1 1 301 300 long.txt`. Les vraies lignes :

```
$ ./wordcount long.txt
line: 255
1 1 301 255 long.txt
$ wc -l -w -c long.txt
  1   1 301 long.txt
```

`lines`, `words` et `bytes` sont comptés droit depuis le flux et concordent
avec `wc` — le fichier est vraiment une ligne, un mot de 300 caractères, 301
octets avec le retour à la ligne. `longest` rapporte 255 parce que c'est tout
ce que le tampon peut *contenir*, et l'impression d'instrumentation montre où
l'information meurt : la ligne est mesurée à `line: 255`, la garde ayant
jeté en silence les caractères après le 255e. La règle derrière : `line`
réserve un octet de ses 256 pour le NUL, aussi
`if (len < sizeof line - 1)` arrête-t-il de stocker à 255 caractères. Le
programme suit exactement ses propres règles — ses règles ne voient juste pas
au-delà du tampon. Le balayage lui-même est innocent : chaque octet de la
ligne est lu et compté ; seule la *copie* est plafonnée. La leçon 004 remplace
le plafond par un tampon qui grandit.

*Page traduite de la version anglaise `book/solutions/lesson-003/ex1.md`,
révision `1772957`.*

<!-- translation-source: book/solutions/lesson-003/ex1.md @ 1772957 -->
