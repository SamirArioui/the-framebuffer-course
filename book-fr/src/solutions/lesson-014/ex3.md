# Solution : exercice 3 — Les structures ne font pas de fichiers

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 3 — Les structures ne font pas de fichiers](../../lessons/part-0/lesson-014-image-headers.md) de la leçon 014.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-014/ex3.patch}}
```

## Visite guidée

La sonde imprime, sur cette machine :

```
sizeof(struct)  56
offset type     0
offset file_size 4
offset data_offset 12
offset info_size 16
offset planes   28
offset bpp      30
```

Deux désastres sont visibles. D'abord, `sizeof` fait 56, pas 54 : le
compilateur a inséré deux octets de remplissage après l'`unsigned short type`
de tête pour que les champs `unsigned int` atterrissent sur des frontières de
4 octets. Un `fwrite` brut de la structure écrirait donc 56 octets et chaque
champ après `type` serait décalé de deux octets — un décodeur lirait
`data_offset` depuis l'octet 10 et y trouverait le champ réservé de la
structure. Ensuite, même sans remplissage, les *valeurs* à l'intérieur de la
structure sont stockées dans l'ordre des octets de l'hôte, aussi une machine
gros boutiste écrirait-elle des champs gros boutiste dans un format petit
boutiste. `#pragma pack` ou `__attribute__((packed))` supprime le
remplissage, mais le problème d'ordre des octets et la dépendance à la magie
propre au compilateur restent — la disposition du format est la seule
disposition qui a le droit de compter, et les encodeurs faits main sont la
façon de l'honorer. La réponse du bonus : les deux octets manquants
décaleraient le champ de décalage des données, aussi un décodeur se
positionnerait au mauvais endroit et interpréterait des données de pixels
comme de l'en-tête, ou l'inverse.

*Page traduite de la version anglaise `book/solutions/lesson-014/ex3.md`,
révision `993f045`.*

<!-- translation-source: book/solutions/lesson-014/ex3.md @ 993f045 -->
