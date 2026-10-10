# Solution : exercice 1 — Le dessin des sprites reçoit le même remède

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — Le dessin des sprites reçoit le même remède](../../lessons/part-5/lesson-099-map-draw.md) de la leçon 099.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-099/ex1.patch}}
```

## Visite guidée

`BlitSpriteFrame` est `BlitSprite` avec la colonne de frame de la planche
repliée dans l'adresse source, donc la correction est la même : des pointeurs de
ligne hissés (`src += 3`, `dst += 4` le long de la ligne, les deux bases
calculées une fois) et l'expansion directe pour une planche dont `key_count`
vaut zéro — une planche sans aucun pixel transparent n'en a certainement aucun
dans la frame qu'on en dessine. Le chemin de la couleur clé garde sa décision
par pixel, pour la même raison que celui de la jumelle : pour le héros et les
projectiles la transparence est réelle.

Puis la mesure que l'exercice exige — la ligne `sprites` sur les frames
chargées de cette machine (le même scénario de 1 551 frames, découpé en frames
de jeu ; les 50 frames où le plus de sprites ont été dessinés) :

```
                               twin untouched   twin fixed
  play frames, sprites avg       0.0063 ms      0.0047 ms
  busiest 50 frames              0.0202         0.0155
  worst single frame             0.164          0.150
```

La réponse honnête à « ce que cela a jamais valu » : **environ 25 % d'une ligne
qui fait 0,3 % de la frame.** Le remède fonctionne — les frames chargées
baissent d'un quart, la même forme de baisse que le dessin de la carte — et il
vaut `0.0016 ms` sur la frame de jeu moyenne. Si le jeu dessinait cinquante fois
plus de sprites (un bullet-hell sur ce moteur), ce serait le premier levier à
actionner. À l'échelle réelle du jeu, c'est une erreur d'arrondi.

Voilà la vraie leçon de l'exercice. Le menu a corrigé les deux points chauds
*mesurés* et a laissé cette boucle tranquille non par négligence mais parce que
la mesure disait que sa ligne est du bruit — et quand vous dépensez une
optimisation là où la mesure est silencieuse, vous gagnez de la complexité et
dépensez de la preuve. Ici la complexité était petite et la preuve est
maintenant à vous : la ligne a bougé, la frame non.

*Page traduite de la version anglaise `book/solutions/lesson-099/ex1.md`,
révision `20f49d8`.*

<!-- translation-source: book/solutions/lesson-099/ex1.md @ 20f49d8 -->
