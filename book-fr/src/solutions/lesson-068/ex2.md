# Solution : exercice 2 — La musique qui ducke

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — La musique qui ducke](../../lessons/part-3/lesson-068-together.md) de la leçon 068.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-068/ex2.patch}}
```

## Visite guidée

Le diff rend le ducking réel sur un mixeur jetable : la musique en boucle sur
son canal, un effet déclenché par-dessus, et le volume du canal de la musique
abaissé pour le moment de l'effet puis remonté ensuite. Il n'y a aucun concept
nouveau de mixeur là-dedans. Un ducking est **un volume par son appliqué au
canal de la musique dans le temps** — le même champ `volume` que chaque canal
possède, la même échelle en virgule fixe, écrit et réécrit depuis le script de
l'exécution. `MixerStop`, les routes et le mixage ne sont pas touchés.

La forme d'un ducking : au déclenchement de l'effet, le volume tombe à la
moitié, tient là pendant dix tampons — le plancher — puis remonte sur dix de
plus, chaque pas une division en virgule fixe qui atterrit exactement sur
`AUDIO_VOLUME_FULL`. Le journal de l'exécution, une ligne par tampon :

```
engine: duck: buffer  0: music volume 128 of 256 — the drop
engine: duck: buffer  1: music volume 128 of 256
...
engine: duck: buffer  9: music volume 128 of 256
engine: duck: buffer 10: music volume 140 of 256 — the climb begins
engine: duck: buffer 11: music volume 153 of 256
engine: duck: buffer 12: music volume 166 of 256
engine: duck: buffer 13: music volume 179 of 256
engine: duck: buffer 14: music volume 192 of 256
engine: duck: buffer 15: music volume 204 of 256
engine: duck: buffer 16: music volume 217 of 256
engine: duck: buffer 17: music volume 230 of 256
engine: duck: buffer 18: music volume 243 of 256
engine: duck: buffer 19: music volume 256 of 256 — back where it was
engine: duck: buffer 20: music volume 256 of 256
engine: duck: the music's frame 1: 277 at full volume, 138 at the duck's floor
```

Le plancher est `AUDIO_VOLUME_FULL / 2` = 128 et la dernière ligne, c'est ce
que la chute veut dire en octets : la trame 1 de la musique, 277 à pleine
échelle, vaut 138 au plancher — le même échantillon, le même curseur, la moitié
de l'amplitude. Le mixage n'a besoin d'aucune explication pour tout ça : il
tire la trame suivante du canal et la met à l'échelle du volume du canal,
exactement comme il le fait quand rien n'est ducké.

Deux détails par lesquels cet exercice gagne sa place.

**La remontée doit atterrir sur le plein et y rester.** Le pas naïf — ajouter
le même incrément au-delà de la fin de la remontée — pousse le volume au-delà
d'`AUDIO_VOLUME_FULL` au tampon qui suit la remontée : un canal qui *amplifie*
son échantillon, brisant le contrat de volume (0 à `AUDIO_VOLUME_FULL`) qui
tient la promesse de la leçon 063 — un canal émet au plus l'échantillon qu'il
joue. Le dernier pas atterrit exactement sur 256 parce que le pas est calculé
comme une fraction de la distance restante, et la remontée s'arrête au tampon
où elle se termine.

**Le ducking, c'est le volume de l'effet, déplacé sur la musique.** Le propos
est l'audibilité : l'effet est ce qui vient de se passer et la musique est la
constante en dessous, donc pour le moment de l'effet la constante s'écarte. La
profondeur et la longueur du ducking sont du game design ; l'arithmétique est
l'échelle de volume que le cours utilise depuis la leçon 063. Rien dans le
mixage ne sait qu'un ducking est en cours.

Ce que la sonde ne peut pas trancher, c'est si dix tampons est la bonne
longueur ou la moitié la bonne profondeur — ça s'entend, ça ne se calcule pas.
Les volumes et les trames sont exacts ici ; le son du ducking est à vous
d'entendre.

*Page traduite de la version anglaise `book/solutions/lesson-068/ex2.md`,
révision `a6c470d`.*

<!-- translation-source: book/solutions/lesson-068/ex2.md @ a6c470d -->
