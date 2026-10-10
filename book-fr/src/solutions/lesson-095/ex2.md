# Solution : exercice 2 — Le pool sous le feu

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — Le pool sous le feu](../../lessons/part-5/lesson-095-audio.md) de la leçon 095.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-095/ex2.patch}}
```

## Visite guidée

**La prédiction, écrite d'abord.** Le pool compte 16 canaux : le 0 est celui
de la musique (réservé, jamais volé) et les 1–15 ceux des effets. Chaque effet
prend le premier canal libre, donc vingt sons déclenchés dans une même frame
se déroulent ainsi :

1. Les **quinze premiers** prennent les canaux **1 à 15**, dans l'ordre.
2. Le **seizième** ne trouve aucun canal libre et vole le **plus ancien** —
   celui qui a commencé le plus tôt, soit le **canal 1** (il a commencé en
   premier, et le vol le redémarre avec l'ordre le plus récent).
3. Le **dix-septième** vole alors le nouveau plus ancien, le **canal 2**, et
   ainsi de suite : les cinq derniers des vingt atterrissent sur **1, 2, 3, 4,
   5**.

**L'exécution** — la sonde déclenchant vingt morts du jeu dans la première
frame du combat — correspond exactement à la prédiction :

```
engine: sound: death -> channel 1 (volume 128 of 256)
engine: sound: death -> channel 2 (volume 128 of 256)
engine: sound: death -> channel 3 (volume 128 of 256)
…
engine: sound: death -> channel 14 (volume 128 of 256)
engine: sound: death -> channel 15 (volume 128 of 256)
engine: sound: death -> channel 1 (volume 128 of 256)   <- the sixteenth: channel 1, stolen
engine: sound: death -> channel 2 (volume 128 of 256)   <- the seventeenth: channel 2
engine: sound: death -> channel 3 (volume 128 of 256)
engine: sound: death -> channel 4 (volume 128 of 256)
engine: sound: death -> channel 5 (volume 128 of 256)
```

`1 … 15`, puis `1, 2, 3, 4, 5` — le seizième vole le *plus ancien*, pas le
dernier, pas un canal au hasard. (Les `8, 9` qui suivent dans l'exécution sont
les événements du jeu lui-même poursuivant le combat après la frame de la
sonde.)

**À quoi sert la politique.** Un son qui est abandonné est *inaudible* — le
joueur entend de toute façon le moment le plus récent du combat — donc quand
le pool est plein, le bon son à perdre est celui que le joueur a déjà le plus
entendu : le plus ancien. C'est le miroir exact du magasin d'entités de la
leçon 074, et le contraste entre les deux est la leçon : un magasin d'entités
**ne vole jamais** (un ennemi volé est un bug que le joueur subit ; le magasin
refuse sous forme de valeur typée et le jeu décide quoi faire), tandis que le
mixeur **vole le plus ancien** (un son abandonné est inaudible, donc la
ressource continue de circuler). Les deux politiques sont celles du moteur,
toutes deux sont typées, et aucune n'est une surprise — et désormais les
événements du jeu sont assez bruyants pour les rencontrer.

*Page traduite de la version anglaise `book/solutions/lesson-095/ex2.md`,
révision `3e7cd54`.*

<!-- translation-source: book/solutions/lesson-095/ex2.md @ 3e7cd54 -->
