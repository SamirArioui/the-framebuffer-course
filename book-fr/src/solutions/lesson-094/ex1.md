# Solution : exercice 1 — Le score pour les mises à mort

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — Le score pour les mises à mort](../../lessons/part-5/lesson-094-hud.md) de la leçon 094.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-094/ex1.patch}}
```

## Visite guidée

Le format grandit exactement comme la leçon 087 l'a fait grandir — une fois,
par colonnes nommées, de façon additive. `points` rejoint la liste des
colonnes ; `EntityDef` et `Entity` le portent comme chaque autre valeur de
ligne ; la valeur par défaut (`TABLE_POINTS_DEFAULT = 0`) est le contrat du
format pour chaque fichier qui ne nomme jamais la colonne. Les lignes des
ennemis donnent un prix à leur type, et l'en-tête nomme la nouvelle colonne
comme le font toujours les en-têtes du format :

```
name x y facing speed health sprite damage rate fires behavior wave count points
bat 560 72 2 160 2 assets/bat.ppm 1 20 bolt chase 1 2 100
wisp 640 336 2 120 1 assets/wisp.ppm 1 20 bolt flee 2 1 150
spitter 120 400 0 96 3 assets/spitter.ppm 1 30 shell keep 2 2 250
golem 384 96 1 72 8 assets/golem.ppm 2 20 shell boss 3 1 1000
```

**La règle de compatibilité tient.** `assets/entities.txt` n'est pas touché —
et l'exécution montre chaque fichier livré se chargeant à la valeur par défaut
que son en-tête ne nomme jamais, à côté des lignes qui la nomment :

```
engine: table assets/entities.txt: 2 definitions
engine: def hero: x 312 y 232 facing 0 speed 240 health 3 sprite assets/hero.ppm accel 120 damage 0 rate 0 fires none range 0 behavior none wave 0 count 1 points 0
engine: def bat: x 560 y 72 facing 2 speed 160 health 2 sprite assets/bat.ppm … wave 1 count 2 points 100
engine: def wisp: x 640 y 336 facing 2 speed 120 health 1 sprite assets/wisp.ppm … wave 2 count 1 points 150
engine: def spitter: x 120 y 400 facing 0 speed 96 health 3 sprite assets/spitter.ppm … wave 2 count 2 points 250
engine: def golem: x 384 y 96 facing 1 speed 72 health 8 sprite assets/golem.ppm … wave 3 count 1 points 1000
```

`points 0` pour le héros, le slime et chaque ligne d'arme et de projectile —
les fichiers livrés dans la leçon 071 se chargent octet pour octet, leur champ
non nommé à la valeur par défaut du format. C'est toute la promesse du fait de
grandir par colonnes nommées : la donnée bouge, le chargeur non.

**La mise à mort gagne les points de sa ligne** dans la branche de mort du vol
— la même ligne où la mort retire l'entité — et le score est celui du jeu,
transmis au vol pour qu'il l'incrémente. La liste jetable (un type fragile à
500, ses propres nombres) montre le score qui bouge dans la frame de la mise à
mort elle-même :

```
engine: hit: bolt hits bag — damage 1, health 1 -> 0
engine: bag retired — zero health
engine: hud: score 000500, health 3/3, time 0:00, wave 1/3 — at 8,8 over camera 8,0
engine: hud: score 000500, health 3/3, time 0:00, wave 2/3 — at 8,8 over camera 8,0
engine: hit: bolt hits bag — damage 1, health 1 -> 0
engine: bag retired — zero health
engine: hud: score 001000, health 3/3, time 0:00, wave 2/3 — at 8,8 over camera 8,0
```

Une mise à mort, `+500` ; la seconde, `001000` — et l'indicateur qui le montre
est dans la frame du retrait, exactement comme la vie l'était dans celle du
coup. La part du score liée au terrain parcouru s'accumule toujours à côté :
`game.score` est un seul nombre, celui du jeu, et le HUD le lit de la même
façon quoi qu'il le remplisse.

Ce que l'exercice laisse délibérément de côté : rien sur les prix qui sont
*amusants*. `100/150/250/1000` est une estimation de tableur — ce que vaut une
chauve-souris à côté d'un golem relève de la conception de jeu, et tout
l'intérêt de la colonne est que son réglage est une modification d'un fichier
texte, jamais une recompilation.

*Page traduite de la version anglaise `book/solutions/lesson-094/ex1.md`,
révision `bf9b9df`.*

<!-- translation-source: book/solutions/lesson-094/ex1.md @ bf9b9df -->
