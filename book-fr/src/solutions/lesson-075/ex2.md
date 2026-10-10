# Solution : exercice 2 — La carte du magasin

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — La carte du magasin](../../lessons/part-4/lesson-075-lifetime.md) de la leçon 075.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-075/ex2.patch}}
```

## Visite guidée

Le diff est une imprimante et trois appels : `PrintStoreMap` parcourt les
emplacements du magasin (store) et écrit un caractère par emplacement — `#` pour
une entité vivante, `.` pour un emplacement libre — et l'exécution imprime la
carte aux trois moments de la démo. Soixante-quatre caractères, un par emplacement,
dans l'ordre des emplacements.

Les cartes, d'une vraie exécution de l'état final de cette leçon plus le patch :

```
engine: store map (world created): ########........................................................
engine: store map (after the walk): ##.#.#.#........................................................
engine: store map (after the reuse): ########........................................................
```

La première carte est le monde : le héros et sept entités du script, emplacements
0-7, tout le reste libre. La deuxième est le travail de la marche rendu visible —
les emplacements 2, 4 et 6 sont passés de `#` à `.`, exactement les trois que la
marche a retirés au passage, et rien d'autre n'a bougé. La troisième est la
réutilisation : les trois requêtes ont atterri sur les trois emplacements
libérés et la carte est de nouveau `########` — pas `#########...` avec les
nouvelles entités à 8, 9, 10.

Relisez la deuxième carte contre le rapport de la marche : les deux disent la
même chose de deux façons — `visited 8 live entities, once each, in slot order`
sur la première carte, `##.#.#.#` après. Un compte qui dit « 5 vivantes » et une
carte qui dit *lesquelles* cinq sont deux rapports différents, et c'est la carte
qui attrape le bug où le bon nombre d'entités se trouve dans les mauvais
emplacements.

Maintenant la question que la carte ne peut pas trancher. Un emplacement libéré
et un emplacement jamais utilisé se lisent tous deux `.` — le magasin ne retient
que `live`, parce que c'est tout ce dont la *politique* a besoin. La distinction
vient de l'histoire propre de l'exécution : les rapports de création nomment
l'emplacement où chaque entité a atterri (`created in slot 2` … `created in slot
6`), et la démo de cette leçon sait qu'elle n'a jamais créé que dans les
emplacements 0-7 — donc les emplacements 8-63 n'ont jamais été touchés. Pour le
montrer dans la carte, le magasin devrait *se souvenir* : un drapeau `used` par
emplacement, posé à la création et jamais effacé (les trois états sont alors
`live`, `freed`, `never used`), ou une marque du plus haut emplacement jamais
pris. Les deux sont bon marché et aucun n'est exigé par les règles — la règle du
premier emplacement libre garantit déjà l'ordre — mais les deux rendent la
différence *imprimable*, et une différence qu'on peut imprimer est une
différence qu'on peut vérifier.

Si vous avez ajouté le drapeau `used`, un état mérite d'être nommé : un
emplacement peut aller `never used → live → freed → live …`, et **`never used`
est l'état auquel un emplacement ne revient jamais**. Ce n'est pas une perte :
« jamais utilisé » est un fait sur le passé, et la politique ne lit que le
présent.

Rien ici ne touche au magasin, à la marche ou à la boucle du jeu : les cartes
sont trois lignes à côté des rapports que la leçon imprime déjà.

*Page traduite de la version anglaise `book/solutions/lesson-075/ex2.md`,
révision `25e6c7f`.*

<!-- translation-source: book/solutions/lesson-075/ex2.md @ 25e6c7f -->
