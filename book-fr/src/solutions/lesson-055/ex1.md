# Solution : exercice 1 — La politique est la vôtre

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — La politique est la vôtre](../../lessons/part-2/lesson-055-collision.md) de la leçon 055.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-055/ex1.patch}}
```

## Visite guidée

Le patch réécrit les deux mêmes requêtes avec la réponse hors limites opposée
— `TilePointFree` / `TileRectFree`, qui n'en diffèrent que sur un point : les
coordonnées hors de la carte répondent *libre*, si bien que seules les
cellules que la carte possède réellement peuvent rapporter une collision. Les
quatre cas hors limites sous les deux politiques :

```
engine: policy: point left of the map — outside-solid solid, outside-free free
engine: policy: point past the right edge — outside-solid solid, outside-free free
engine: policy: rect leaving the map — outside-solid solid, outside-free solid
engine: policy: rect entirely past the edge — outside-solid solid, outside-free free
```

La prédiction, puis la réconciliation. Une requête située *entièrement* hors
de la carte est décidée par la politique seule — les deux points et le
rectangle lointain basculent donc. Mais **`rect leaving the map` ne bascule
pas** : `(−8, 100, 16, 16)` va de x = −8 à x = 7, et sa partie dans la carte —
les pixels x = 0..7 — est la colonne de bordure de la carte, qui est un mur.
Les cellules dans la carte répondent *solide* sous **toutes** les politiques ;
la politique ne décide que la partie de la requête qui sort de la carte.

C'est le point plus profond sur la gestion du hors-limites : ce n'est pas un
comportement global, mais la réponse à *une partie* d'une requête. Un
rectangle à cheval sur le bord pose deux questions à la fois — « la partie
intérieure touche-t-elle quelque chose ? » (les données de la carte répondent)
et « la partie extérieure est-elle autorisée ? » (la politique répond) — et la
collision rapportée est `inside-something OR outside-not-allowed`. Le patch
`TileRectFree` rend cela littéral : il borne d'abord le rectangle à la carte
(répondant à la deuxième question en jetant l'extérieur) puis interroge les
cellules (la première question).

Quelle politique votre jeu veut-il ?

- **L'extérieur est solide** — le monde est une île ; rien n'en sort. Juste
  pour une arène bornée, un donjon, tout jeu où la carte *est* le monde. La
  démo de ce cours l'utilise.
- **L'extérieur est libre** — la carte est un lieu, pas une frontière. Juste
  pour des mondes à défilement entourés de vide, pour des éditeurs, pour des
  jeux où quitter la carte est permis et géré ailleurs (un état « perdu », un
  retournement, un mur invisible dessiné comme art).

Ni l'une ni l'autre n'est plus correcte ; un jeu qui ne choisit ni l'une ni
l'autre — un défaut non choisi, ce que la vérification des limites de `TileAt`
renvoie par hasard — a déjà choisi l'une par accident. C'est la vraie réponse
de l'exercice : **la politique est la vôtre, alors nommez-la.**

*Page traduite de la version anglaise `book/solutions/lesson-055/ex1.md`,
révision `c39d430`.*

<!-- translation-source: book/solutions/lesson-055/ex1.md @ c39d430 -->
