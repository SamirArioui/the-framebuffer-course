# Solution : exercice 2 — La table d'acceptation

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — La table d'acceptation](../../lessons/part-1/lesson-043-demo.md) de la leçon 043.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-043/ex2.patch}}
```

## Visite guidée

« La couche plateforme terminée » est une affirmation, et une affirmation veut
une table. La première colonne de l'instrument est le contrat en usage, nommé
au démarrage :

```
engine: platform layer done — window, polled input, arena-backed framebuffer, measured frames
engine: contract: window + presentation (lessons 027-031)
engine: contract: polled input (lessons 032-034)
engine: contract: monotonic clock (lessons 035-036)
engine: contract: whole-file I/O (lessons 037-038)
engine: contract: reservations + arena (lessons 040-041)
```

Le reste de la table est à vous, à remplir à partir des exigences de la
spécification et de vos propres exécutions. La forme à viser :

| Exigence de la spécification | Scénario | Preuves tirées de l'exécution |
| ---------------------------- | -------- | ----------------------------- |
| Fenêtre et présentation | la fenêtre s'ouvre à la taille demandée | la fenêtre fait 640×480 (`xdotool getwindowgeometry`) |
| | les pixels présentés sont inchangés | les pixels du marqueur relus à la position rapportée |
| | la fermeture est signalée, les ressources libérées | `close reported`, `closed`, fenêtre disparue après la sortie |
| Entrée scrutée | la scrutation rapporte l'état courant | le marqueur ne bouge que pendant que les touches sont enfoncées |
| | les touches maintenues restent maintenues | un seul maintien, plusieurs pas |
| | les appuis brefs ne sont pas perdus | le rapport de mémorisation de la leçon 033 |
| Chronométrage monotone des frames | les lectures ne reculent jamais | la vérification de démarrage sur 100 000 échantillons |
| | les durées de frame sont résolubles | les chiffres en millisecondes du journal de frames |
| E/S de fichiers entiers | lecture réussie / l'échec est une valeur | les exécutions de la leçon 037 |
| | l'écriture fait un aller-retour | le `round-trip ok` de la leçon 038 |
| Frontière unique | le code du moteur est sans OS | `check-boundary.sh` qui passe |
| | un second OS se branche | la ligne de liaison du stub (l'exercice 1 de la leçon 042) |

Chaque ligne a une exécution derrière elle — c'est ce qui fait de la table une
table d'acceptation et non une liste de souhaits. Là où une ligne manque de
preuves, la démo n'est pas terminée : trouvez l'exécution qui tranche.

La dernière question de la rédaction est celle qui vaut la peine d'être
retenue : quelles lignes casseraient en premier si le moteur changeait ? La
ligne de la présentation casse quand le moteur de rendu change le format de
pixel ; la ligne de la frontière casse la première fois que quelqu'un inclut un
en-tête d'OS dans `main.cpp` ; la ligne du chronométrage casse quand les passes
d'optimisation de la partie 5 commencent à déplacer les coûts de frame. La
table n'est pas de la paperasse — c'est la liste des choses qui doivent rester
vraies pendant que le moteur grandit.

*Page traduite de la version anglaise `book/solutions/lesson-043/ex2.md`,
révision `5ec1553`.*

<!-- translation-source: book/solutions/lesson-043/ex2.md @ 5ec1553 -->
