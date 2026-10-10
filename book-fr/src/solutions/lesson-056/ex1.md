# Solution : exercice 1 — La hitbox

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — La hitbox](../../lessons/part-2/lesson-056-mover.md) de la leçon 056.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-056/ex1.patch}}
```

## Visite guidée

Le patch fait deux choses : les requêtes du mover se rétrécissent en une boîte
en retrait de deux pixels de chaque côté (`(int)next_x + 1`, `width - 2`), et
une vérification de démarrage compare les deux boîtes à une position —
`x = 15`, un pixel dans la dernière colonne du mur :

```
engine: hitbox check: at x=15 — full box solid, 2px inset free
```

Cette paire, c'est toute l'idée en une ligne. À `x = 15`, le dessin du sprite
s'étend de 15 à 30 — sa colonne de gauche repose sur le pixel 15, qui est le
dernier pixel du mur de bordure. La boîte complète 16×16 chevauche donc une
case de mur et la requête répond **solide**. La boîte en retrait couvre les
pixels 16..29 — entièrement du sol — et répond **libre**. Le mover muni de la
boîte en retrait est autorisé à se tenir à `x = 15`, et le joueur voit le
dessin du sprite fusionner d'un pixel avec le bord du mur : aucun trou, aucune
couture visible.

Maintenant la seconde moitié honnête de la vérification — le mover poussé
contre le mur. L'exécution s'arrête sur `blocked at 18,232`, la même position
entière que rapportait le mover à boîte complète. Le pixel supplémentaire de la
hitbox est invisible à la taille de pas du mover : le sprite avance par pas
d'environ 10 pixels (240 px/s × le dt de la frame), et le dernier pas qui
*rentre* atterrit à 17–18 quoi que permette le retrait d'un pixel de la boîte.
La différence entre les requêtes est réelle (la vérification le prouve) ; le
*mover* ne peut pas la montrer sans des pas plus fins — ce qui est exactement
le sujet de l'exercice 2. Une vérification qui isole la variable (la
comparaison de démarrage) a gagné sa place à côté de la vérification
comportementale.

La question de design — pourquoi les jeux gardent une hitbox plus petite que le
dessin :

- **L'indulgence.** Les joueurs lisent le dessin comme le personnage, mais une
  hitbox exactement au bord du dessin rend les frôlements injustes : le sprite
  *semble* avoir dépassé le pilier et il s'arrête quand même. Deux pixels de
  retrait transforment « j'ai clairement touché ça » en « je l'ai manqué de
  justesse » : la même physique, en plus courtois.
- **La lisibilité en mouvement.** À vitesse élevée, une hitbox serrée fait
  accrocher le sprite sur des coins que le joueur ne voit pas ; une boîte plus
  petite se faufile dans les creux visuels du dessin.
- **Le mauvais choix** : quand le dessin *est* la surface de jeu — des puzzles
  de poussée de blocs où la face du bloc doit rejoindre le mur exactement, ou
  des jeux de plateforme à la tuile près où un pixel fait la différence entre
  rester debout et tomber —, le retrait ment sur la géométrie, et le joueur
  apprend à se méfier de l'image. La collision doit être aussi indulgente que
  les verbes du jeu, et pas plus.

Une dernière habitude à adopter : le retrait appartient *à la boîte du mover*,
pas à la carte. Les requêtes restent exactes (le contrat de la leçon 055 est
inchangé) ; le jeu décide quel rectangle il accepte d'être.

*Page traduite de la version anglaise `book/solutions/lesson-056/ex1.md`,
révision `853ad02`.*

<!-- translation-source: book/solutions/lesson-056/ex1.md @ 853ad02 -->
