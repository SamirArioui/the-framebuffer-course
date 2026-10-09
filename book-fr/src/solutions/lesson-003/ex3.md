# Solution : exercice 3 — Le mot le plus long

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 3 — Le mot le plus long](../../lessons/part-0/lesson-003-char-buffers.md) de la leçon 003.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-003/ex3.patch}}
```

## Visite guidée

La passe parcourt déjà chaque octet et sait déjà quand un mot commence —
`in_word` est la moitié de la machine. L'autre moitié est un compteur
`word_len` qui s'incrémente sur les octets non blancs et est comparé à
`wlongest` partout où un mot peut se terminer : sur une espace blanche, sur un
retour à la ligne, et une fois de plus après la boucle pour un mot final qui
rencontre EOF. La structure gagne le membre et `printf` gagne une colonne :

```
$ ./wordcount story.txt
2 4 21 11 5 story.txt
$ ./wordcount a.txt b.txt
1 1 3 2 2 a.txt
1 1 6 5 5 b.txt
$ ./wordcount partial.txt
0 2 10 10 7 partial.txt
```

Le mot le plus long de `story.txt` est `world` (5) ; celui de `partial.txt`
est `newline` (7) — y compris un mot qui se termine sur EOF, le cas que
l'après-boucle attrape.

Maintenant `long.txt`, la ligne de 300 caractères :
`1 1 301 255 300 long.txt`. La nouvelle colonne dit 300 — la vérité — tandis
que `longest` dit 255. Le compteur de mots s'incrémente sur chaque octet du
flux ; seul le *tampon* de ligne a un plafond. Même passe, même fichier, et
les deux colonnes ne sont pas d'accord parce que l'une d'elles traverse
`line[256]`. C'est le plafond de l'exercice 1, vu sous un second angle.

*Page traduite de la version anglaise `book/solutions/lesson-003/ex3.md`,
révision `1772957`.*

<!-- translation-source: book/solutions/lesson-003/ex3.md @ 1772957 -->
