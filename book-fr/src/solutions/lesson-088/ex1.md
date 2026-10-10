# Solution : exercice 1 — Les types que personne n'a encore faits

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — Les types que personne n'a encore faits](../../lessons/part-5/lesson-088-enemy-tables.md) de la leçon 088.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-088/ex1.patch}}
```

## Visite guidée

Trois types, trois lignes — le diff est `assets/enemies.txt` et rien d'autre :

```
swarm 240 160 0 140 1 assets/wisp.ppm 1 90 bolt chase 2 6
sprinter 240 200 0 240 1 assets/bat.ppm 1 60 bolt chase 1 3
tank 600 440 0 48 12 assets/golem.ppm 3 20 shell keep 3 1
```

(L'art est réutilisé exprès — la nuée porte les couleurs du wisp, le sprinteur
celles du bat. Un art nouveau est un nouveau `.ppm`, aussi de la donnée, aussi
sans code.) L'exécution fait apparaître chacun en portant les valeurs de sa
ligne :

```
engine: entity swarm: x 240 y 160 facing 0 speed 140 health 1 sprite 16x16 accel 120 damage 1 rate 90 fires bolt range 0 behavior chase wave 2 count 6
engine: entity sprinter: x 240 y 200 facing 0 speed 240 health 1 sprite 16x16 accel 120 damage 1 rate 60 fires bolt range 0 behavior chase wave 1 count 3
engine: entity tank: x 600 y 440 facing 0 speed 48 health 12 sprite 16x16 accel 120 damage 3 rate 20 fires shell range 0 behavior keep wave 3 count 1
engine: roster: 7 enemies from the table's rows, live 9 of 64
```

et `git diff --stat src/` n'imprime rien du tout — l'arbre source n'a pas
bougé.

Ce que les colonnes ont exprimé que du code aurait caché : la nuée est six fois
la même chose (`count 6`), le sprinteur est vitesse 240 avec un point de vie —
tout le compromis « rapide et fragile » écrit là où un designer peut le lire —
et le tank, c'est douze points de vie, trois dégâts, vitesse 48 : une autre
*position dans l'espace de conception*, et la seule différence entre lui et le
sprinteur est quatorze octets de texte dans un fichier. En code, ce seraient
des constantes que personne ne trouve, trois chaînes de `if`, et une
discussion en revue pour savoir dans quel fichier la vitesse du boss doit
vivre. Ici, c'est une table qu'on peut passer au diff.

*Page traduite de la version anglaise `book/solutions/lesson-088/ex1.md`,
révision `ac121ed`.*

<!-- translation-source: book/solutions/lesson-088/ex1.md @ ac121ed -->
