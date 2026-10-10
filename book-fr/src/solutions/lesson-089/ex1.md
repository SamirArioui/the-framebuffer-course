# Solution : exercice 1 — La distance du gardien est de la donnée

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — La distance du gardien est de la donnée](../../lessons/part-5/lesson-089-enemy-ai.md) de la leçon 089.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-089/ex1.patch}}
```

## Visite guidée

La distance devient une colonne nommée — `keep` — grandi exactement comme la
leçon 087 a grandi le format : la colonne existe, le chargeur lui donne son
défaut (`TABLE_KEEP_DEFAULT`, le 160 que le comportement codait en dur), et
chaque fichier décide de la nommer ou non. `assets/enemies.txt` la nomme, donc
chaque ligne de l'effectif déclare sa propre distance ; `assets/entities.txt`
reste silencieux et se charge octet pour octet, ses lignes portant le défaut :

```
engine: def hero: x 312 y 232 facing 0 speed 240 health 3 sprite assets/hero.ppm accel 120 damage 0 rate 0 fires none range 0 behavior none keep 160 wave 0 count 1
engine: def spitter: x 120 y 400 facing 0 speed 96 health 3 sprite assets/spitter.ppm accel 120 damage 1 rate 30 fires shell range 0 behavior keep keep 120 wave 2 count 2
engine: def warden: x 240 y 320 facing 0 speed 72 health 6 sprite assets/golem.ppm accel 120 damage 1 rate 30 fires shell range 0 behavior keep keep 240 wave 3 count 1
```

(Une nouvelle ligne `warden` rejoint l'effectif — un second gardien, pour que
l'exécution en ait deux à deux distances. Les valeurs `keep` des lignes se
tiennent à côté de leur `behavior keep`, ce qui se lit un peu bizarrement et
est honnête : l'une nomme ce qu'il fait, l'autre dit à quelle distance.)
`AiKeep` lit maintenant le nombre propre de l'entité au lieu d'`AI_KEEP`, et
le bilan de clôture montre les deux gardiens chacun à sa propre distance :

```
engine: world: spitter ends at 221,298 — 112 px of the hero
engine: world: warden ends at 156,403 — 232 px of the hero
```

112 px d'un 120 (le bord intérieur de la bande), 232 px d'un 240 — même
comportement, nombres différents, aucun code entre eux.

**Ce que le défaut achète à un fichier silencieux.** Exactement ce que la leçon
087 promettait : `assets/entities.txt` ne nomme aucune colonne `keep` et se
charge quand même, octet pour octet, et ses lignes portent le défaut du
format — visible directement dans le rapport (`keep 160` sur la ligne du
héros). Un fichier ne déclare que là où il est en désaccord avec le format ;
les défauts du format sont le réglage partagé. Changez `TABLE_KEEP_DEFAULT` et
chaque ligne silencieuse bouge avec lui, sans qu'un seul fichier soit édité.

*Page traduite de la version anglaise `book/solutions/lesson-089/ex1.md`,
révision `d371511`.*

<!-- translation-source: book/solutions/lesson-089/ex1.md @ d371511 -->
