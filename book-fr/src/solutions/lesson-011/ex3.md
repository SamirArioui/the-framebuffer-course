# Solution : exercice 3 — La longueur de la marche

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 3 — La longueur de la marche](../../lessons/part-0/lesson-011-hashtable.md) de la leçon 011.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-011/ex3.patch}}
```

## Visite guidée

(a) Une recherche marche une chaîne, et la longueur de chaîne attendue est le
facteur de charge — `len / nbuckets`. Une trouvaille en marche environ la
moitié en moyenne, une non-trouvaille toute entière. Avec `len` mots dans
`nbuckets` seaux, les sondages attendus sont donc `len / (2 * nbuckets)` pour
les trouvailles — une constante tant que le facteur de charge l'est, ce que
dit vraiment « O(1) en moyenne ». (b) Une collision est deux clés différentes
qui hachent vers le même seau ; la chaîne stocke les deux et `strcmp` démêle
laquelle est laquelle. Les hachages sont une décision de *routage*, jamais
une preuve : deux hachages égaux disent « regardez ici », seules des clés
égales disent « trouvé » — comparer les hachages au lieu des clés fusionne
des mots distincts pour toujours. (c) Le cas dégénéré est chaque clé qui
entre en collision dans un seul seau — mauvaise chance avec peu de seaux ou
un attaquant qui choisit les clés — et là chaque recherche est O(n). Les
vraies tables gardent le facteur de charge borné en agrandissant le tableau
de seaux et en re-hachant quand il franchit un seuil, et certaines utilisent
des fonctions de hachage plus fortes pour qu'une entrée ordinaire ne puisse
pas viser un seul seau.

La vérification confirmante journalise chaque recherche ; sur le fichier de
deux lignes de l'exercice, le motif obtenir-ou-placer est visible dans les
paires de non-trouvailles avant chaque insertion :

```
lookup the: miss after 0 probe(s)
lookup the: miss after 0 probe(s)
lookup cat: miss after 0 probe(s)
...
lookup the: 1 probe(s)
```

Deux non-trouvailles pour un mot nouveau — `HtGet` cherche, `HtPut` cherche
encore avant d'insérer — et les trouvailles suivantes coûtent un sondage.
Votre explication devrait prédire exactement cela : les sondages suivent le
facteur de charge, et 8 clés dans 1024 seaux fait un facteur de charge de
0,008 — les chaînes sont à peine des chaînes.

*Page traduite de la version anglaise `book/solutions/lesson-011/ex3.md`,
révision `11ce1ee`.*

<!-- translation-source: book/solutions/lesson-011/ex3.md @ 11ce1ee -->
