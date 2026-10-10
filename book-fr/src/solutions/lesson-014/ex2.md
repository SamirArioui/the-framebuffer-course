# Solution : exercice 2 — Dans le mauvais sens

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — Dans le mauvais sens](../../lessons/part-0/lesson-014-image-headers.md) de la leçon 014.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-014/ex2.patch}}
```

## Visite guidée

`PutU32BE` est `PutU32LE` avec l'ordre des décalages inversé — l'octet le plus
significatif d'abord. Encoder la largeur (8) avec lui stocke `00 00 00 08` aux
décalages d'en-tête 18-21, et le décodeur petit boutiste de `main` lit
fidèlement ces octets en retour comme `0x08000000` :

```
width       134217728
```

Rien n'a mal fonctionné ; l'encodeur et le décodeur ne sont simplement pas
d'accord sur quel bout va en premier. Ce nombre est exactement ce qu'un
décodeur BMP petit boutiste sur n'importe quelle machine calculerait pour un
fichier dont le champ largeur a été écrit gros boutiste — c'est le « largeur
134217728 » de la prose, reproduit exprès. L'expérience confirme l'argument de
la leçon : le format de fichier épingle le petit boutisme, aussi l'*écrivain*
doit-il encoder en petit boutisme quel que soit l'hôte, et une fonction inversée
suffit pour émettre un fichier bien formé dans chaque champ sauf celui qui
compte.

*Page traduite de la version anglaise `book/solutions/lesson-014/ex2.md`,
révision `993f045`.*

<!-- translation-source: book/solutions/lesson-014/ex2.md @ 993f045 -->
