# Solution : exercice 1 — Des octets jusqu'au bout

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — Des octets jusqu'au bout](../../lessons/part-0/lesson-021-terminal-input.md) de la leçon 021.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-021/ex1.patch}}
```

## Visite guidée

Chaque octet imprimé en hexadécimal, les quatre exécutions confirment la
prédiction qu'un terminal n'envoie rien *que* des octets. `q` arrive comme
`0x71`, rien de plus. Une flèche fait trois octets — `0x1b 0x5b 0x41` est
ESC `[` `A`, haut — et le parseur les assemble en un changement de direction.
Un ESC solitaire laisse le parseur dans son état « après ESC » et aucune
direction ne change. `ESC [ A q` fait quatre octets dans une seule lecture :
direction haut, puis quitter.

Une prédiction aura été fausse sur la plupart des machines : exactement *sur
quelle* frame les octets atterrissent. Dans une exécution, `q` a quitté à
`done after 1 frames` ; avec la construction instrumentée plus lourde, le tube
l'a livré à la frame 2 — `done after 2 frames`. L'entrée est asynchrone : les
octets restent dans le tube jusqu'à ce que `poll` les voie, et la frame dont
`ProcessInput` les attrape relève de l'ordonnancement, pas de la logique.
C'est aussi pourquoi un seul `read` peut contenir plusieurs touches — la
boucle draine tout ce qui est arrivé en un appel, et chaque octet est analysé
dans l'ordre.

À travers un vrai terminal (`printf '\033[Aq' | script -qec './snek 30'
/dev/null`) les mêmes octets apparaissent, voyageant désormais par un pty, et
le programme se comporte identiquement.

*Page traduite de la version anglaise `book/solutions/lesson-021/ex1.md`,
révision `2369864`.*

<!-- translation-source: book/solutions/lesson-021/ex1.md @ 2369864 -->
