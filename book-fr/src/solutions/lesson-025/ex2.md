# Solution : exercice 2 — Le mot caché

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — Le mot caché](../../lessons/part-0/lesson-025-cpp-subset.md) de la leçon 025.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-025/ex2.patch}}
```

## Visite guidée

La prédiction tentante est « zéro — les deux vues ne portent aucune donnée ».
C++ ne laisse jamais un objet complet avoir une taille nulle (des objets
distincts ont besoin d'adresses distinctes), aussi le jumeau vide
imprime-t-il 1 ; la question est ce que les vues impriment en plus. L'exécution
instrumentée rapporte, sur une machine 64 bits :

```
sizes: GridView=8 StatusView=8 PlainView=1
```

Huit octets, c'est exactement un mot de la taille d'un pointeur : le **vptr**,
stocké dans chaque objet dont la classe a des fonctions virtuelles.
`PlainView` a une fonction membre ordinaire, pas de `virtual`, aussi ne garde-t-
il que l'octet obligatoire. Le vptr est ce que `views[i]->Draw(grid)` charge en
premier — il pointe vers la table de pointeurs de fonction de sa classe, que
l'exercice 3 suit jusqu'au bout. Sur une machine 32 bits attendez-vous à
`4 4 1` : le mot y est un pointeur aussi. Notez que le `Draw` du jumeau n'est
jamais appelé ni défini — `sizeof` est une question de compilation, répondue
avant que rien de tout cela ne tourne.

*Page traduite de la version anglaise `book/solutions/lesson-025/ex2.md`,
révision `975e844`.*

<!-- translation-source: book/solutions/lesson-025/ex2.md @ 975e844 -->
