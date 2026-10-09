# Solution : exercice 3 — Pourquoi le pont existe

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 3 — Pourquoi le pont existe](../../lessons/part-0/lesson-009-function-pointers.md) de la leçon 009.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-009/ex3.patch}}
```

## Visite guidée

(a) Le paramètre de comparateur de `qsort` est déclaré
`int (*)(const void *, const void *)`, et `CmpByKey` a pour type
`int (*)(const struct Item *, const struct Item *)` — un type de pointeur de
fonction différent. C exige un diagnostic si l'un est passé là où l'autre est
attendu, et si convertir *entre* types de pointeurs de fonction est permis,
appeler à travers un pointeur converti dont le type ne correspond pas à la
fonction réellement là est un comportement indéfini. Le pont est la traversée
conforme : `void *` à l'entrée, transtypage explicite, appel typé.

(b) `g_cmp` compense le fait que `qsort` ne passe aucun contexte au
comparateur — il n'y a pas d'emplacement pour « quel comparateur voulait dire
`DaSort` » (le `qsort_r` de POSIX en ajoute un ; le C standard n'en a pas).
Le contournement cesse d'être sûr dès que deux tris peuvent être en vol en
même temps : du code de comparateur qui appelle lui-même `DaSort`, ou deux
fils qui trient en même temps, se battraient pour la même variable statique
de fichier.

(c) `DaEach` emballe le parcours pour que les appelants n'apportent que du
comportement — utile quand le tableau est l'un parmi d'autres, ou que le
parcours doit rester identique pendant que l'action varie. Pour une boucle
ponctuelle sur un tableau local, un simple `for` est plus court et plus clair
; les callbacks gagnent leur salaire quand la boucle n'est pas à vous de
réécrire.

La vérification confirmante rend la tuyauterie visible — de vrais appels
depuis une exécution :

```
bridge: pear vs apple
bridge: banana vs cherry
bridge: fig vs banana
```

41 appels au pont dans une seule exécution du programme : les deux tris
partagent le pont, et chaque `qsort` de dix éléments pose environ vingt
questions.

*Page traduite de la version anglaise `book/solutions/lesson-009/ex3.md`,
révision `333e81a`.*

<!-- translation-source: book/solutions/lesson-009/ex3.md @ 333e81a -->
