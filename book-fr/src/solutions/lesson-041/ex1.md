# Solution : exercice 1 — La traînée du marqueur

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — La traînée du marqueur](../../lessons/part-1/lesson-041-arenas.md) de la leçon 041.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-041/ex1.patch}}
```

## Visite guidée

La traînée, ce sont huit positions — les huit dernières demeures du marqueur —
et l'endroit naturel pour les garder est l'allocateur de cette leçon. L'arena
est vidée de l'expérience de démarrage d'un seul `ArenaRollback(arena, 0)`, et
la traînée prend sa part :

```
engine: trail: 8 slots at offset 0 (arena used 64)
engine: marker at 548,228 (t=1.002)
engine: marker at 554,228 (t=1.053)
engine: marker at 560,228 (t=1.103)
engine: marker at 566,228 (t=1.154)
engine: trail used 64 bytes of its arena mark 0
```

Huit `TrailPos` de deux ints chacun : 64 octets à l'offset 0 — remarquez le
retour arrière à l'œuvre dans le rapport de l'allocateur lui-même : la traînée
a atterri là où avait été la première allocation de l'expérience de démarrage,
parce que la mémoire de l'expérience est revenue au bump pointer et que l'arena
n'oublie jamais que tout cela n'était qu'une seule réservation.

Chaque déplacement écrit l'emplacement suivant et boucle (`trail_at % 8`) — un
anneau de huit — et le rendu dessine les emplacements plus anciens plus ternes
et plus petits derrière le marqueur. La traînée est dessinée *avant* le marqueur
à chaque frame, donc le marqueur peint toujours par-dessus son propre
historique — la règle d'ordre de dessin de la leçon 025.

Une note de conception pour la rédaction : le retournement de l'anneau n'est
*pas* un retour arrière — l'historique garde ses huit emplacements vivants à la
fois. Le retour arrière sert à « tout ce qui suit ce point est terminé » (les
données temporaires d'une frame, les données d'un niveau) ; un anneau sert à
« réutiliser le plus ancien emplacement ». Savoir quelle forme est la vôtre,
c'est tout l'art de l'usage d'une arena.

*Page traduite de la version anglaise `book/solutions/lesson-041/ex1.md`,
révision `3f18b81`.*

<!-- translation-source: book/solutions/lesson-041/ex1.md @ 3f18b81 -->
