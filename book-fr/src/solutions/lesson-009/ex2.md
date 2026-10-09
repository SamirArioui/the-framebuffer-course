# Solution : exercice 2 — Chercher par prédicat

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — Chercher par prédicat](../../lessons/part-0/lesson-009-function-pointers.md) de la leçon 009.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-009/ex2.patch}}
```

## Visite guidée

`DaFind` est la boucle de `DaEach` avec une question au lieu d'une action :
le hook ici est un prédicat — `int (*)(const struct Item *)` — qui répond
oui (non nul) ou non (zéro) sur un élément. Le parcours s'arrête au premier
oui et remet un pointeur vers l'élément ; un parcours sans aucun oui remet
`NULL`. Les deux requêtes du pilote impriment :

```
found grape 7
mango: not found
```

La réponse `NULL` n'est pas une décoration facultative — c'est la seule
réponse que la seconde requête puisse donner, aussi le pilote vérifie-t-il
avant de toucher `hit`, la même discipline que `fopen` a enseignée avec les
poignées de fichier dès la leçon 001. Un pointeur qui peut être « vide » est
vérifié ; cette habitude est l'essentiel du style défensif du C.

Deux petites notes. `DaFind` renvoie `const struct Item *` exprès : une
recherche ne doit pas être une porte dérobée vers la mutation. Et regardez ce
que coûtent les deux prédicats : une fonction nommée pour chacun, et aucun
des deux ne peut poser sa question avec un argument — `KeyIsGrape` code en
dur `grape`. Porter du *contexte* dans un callback est la peine dont traite
la leçon 010 ; le `g_cmp` de l'étape de code est le même problème avec un
autre chapeau.

*Page traduite de la version anglaise `book/solutions/lesson-009/ex2.md`,
révision `333e81a`.*

<!-- translation-source: book/solutions/lesson-009/ex2.md @ 333e81a -->
