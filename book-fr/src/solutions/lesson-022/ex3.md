# Solution : exercice 3 — Ce que le différentiel économise

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 3 — Ce que le différentiel économise](../../lessons/part-0/lesson-022-double-buffer.md) de la leçon 022.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-022/ex3.patch}}
```

## Visite guidée

`SNEK_FULL=1` fait traiter chaque cellule comme changée par le vidage — la
même grille, redessinée en gros à chaque frame. Sur une machine,
`./snek 60 2>/dev/null | wc -c` rapportait 6 949 octets avec le différentiel
et 396 007 octets avec le redessin complet : 57× plus de trafic pour la même
image. À 120 frames l'écart grandit — 7 289 contre 792 007 octets, 109× —
parce que le coût du différentiel est presque constant (un dessin complet,
puis un filet de cellules changées) tandis que le coût du redessin complet est
linéaire en frames. Vos ratios différeront avec le nombre de frames ; la
forme, non.

Deux coûts se cachent dans ces nombres. La bande passante est l'évident : le
redessin complet pousse environ 6 600 octets de séquences d'échappement par
frame, ce qui va bien par un pty et fait mal par SSH. Le plus subtil est le
*scintillement* : une réécriture plein écran montre brièvement des frames à
moitié dessinées, et l'œil lit cela comme du miroitement — la même raison pour
laquelle les terminaux de l'ère du film et les consoles de jeu mettent en
double tampon. Le vidage différentiel n'efface jamais ce qui est déjà correct,
aussi seuls de vrais changements sont-ils jamais visibles en mouvement.
`getenv` est le plus petit interrupteur possible : une comparaison par vidage,
aucune plomberie de ligne de commande.

*Page traduite de la version anglaise `book/solutions/lesson-022/ex3.md`,
révision `eea0461`.*

<!-- translation-source: book/solutions/lesson-022/ex3.md @ eea0461 -->
