# Solution : exercice 4 — Interrogez votre propre machine

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 4 — Interrogez votre propre machine](../../lessons/part-0/lesson-014-image-headers.md) de la leçon 014.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-014/ex4.patch}}
```

## Visite guidée

La ligne ajoutée stocke l'entier 1 et regarde son premier octet : 1 si
l'octet le moins significatif vit à l'adresse la plus basse, 0 sinon — un
test de boutisme à l'exécution sans macros. Sur cette machine le programme
imprime

```
host byte order: little-endian
```

et `lscpu` est d'accord (`Byte order: Little Endian`, `uname -m` dit
`x86_64`). Sur une machine gros boutiste le même code imprime `big-endian` —
le test inspecte la mémoire, aussi n'a-t-il besoin d'aucune connaissance de
l'architecture. Pour la question finale, regardez quels octets dépendent de
l'hôte : le dump d'*en-tête* ne change jamais, parce que `PutU16LE`/`PutU32LE`
émettent des octets petit boutiste épinglés avec des décalages — la même
sortie partout. Les lignes qui changeraient sont seulement les sondes de
l'hôte : la ligne `0x01020304 in memory` lirait `01 02 03 04`, et la nouvelle
ligne de cet exercice basculerait. Cette coupure est tout le but de la leçon
— la sortie d'un programme peut dépendre de la machine ; les octets d'un
fichier ne doivent pas.

*Page traduite de la version anglaise `book/solutions/lesson-014/ex4.md`,
révision `993f045`.*

<!-- translation-source: book/solutions/lesson-014/ex4.md @ 993f045 -->
