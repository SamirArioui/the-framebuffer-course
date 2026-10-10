# Solution : exercice 2 — Le même pixel en 32 bits compacté

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — Le même pixel en 32 bits compacté](../../lessons/part-0/lesson-013-raw-bytes.md) de la leçon 013.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-013/ex2.patch}}
```

## Visite guidée

L'exécution imprime

```
packed (1,0) word:  0x0000FF00
packed (1,0) bytes: 00 FF 00 00
rgb888 (1,0) bytes: 00 FF 00
```

Le même pixel vert coûte quatre octets compacté et trois octets en RGB888 —
25 % de mémoire de plus pour le confort d'un mot par pixel. Mais regardez les
octets compactés : le *mot* est `0x0000FF00`, et en mémoire il se lit
`00 FF 00 00` — l'octet le moins significatif d'abord. C'est du stockage petit
boutiste, et c'est pourquoi la ligne compactée imprimerait `00 00 FF 00` sur
une machine gros boutiste tandis que la ligne RGB888 imprime `00 FF 00`
partout. Trois canaux d'un seul octet n'ont pas d'ordre d'octets du tout ; un
mot multi-octets en a toujours un. La leçon 014 traite entièrement de cette
asymétrie — et de ce que font les fichiers image pour éviter de dépendre de la
machine qui les a écrits.

*Page traduite de la version anglaise `book/solutions/lesson-013/ex2.md`,
révision `66eaafc`.*

<!-- translation-source: book/solutions/lesson-013/ex2.md @ 66eaafc -->
