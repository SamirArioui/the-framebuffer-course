# Solution : exercice 1 — L'échec qui se nomme lui-même

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — L'échec qui se nomme lui-même](../../lessons/part-2/lesson-052-tilemap.md) de la leçon 052.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-052/ex1.patch}}
```

## Visite guidée

Le patch scinde `TILE_MALFORMED` en quatre cas nommés et suit l'*étape* où se
trouvait l'analyse quand elle a abandonné — un unique compteur `stage` qui
n'avance que tant que le chargement tient encore, puis une correspondance de
l'étape vers la valeur de l'enum. Le switch de l'exécution rapporte chaque cas
dans ses propres mots. Chaque copie corrompue de la leçon, relancée :

```
bad-header: engine: assets/map.txt: the first line is not width height kinds
bad-row:    engine: assets/map.txt: the rows are wrong (length, count, or an unknown character)
bad-cell:   engine: assets/map.txt: the rows are wrong (length, count, or an unknown character)
bad-rows:   engine: assets/map.txt: the rows are wrong (length, count, or an unknown character)
trailing:   engine: assets/map.txt: there is content after the last row
```

Chaque rapport nomme son étape — et remarquez que les copies corrompues de la
leçon se regroupent déjà d'elles-mêmes : la ligne trop courte, le caractère
inconnu et les lignes manquantes refont tous surface à l'étape des lignes,
parce que c'est là que l'analyse a *découvert* chacun d'eux.

C'est exactement la question de frontière que pose l'énoncé. Une ligne qui
utilise un caractère que la table n'a jamais défini est détectée pendant la
lecture d'une **ligne** — mais l'erreur a été écrite dans la **table** (une
ligne de type manquante, ou une ligne tapée avant le type qui la nomme). Le
chargeur ne peut rapporter que l'endroit où il s'est arrêté ; c'est à l'humain
de décider qui a écrit le bug. L'énoncé honnête de la limite : *le rapport
nomme l'étape de détection, pas l'étape de la cause.*

L'étape suivante — et un bon exercice de prolongement — est le diagnostic au
niveau de l'emplacement : `row 7, column 5: character '?' names no kind`,
`row 4: 19 characters, expected 20`. C'est ce en quoi grandissent les vrais
pipelines d'assets, et les boucles d'analyse portent déjà tous les nombres
nécessaires (`y`, `x`, `len`). Ce cours s'arrête aux noms d'étapes parce que le
fichier fait 20 lignes et que l'étape *plus un éditeur de texte* trouvent le bug
en quelques secondes. Quand vos cartes feront 20 000 lignes, vous voudrez les
coordonnées — et la forme du chargeur ne changera pas pour les obtenir.

Un détail de conception à retenir du patch : le compteur `stage` ne doit
avancer que **tant que `ok` est encore vrai**. La première version de cet
exercice l'avançait sans condition — et chaque échec rapportait « il y a du
contenu après la dernière ligne », parce que le compteur avait continué sa
marche au-delà de l'épave. Les machines à états mentent quand leur état avance
sans leurs conditions ; celle-ci est à deux `if` de l'honnêteté.

*Page traduite de la version anglaise `book/solutions/lesson-052/ex1.md`,
révision `cc9b3fa`.*

<!-- translation-source: book/solutions/lesson-052/ex1.md @ cc9b3fa -->
