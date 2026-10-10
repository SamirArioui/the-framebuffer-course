# Solution : exercice 2 — Aller-retour

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — Aller-retour](../../lessons/part-0/lesson-017-image-file.md) de la leçon 017.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-017/ex2.patch}}
```

## Visite guidée

`CheckBmp` traite le fichier exactement comme le ferait le décodeur d'un
inconnu : se positionner, décoder les champs avec les lecteurs de la leçon
014, échantillonner les pixels à travers la disposition documentée.
L'exécution rapporte l'accord sur chaque point :

```
check: size field 198, width 8, height 6, bpp 24
check: file (7,5) = 255 255 0, buffer = 255 255 0
```

Le positionnement du pixel est la moitié intéressante : `(w - 1, h - 1)` est
le dernier pixel de la *première* ligne sur disque, aussi son décalage dans le
fichier est `54 + (w - 1) * 3` — pas besoin d'arithmétique de lignes parce que
l'ordre bas en haut met la ligne du bas de l'image en premier. Les trois
octets reviennent bleu-vert-rouge et sont dés-échangés avant la comparaison
avec `GetPixel`, qui renvoie du RGB — l'aller-retour traverse l'ordre des
octets du format deux fois et atterrit là où il a commencé. Pointez la
comparaison du tampon sur `(w - 1, 0)` à la place et les deux lignes cessent
de correspondre — le fichier dit toujours `255 255 0` (jaune) tandis que le
tampon répond `0 255 255` (le cyan en `(7, 0)`) — ce qui est le détecteur de
désaccord qui fait son travail. Un écrivain qui vérifie sa propre sortie
attrape les bugs de format à la même exécution où ils sont introduits.

*Page traduite de la version anglaise `book/solutions/lesson-017/ex2.md`,
révision `c6657c1`.*

<!-- translation-source: book/solutions/lesson-017/ex2.md @ c6657c1 -->
