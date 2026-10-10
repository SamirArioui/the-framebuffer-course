# Solution : exercice 2 — Pourquoi le temps mural

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — Pourquoi le temps mural](../../lessons/part-5/lesson-086-feedback-animation.md) de la leçon 086.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-086/ex2.patch}}
```

## Visite guidée

La sonde imprime le compte à rebours propre du hitstop en **secondes murales** à
côté du facteur de temps de jeu qu'il maintient — les deux horloges, côte à
côte.

```
engine: feel: hitstop 0.300 s of wall time left, scale factor 0.25
engine: feel: hitstop 0.200 s of wall time left, scale factor 0.25
...
engine: feel: hitstop rested — full speed again
```

Le compte à rebours descend en secondes réelles pendant que le facteur reste à
`0.25`. C'est le propos.

**Pourquoi pas le temps de jeu ?** Le travail d'un hitstop est précisément de
*changer* l'échelle du temps de jeu — à `0.25` ici. Si le compte à rebours du
hitstop tournait au temps de jeu, il serait multiplié par ce même `0.25` : le
compte à rebours avancerait au quart de la vitesse, donc un hitstop de 0,4
seconde prendrait 1,6 seconde réelle — et plus le ralentissement serait
dramatique, plus il durerait longtemps, ce qui est à l'envers. Pire, à une pause
*pleine* (échelle `0`), le temps de jeu ne bouge pas du tout, donc un compte à
rebours de hitstop au temps de jeu se figerait pour toujours : la seule chose qui
doit finir *pendant que le jeu est arrêté* ne finirait jamais. La leçon 078 a
tranché cette horloge — tout ce qui doit se terminer pendant que le jeu est
arrêté (un hitstop, un screenshake sous pause) tourne sur l'horloge murale.

**Que deviendrait un screenshake au temps de jeu ?** Le même piège : mettez le
jeu en pause au milieu d'une secousse et le temps de jeu s'arrête, donc la
secousse resterait suspendue pour toujours avec la caméra bloquée hors du centre
— le décalage ne reviendrait jamais à `0,0`. Sur le temps mural, elle continue
de décroître et se stabilise à exactement zéro même sous pause.

**Ce que prouvent les deux lignes de repos.** `hitstop rested — full speed
again` et `shake rested at 0,0` — les deux hooks ont fini *tout seuls* pendant
que le hitstop maintenait encore l'échelle à une fraction. Le hitstop s'est
terminé alors même que le temps de jeu était ralenti à `0.25` pendant toute sa
durée ; si son horloge avait été le temps de jeu, il n'aurait pas pu. Ces deux
lignes sont le choix du temps mural, mesuré : la vie des hooks est en secondes
réelles, indépendante de l'échelle même qu'ils règlent.

*Page traduite de la version anglaise `book/solutions/lesson-086/ex2.md`,
révision `4725cee`.*

<!-- translation-source: book/solutions/lesson-086/ex2.md @ 4725cee -->
