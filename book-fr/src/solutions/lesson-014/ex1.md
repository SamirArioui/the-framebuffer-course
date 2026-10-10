# Solution : exercice 1 — Largeur 258

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — Largeur 258](../../lessons/part-0/lesson-014-image-headers.md) de la leçon 014.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-014/ex1.patch}}
```

## Visite guidée

La largeur 258 est `0x00000102`, aussi ses octets petit boutiste sont
`02 01 00 00` ; la hauteur 4 est `04 00 00 00`. La seconde ligne du dump
(décalages d'en-tête 16-31) sort

```
00 00 02 01 00 00 04 00 00 00 01 00 18 00 00 00
```

et l'impression décodée rapporte `width 258`, `height 4` — l'encodeur et le
décodeur font l'aller-retour. 258 est la valeur intéressante parce que ses
quatre octets ne sont *pas* tous sauf un à zéro : `02 01 00 00` montre l'ordre
des octets à l'œuvre, tandis que la largeur 8 (`08 00 00 00`) ne peut pas
distinguer « petit boutiste » de « le nombre est petit par hasard ». Notez
aussi que le champ taille du fichier a grandi à 3158 : la ligne fait
désormais 774 octets, ce qui a besoin de 2 octets de remplissage pour
atteindre la frontière de 4 octets, donnant 776 octets par ligne — la
première observation concrète du remplissage dont vit la leçon 017.

*Page traduite de la version anglaise `book/solutions/lesson-014/ex1.md`,
révision `993f045`.*

<!-- translation-source: book/solutions/lesson-014/ex1.md @ 993f045 -->
