# Solution : exercice 3 — Monotone contre horloge murale

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 3 — Deux horloges](../../lessons/part-0/lesson-020-timing.md) de la leçon 020.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-020/ex3.patch}}
```

## Visite guidée

Les deux horloges répondent à des questions différentes. `CLOCK_REALTIME` est
le temps civil — des secondes depuis l'époque, ce que veulent un fichier de
journal ou l'horodatage d'une sauvegarde. `CLOCK_MONOTONIC` est le temps depuis
un point fixe (approximativement le démarrage), et le noyau garantit qu'il ne
va qu'en avant à un rythme régulier. L'exécution instrumentée montre les deux
sur cette machine : `mono=17568.9…` contre `wall=1791233614.…` — temps de
fonctionnement contre calendrier.

La garantie est la raison pour laquelle le jeu utilise la monotone. L'horloge
murale est *sautée* : une correction NTP, une réparation après double
démarrage, ou `date -s` peuvent la déplacer en avant ou en arrière pendant que
le jeu tourne. Un `dt` calculé depuis `CLOCK_REALTIME` rapporterait alors une
frame négative ou énorme, et l'accumulateur avalerait soit des ticks soit
ferait exploser le serpent à travers le mur. Les deltas monotones sont
toujours le vrai temps écoulé, aussi `dt` reste-t-il honnête et le pas fixe
stable — les deltas des deux horloges sont d'accord aujourd'hui, et une seule
promet de l'être demain.

*Page traduite de la version anglaise `book/solutions/lesson-020/ex3.md`,
révision `203c219`.*

<!-- translation-source: book/solutions/lesson-020/ex3.md @ 203c219 -->
