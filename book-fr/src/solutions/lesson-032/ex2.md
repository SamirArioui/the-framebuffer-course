# Solution : exercice 2 — L'appui qui a disparu

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — L'appui qui a disparu](../../lessons/part-1/lesson-032-polled-input.md) de la leçon 032.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-032/ex2.patch}}
```

## Visite guidée

La prédiction à faire d'abord : un geste — appui et relâchement sans attente —
produit deux événements, et le moteur scrute *après* que la pompe a vidé le
lot. La scrutation voit donc l'état tel qu'à la fin du lot : la touche est de
nouveau relâchée. L'appui et le relâchement s'annulent avant qu'aucune
scrutation n'ait lieu.

Avec le pliage instrumenté, les deux événements sont visibles — et leur
conséquence aussi :

```
$ DISPLAY=:99 xdotool key --delay 0 --window <id> space
platform: fold press   keysym=32
platform: fold release keysym=32
engine: presented
engine: polled: -
engine: polled: -
```

(`--delay 0` est ce qui met les deux événements dans un même lot de pompe ; le
délai par défaut laisse au moteur le temps de scruter entre les deux, et
l'appui apparaît alors normalement — c'est le timing qui décide de l'allure de
l'état, et c'est tout le problème.)

Les deux lignes de pliage prouvent que les événements sont arrivés ; les deux
scrutations `-` prouvent que le contrat les a perdus. L'état scruté répond
« qu'est-ce qui est enfoncé *maintenant* », et un appui déjà terminé n'est pas
enfoncé maintenant. Rien n'est cassé dans le pliage — les deux affectations
s'exécutent — l'*état* n'a simplement pas la bonne forme pour une nouvelle déjà
terminée.

Ce dont le moteur a besoin, c'est d'une seconde pièce d'état à côté de
l'actuelle : pas seulement « est enfoncée maintenant », mais « s'est enfoncée au
moins une fois depuis la dernière scrutation » — une mémorisation (latch) qui
tient jusqu'à ce que le moteur l'ait vue. C'est exactement ce que construit la
leçon 033, et cet appui qui disparaît est la raison de son existence.

*Page traduite de la version anglaise `book/solutions/lesson-032/ex2.md`, révision `a3b9017`.*

<!-- translation-source: book/solutions/lesson-032/ex2.md @ a3b9017 -->
