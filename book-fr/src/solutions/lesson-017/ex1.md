# Solution : exercice 1 — Les lignes sur disque

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — Les lignes sur disque](../../lessons/part-0/lesson-017-image-file.md) de la leçon 017.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-017/ex1.patch}}
```

## Visite guidée

Les lignes atterrissent bas en haut, une ligne de 24 octets chacune, en
partant du décalage de données 54 qu'a promis l'en-tête — et `ftell` confirme
l'ordre :

```
row 5 -> file offset 54
row 4 -> file offset 78
row 3 -> file offset 102
row 2 -> file offset 126
row 1 -> file offset 150
row 0 -> file offset 174
```

`174 + 24 = 198` : la dernière ligne finit exactement à la fin du fichier, et
aucun octet de remplissage n'a été nécessaire parce que 24 est déjà un
multiple de 4. Le décalage 54 du fichier est donc le pixel d'image `(0, 5)` —
le coin inférieur gauche, fond gris — dont les trois octets sont `20 20 20`
(le gris se lit pareil en BGR). Le décalage 75 est le huitième pixel de cette
même première ligne : le pixel d'image `(7, 5)`, l'extrémité jaune de la
diagonale, stocké BGR comme `00 FF FF`. Si vous avez prédit `FF FF 00`, vous
avez écrit du RGB dans le fichier — l'échange par pixel dans `WriteBmp` est
exactement ce qui transforme l'ordre de notre tampon en celui du fichier, et
l'exercice 2 vérifie cet échange depuis le côté lecture.

*Page traduite de la version anglaise `book/solutions/lesson-017/ex1.md`,
révision `c6657c1`.*

<!-- translation-source: book/solutions/lesson-017/ex1.md @ c6657c1 -->
