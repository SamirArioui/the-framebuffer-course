# Solution : exercice 1 — L'auxiliaire qui est entré en collision

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — L'auxiliaire qui est entré en collision](../../lessons/part-0/lesson-012-multi-file.md) de la leçon 012.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-012/ex1.patch}}
```

## Visite guidée

Chaque fichier compile propre parce que chaque unité de traduction voit
exactement un `ReportOOM` — le compilateur n'a aucune idée que l'autre
fichier existe. La liaison est l'endroit où les deux définitions se
rencontrent, et le nom est exporté par les deux :

```
/usr/bin/ld: hashtable.o: in function `ReportOOM':
hashtable.c:(.text+0x0): multiple definition of `ReportOOM'; dynarray.o:dynarray.c:(.text+0x0): first defined here
collect2: error: ld returned 1 exit status
```

La correction choisie est le mot-clé de la leçon : `static` sur les deux
définitions. Chaque unité de traduction garde sa propre copie privée — lien
interne, rien d'exporté, aucun nom à faire entrer en collision — et `nm`
montrerait désormais un `t ReportOOM` minuscule dans les deux objets.
L'alternative est de vouloir une copie partagée : définir `ReportOOM` dans un
seul fichier `.c`, le déclarer une fois dans un en-tête que les deux fichiers
incluent, et l'éditeur de liens voit une définition et une promesse par
utilisation.

Quelle correction est juste dépend de la question à laquelle l'auxiliaire
répond. L'échec d'allocation de `DaPush` et `HtInit` est l'urgence privée de
chaque module — six lignes, aucune politique — aussi deux copies statiques
sont-elles honnêtes et sans dépendance. Si l'auxiliaire avait grandi en
politique — journalisation, nettoyage, un format de rapport d'erreur — une
seule unité de traduction partagée empêcherait les copies de diverger. La
règle que suit le kit : `static` par défaut, externe seulement quand un nom
fait réellement partie du contrat d'un en-tête.

*Page traduite de la version anglaise `book/solutions/lesson-012/ex1.md`,
révision `e5cc4fc`.*

<!-- translation-source: book/solutions/lesson-012/ex1.md @ e5cc4fc -->
