# Solution : exercice 1 — La ligne, à deux tailles

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — La ligne, à deux tailles](../../lessons/part-4/lesson-081-entities-row.md) de la leçon 081.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-081/ex1.patch}}
```

## Visite guidée

Le diff est une ligne de contexte : la table du budget imprime combien
d'entités la marche visite par frame à côté de la table elle-même, si bien que
le nombre d'une ligne peut se lire contre sa taille. La mesure est à vous de
prendre — des lignes ajoutées à `assets/entities.txt` (de la donnée, aucune
recompilation de la logique du moteur) et la table de l'exécution elle-même,
lue à chaque taille.

Les mesures, d'après de vraies exécutions de l'état final de cette leçon plus
le patch sur la machine de l'auteur — la tranche à trois tailles de table,
`ENTITY_CAP` augmenté à 256 pour la plus grande (un changement jetable, le 64
de la leçon étant restauré ensuite) :

| entités parcourues par frame | ligne `entities` |
| ------------------------- | -------------- |
| 2 | 0.001 ms |
| 20 | 0.002 ms |
| 200 | 0.007 ms (deux exécutions, identiques au chiffre près) |

**Le coût de la marche est-il linéaire ?** Oui — avec un plancher. La forme est
`fixed + count × per-entity` : les ~0.001 ms du début sont le coût de la mesure
elle-même (les deux appels à `platform::Now()` et la mise en place de la
boucle, payés que le magasin contienne deux entités ou aucune), et le coût
marginal est d'environ 30-50 nanosecondes par entité (de 20 à 200 entités, la
ligne croît de 0.005 ms sur 180 entités ≈ 28 ns chacune). Dix fois plus
d'entités, c'est quelques fois la ligne, pas dix fois la ligne, parce qu'à ces
tailles le plancher fait l'essentiel du nombre.

**Combien d'entités en 0.1 ms ?** À ~30 ns par entité, quelque trois mille — et
le nombre est un nombre de *machine* : votre CPU, vos options de compilation,
votre ligne. (Cette machine : `-O0 -g`, un affichage Xvfb, pas de son.) La
vraie réponse de l'exercice est la mesure que vous avez prise, avec votre
machine nommée à côté — une table de budget sans sa machine est une rumeur, la
formule de la leçon 069.

**La réconciliation**, de l'exécution à 200 entités contre son propre journal —
la discipline de la leçon, faite une fois ici : les lignes `frame N:` de
l'exécution portent `entities` dans chaque enregistrement (`update 0.018 ms
(entities 0.002), …`), et la moyenne de cette colonne sur les frames de
l'exécution donne le `entities 0.007 ms` de la table. La ligne est le journal ;
le journal est les frames.

Deux choses que la ligne ne dit *pas* dans cet exercice. Elle ne dit pas que le
jeu est lent — 0.007 ms d'une frame de ~1.9 ms, c'est 0.4 %, et la marche de la
tilemap possède encore le render. Et ce n'est pas une prédiction pour la partie
5 : deux cents entités qui font chacune une multiplication par frame ne sont
pas deux cents entités qui font chacune tourner une IA, et la ligne mesurera
*celles-là* tout aussi honnêtement quand elles arriveront. La valeur de
l'instrument, c'est qu'il continue de répondre.

Rien ici ne touche la marche, l'enregistrement ni la boucle du jeu : le patch
est une ligne à côté de la table que la leçon imprime déjà.

*Page traduite de la version anglaise `book/solutions/lesson-081/ex1.md`,
révision `15b1166`.*

<!-- translation-source: book/solutions/lesson-081/ex1.md @ 15b1166 -->
