# Solution : exercice 2 — Le sprite qui se retourne

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — Le sprite qui se retourne](../../lessons/part-2/lesson-046-movable-sprite.md) de la leçon 046.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-046/ex2.patch}}
```

## Visite guidée

Le patch remplace le bornage par un retournement (wrap) : quand le sprite a
voyagé d'une largeur entière de sprite au-delà d'un bord, il réapparaît juste en
dehors du bord opposé. Rien n'est dessiné tant qu'il est dehors — le découpage
du blit s'en charge gratuitement, et les frames de traversée dessinent la
portion encore à l'écran.

Une entrée scriptée a maintenu la flèche droite à travers le bord ; le rapport
montre le retournement se produire exactement à la frontière :

```
engine: sprite at 519,232 (t=2.673)
engine: sprite at 577,232 (t=2.915)
engine: sprite at 635,232 (t=3.156)
engine: sprite at 32,232 (t=3.398)
engine: sprite at 89,232 (t=3.638)
engine: sprite at 147,232 (t=3.879)
```

Depuis `635`, le sprite a continué à avancer vers la droite : la portion visible
a rétréci jusqu'à ce que le sprite soit entièrement dehors (`sprite_x` a
dépassé `640`, puis la règle `sprite_x < -sprite.width` l'a repositionné à
`FRAME_WIDTH`), et il est rentré par la gauche — `32`, puis `89`, puis `147` —
avec la même vitesse. La relecture à une position rapportée de l'autre côté
confirme que les pixels ont fait le voyage : en `176,232`, le pixel central du
sprite se lit `(220, 40, 40)` dans la fenêtre — le même art, de l'autre côté du
monde.

Notez les deux ou trois frames où le sprite n'est *nulle part à l'écran*. Se
retourner à `±sprite.width` signifie que le sprite doit voyager de sa propre
largeur au-delà du bord avant de réapparaître ; si vous vous retourniez
exactement au bord, le sprite bondirait de « un pixel visible » à « un pixel
visible » de l'autre côté. Les deux sont des politiques ; l'énoncé demandait ce
que le bornage achetait :

- **Le bornage garantit la présence.** L'objet est toujours entièrement à
  l'écran — chacun de ses pixels dessiné, chaque position lisible. C'est
  pourquoi les vérifications par relecture de la leçon pouvaient échantillonner
  en `(x + 8, y + 8)` sans réfléchir : avec le bornage, ce point est toujours un
  pixel du sprite.
- **Le retournement échange cette garantie contre la continuité du mouvement**
  — un monde dont les bords ne sont pas des murs. Le coût est exactement la
  propriété que le bornage achetait : le sprite peut être partiellement dessiné
  (de la comptabilité de deux objets si quoi que ce soit se base un jour sur
  « où est-il »), et il peut être entièrement hors écran au moment où une
  capture ou une vérification le chercherait.

Ni l'un ni l'autre n'a raison dans l'abstrait. Le bornage est la bonne politique
pour un objet que le joueur doit toujours voir (le héros de ce cours) ; le
retournement est la bonne politique pour un monde sans bords (un champ
d'étoiles, un bandeau défilant). Le code de la leçon 046 garde le bornage pour
exactement la première raison — et maintenant vous pouvez changer la politique
en un bloc de quatre lignes, parce que le découpage du blit rend les deux
politiques sûres.

*Page traduite de la version anglaise `book/solutions/lesson-046/ex2.md`,
révision `cfc6de7`.*

<!-- translation-source: book/solutions/lesson-046/ex2.md @ cfc6de7 -->
