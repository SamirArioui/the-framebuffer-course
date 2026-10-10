# Solution : exercice 1 — Le premier changement de votre jeu

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — Le premier changement de votre jeu](../../lessons/part-5/lesson-103-your-game.md) de la leçon 103.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-103/ex1.patch}}
```

## Visite guidée

Un changement travaillé, dans la forme que l'énoncé exige : un fait en données,
une règle dans la couche jeu. Le jeu ici est *un peu plus rapide et un peu plus
gourmand* que celui du cours — sa première vague amène un **hound** (un chasseur
à presque deux fois la vitesse du bat, apparu à côté du héros), et sa règle est
**une mise à mort vaut 100 points**.

**Le changement de données** est une ligne dans `assets/enemies.txt` — le hound
est un type chase de la vague 1, `count 2`, réutilisant `assets/bat.ppm` comme
art jusqu'à ce que le jeu ait le sien. Aucun code : le chargeur, le magasin,
l'IA et les vagues savent déjà tous quoi faire d'une ligne. C'est la couture des
tables qui fonctionne comme prévu — un nouveau type est une nouvelle ligne.

**Le changement de règle** traverse les paires de fichiers de la couche jeu et
rien d'autre : le score vit dans l'état du jeu (`Game.score`, la valeur que le
HUD lit), la mise à mort est un événement à l'intérieur de `CombatFly`, et les
deux sont câblés à travers `GameWalk` — `double &score` transmis par les mêmes
listes de paramètres qui portent déjà `feel` et `sound`. L'incrément se trouve
dans la branche de vie nulle, à côté du retrait qu'il récompense. `main.cpp`
change d'un seul token — le site d'appel de la boucle passe `game.score`, ce qui
est la raison d'être de la boucle : c'est la racine de composition où les paires
de la couche jeu se rencontrent.

La preuve que l'énoncé demande, dans ses deux parties — la transcription de
l'exécution (héros immobile, tirant droit sur la vague qui charge) :

```
engine: def hound: x 448 y 232 facing 0 speed 192 health 2 sprite assets/bat.ppm … behavior chase wave 1 count 2
engine: wave 1: spawns hound at 448,232 — speed 192, health 2, chase
engine: wave 1: spawns hound at 464,232 — speed 192, health 2, chase
engine: hit: bolt hits hound — damage 1, health 2 -> 1
engine: hit: bolt hits hound — damage 1, health 1 -> 0
engine: hound retired — zero health
engine: hud: score 000100, health 1/3, time 0:00, wave 1/3 — at 8,8 over camera 8,0
…
engine: hound retired — zero health
engine: hud: score 000200, health 1/3, time 0:01, wave 1/3 — at 8,8 over camera 8,0
```

Chaque mise à mort est exactement un bond de 100 (le héros reste immobile, donc
l'ancien score fondé sur le déplacement n'ajoute rien à lire au travers), les
hounds meurent en deux traits chacun comme le dit la vie de leur ligne, et
`wave 1 cleared — the next begins` suit quand le dernier d'entre eux tombe. Et
la preuve de frontière :

```
 assets/enemies.txt |    1 +
 src/combat.cpp     |    9 ++++++++-
 src/combat.h       |    6 ++++--
 src/game.cpp       |    4 ++--
 src/game.h         |    5 +++--
 src/main.cpp       |    2 +-
 6 files changed, 19 insertions(+), 8 deletions(-)
```

Chaque fichier est une paire de la couche jeu, la racine de composition de la
boucle, ou `assets/` — les services (`arena.*`, `table.*`, `entity.*`,
`gametime.*`, `tilemap.*`, `audio.*`, `camera.*`, `framebuffer.*`)
n'apparaissent pas, `./build.sh` est sans avertissement, et
`./tools/check-boundary.sh` est propre. C'est le test de la passation : votre
changement a plié le jeu et jamais le moteur.

Trois notes pour votre propre premier changement :

1. **La règle de score est volontairement simple.** Une prime par type serait un
   meilleur game design et toucherait plutôt la couture des *tables* : une
   colonne nommée, ajoutée de façon additive (la règle de la leçon 087), pour
   que tout fichier jamais livré continue de se charger. Quand votre changement
   veut un nouveau fait sur les types, c'est la manière approuvée de grandir —
   et la règle de compatibilité est ce qu'il faut garder.
2. **Là où une règle traverse des fichiers est une information.** Le passage de
   `double &score` montre exactement où cette règle vit et qui la connaît. Si
   votre règle doit traverser quatre paires et un service, arrêtez : soit la
   règle est au mauvais endroit, soit c'est la couture. Les deux se corrigent —
   par vous, exprès.
3. **Gardez la démonstration.** Un changement sans ligne de transcription qui le
   prouve est une supposition. Les rapports sont déjà dans l'exécution ;
   utilisez-les avant d'en ajouter de nouveaux (et quand vous ajoutez des
   sondes, la discipline de la transcription de la leçon 097 les empêche de
   devenir la prochaine dette).

*Page traduite de la version anglaise `book/solutions/lesson-103/ex1.md`,
révision `cfeaecd`.*

<!-- translation-source: book/solutions/lesson-103/ex1.md @ cfeaecd -->
