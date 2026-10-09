# Solution : exercice 4 — Ce que `realloc` promet vraiment

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 4 — Ce que `realloc` promet vraiment](../../lessons/part-0/lesson-008-dynarray.md) de la leçon 008.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-008/ex4.patch}}
```

## Visite guidée

(a) Sous le doublement, les croissances arrivent aux capacités 4, 8, 16, …,
n/2 et les éléments re-stockés à travers toutes somment moins que n — une
suite géométrique — aussi sur *n* envois la copie ajoute-t-elle une moyenne
constante à chaque envoi. Sous la croissance d'un emplacement, la croissance
numéro *k* re-stocke *k* éléments et le total est n(n+1)/2 : quadratique, et
aucune moyenne ne le sauve. (b) En cas d'échec `realloc` renvoie `NULL` et
l'ancien bloc est laissé intact, alloué, et à vous — aussi
`da->items = realloc(da->items, ...)` écrase-t-il le seul pointeur vers un
bloc parfaitement bon avec `NULL` : le bloc fuit instantanément. Le
temporaire `p` garde l'ancien pointeur en sécurité jusqu'à ce que le succès
soit su. (c) `realloc` renvoie le même pointeur quand il peut étendre le bloc
là où il se tient — de l'espace libre doit immédiatement le suivre — et un
différent quand il doit allouer, copier, et libérer.

La vérification confirmante étiquette chaque cas au moment où il se produit.
Trois croissances, trois réponses différentes :

```
realloc cap 0 -> 4 (fresh)
realloc cap 4 -> 8 (moved)
realloc cap 8 -> 16 (in place)
```

`fresh` est la première croissance : il n'y a pas d'ancien bloc à déplacer ou
étendre. La seconde croissance a déplacé ; la troisième a étendu sur place.
Lancez le pilote avec cinquante envois et le flux montre le mélange que le
tas vous donne — ce qui est exactement pourquoi la *règle* de croissance ne
doit pas dépendre de la bonté du tas (voir l'exercice 3).

*Page traduite de la version anglaise `book/solutions/lesson-008/ex4.md`,
révision `f4168c6`.*

<!-- translation-source: book/solutions/lesson-008/ex4.md @ f4168c6 -->
