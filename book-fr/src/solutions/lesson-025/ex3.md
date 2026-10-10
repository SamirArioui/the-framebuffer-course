# Solution : exercice 3 — La vtable sous la loupe

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 3 — La vtable sous la loupe](../../lessons/part-0/lesson-025-cpp-subset.md) de la leçon 025.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-025/ex3.patch}}
```

## Visite guidée

La répartition instrumentée imprime, sur une machine :

```
vptr=0x55a956616ce8 draw slot=0x55a956613e9c
```

Comparez les deux avec la table de symboles du binaire lui-même :

```
$ nm snek | grep -E 'GridView4Draw|_ZTVN4snek8GridViewE' | c++filt
0000000000001e9c T snek::GridView::Draw(snek::Grid&) const
0000000000004cd8 V vtable for snek::GridView
```

L'emplacement de dessin est `GridView::Draw` plus l'adresse de chargement :
soustrayez le décalage que `nm` rapporte (`0x1e9c`) de `0x55a956613e9c` et vous
obtenez l'adresse de chargement de l'exécutable — soustrayez `0x4cd8 + 16` du
vptr et vous obtenez le même nombre (un binaire PIE est relocalisé au
démarrage ; `nm` montre des décalages, exactement comme dans la leçon 024).
Donc l'emplacement zéro de cette table contient l'adresse de `GridView::Draw`,
et le vptr contient l'adresse de la table au-delà des deux mots d'en-tête
(décalage-vers-le-haut et pointeur de type) qui précèdent les pointeurs de
fonction. `views[i]->Draw(grid)` se compile en trois instructions en esprit :
charger le premier mot de l'objet, charger l'emplacement vers lequel il pointe,
l'appeler.

C'est la table de commandes de la leçon 024, automatisée : le compilateur écrit
une table par classe, plante un pointeur par objet, et remplace le balayeur par
un indice fixe. Les lignes que `RunCommand` appariait à la main sont désormais
une recherche que fait le matériel ; ajouter une sous-classe (exercice 1) ne
modifie aucun code de répartition du tout.

*Page traduite de la version anglaise `book/solutions/lesson-025/ex3.md`,
révision `975e844`.*

<!-- translation-source: book/solutions/lesson-025/ex3.md @ 975e844 -->
