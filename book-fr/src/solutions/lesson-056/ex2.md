# Solution : exercice 2 — Le pas et le mur

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — Le pas et le mur](../../lessons/part-2/lesson-056-mover.md) de la leçon 056.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-056/ex2.patch}}
```

## Visite guidée

Deux prédictions à écrire d'abord.

**Le bond.** L'étrange paire du journal —

```
engine: sprite at 18,156 (t=3.867)
engine: sprite blocked at 18,156 (t=4.528)
engine: sprite unblocked at 18,146 (t=4.568)
```

— n'est pas un mur. Les horodatages sont la preuve : la frame à t=3.867 est
arrivée 0,315 s après la précédente, et la frame à t=4.528 est arrivée
**0,66 s** plus tard. Le mover avance de `240 px/s × dt`, donc cette frame a
tenté un pas de `240 × 0.66 ≈ 158 pixels` — de y=156 jusqu'au-delà du bord
supérieur de la carte. `TileRectSolid` a vu un rectangle hors du monde, le bord
est solide, et le pas a été refusé *entier* — y compris ses 137 pixels
parfaitement valides. La frame suivante (0,04 s, un pas normal) s'est bien
passée, d'où le saut de la position 156 → 146 sur la ligne « unblocked ». La
prédiction à formuler avant de lancer quoi que ce soit : **le rapport blocked à
156 est un pas géant refusé, pas un mur** — et la double précision du rapport
est ce qui sépare « n'a pas bougé » (156.00 → 156.00) de « a bougé de moins
d'un pixel » (que le rapport entier de l'état final de la leçon aurait aussi
appelé blocked).

**La distance d'arrêt.** En allant vers la gauche contre le mur de bordure, le
sprite ne peut pas dépasser x = 16 — sa boîte de 16 pixels de large
chevaucherait alors les pixels 0..15 du mur. Mais le mover avance par pas, pas
au millimètre : le dernier pas *accepté* atterrit là où la foulée du déplacement
le place, et le premier *refusé* l'y laisse. La prédiction n'est donc pas
« exactement 16.00 » mais **à un pas de 16** — et la taille du pas ici est
240 × dt, soit 5 à 10 pixels à l'espacement des frames de l'entrée scriptée. Le
rapport fractionnaire du patch montre ce qui s'est réellement passé :

```
engine: sprite blocked at 17.77,232.00 (t=4.796)
engine: sprite unblocked at 17.77,232.00 (t=5.548)
```

**17.77** — à 1,77 pixel en deçà du 16.00 parfait. Le nombre n'est pas une
constante : il dépend de l'endroit où les foulées des pas tombent contre le mur
(l'exécution précédente s'arrêtait à 18,17 avec un cadencement d'entrée
différent). Le mur est exact ; l'arrivée est un accident de foulée.

Que *devrait* faire le mover des pas trop grands pour être franchis ? Trois
réponses, dans l'ordre où les jeux les rencontrent d'habitude :

1. **Borner dt.** Plafonner le pas à, disons, 1/30 s de mouvement. Simple, et
   cela borne à quel point une frame peut se tromper — au prix d'un sprite qui
   avance au ralenti pendant une longue accalmie (le monde ne récupère pas son
   temps perdu).
2. **Sous-pas.** Découper le déplacement prévu en pas de la taille d'une tuile
   (ou plus petits) et les appliquer jusqu'à ce que la requête refuse. Le
   sprite glisse jusqu'au mur exactement — les bons pixels du pas géant ne sont
   plus jetés. C'est ce que veut un vrai mover, et cela coûte une petite
   boucle.
3. **Balayage.** Demander « jusqu'où puis-je aller dans cette direction ? »
   plutôt que « puis-je être ici ? » — la requête devient un rayon ou un
   rectangle balayé. La bonne réponse pour les movers rapides (projectile,
   dash) qu'on ne peut pas laisser traverser les murs, même à la granularité
   des sous-pas.

Le héros de la partie 4 prendra l'option 2 avec l'option 1 derrière — et
l'habitude de cumul des frames de la leçon 036 montrera le coût du mover à
l'intérieur de la phase `update` quand ce sera fait. L'état final de la leçon
laisse le pas tout ou rien *à dessein* : le comportement est honnête, le
rapport est honnête, et la limite est désormais une chose mesurée et nommée
plutôt qu'une surprise.

*Page traduite de la version anglaise `book/solutions/lesson-056/ex2.md`,
révision `853ad02`.*

<!-- translation-source: book/solutions/lesson-056/ex2.md @ 853ad02 -->
