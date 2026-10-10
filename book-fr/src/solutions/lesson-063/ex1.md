# Solution : exercice 1 — Quart de volume, au chiffre près

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — Quart de volume, au chiffre près](../../lessons/part-3/lesson-063-channel.md) de la leçon 063.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-063/ex1.patch}}
```

## Visite guidée

Le diff fait deux choses : le volume de départ du canal — `AUDIO_VOLUME_FULL /
4`, soit 64 sur 256 — et la sonde, une ligne par remplissage, nommant ce que le
canal a produit et s'il joue encore après.

La prédiction, avant toute exécution. Au volume 64, la mise à l'échelle est
`(frame * 64) / 256` — la trame de l'échantillon divisée par quatre, tronquée
vers zéro :

| trame de l'échantillon | × 64 / 256 | émise |
| --- | --- | --- |
| 0 | 0 | 0 |
| 513 | 128,25 | 128 |
| 1024 | 256 | 256 |
| 1531 | 382,75 | 382 |
| 2032 | 508 | 508 |
| 2525 | 631,25 | 631 |
| 3009 | 752,25 | 752 |
| 3480 | 870 | 870 |

Puis les remplissages. `CHUNK_FRAMES` fait 735, et 22050 / 735 = 30
exactement :

| remplissage | trames d'échantillons | le canal après le remplissage | le tampon |
| --- | --- | --- | --- |
| 1–29 | 735 chacun | actif | les trames de l'échantillon au volume 64 |
| 30 | 735 | actif | les dernières trames de l'échantillon — le curseur atteint `frame_count`, et rien ne l'a encore regardé |
| 31 | 0 | inactif | tout en silence — le canal s'éteint à la première trame du remplissage |
| 32 … | 0 | inactif | du silence, chaque tampon après |

Le remplissage 31 est celui où la fin s'observe : sa **première** trame est la
première trame de sortie après la fin de l'échantillon, et cette trame écrit du
silence et rend le canal inactif — comme chaque trame après elle dans ce tampon.
Le rapport de fin atterrit avec la soumission de l'alimentation 31 et dit les
nombres propres de l'échantillon.

Le journal de l'exécution, vu par la sonde :

```
engine: channel: playing 22050 frames at volume 64 of 256
engine: channel: first frames at that volume: 0 128 256 382 508 631 752 870
engine: fill: 735 sample frames, channel active
...
engine: fill: 735 sample frames, channel active
engine: fill: 0 sample frames, channel inactive
engine: sample: 22050 frames fed in 30 buffers — the sample's end; the stream is silence from here
engine: fill: 0 sample frames, channel inactive
```

La première ligne de remplissage se répète trente fois, exactement comme prédit.
Chaque nombre se réconcilie : les trames émises correspondent au tableau ligne
pour ligne — 128 à partir de 128,25, 382 à partir de 382,75, la troncature par
deux fois — les trente remplissages actifs font 30 × 735 = 22050 =
`frame_count`, et le remplissage passé la fin ne porte rien d'autre que des
zéros. La prédiction facile à perdre est celle de la troisième ligne du tableau :
le remplissage qui dépense la dernière trame de l'échantillon revient avec le
canal *encore actif*. `cursor == frame_count` n'est pas encore la fin ; la fin,
c'est la première trame de sortie qui le constate.

Maintenant la question de frontière. Un échantillon de 1000 trames : le
remplissage 1 en dépense 735 et laisse le canal actif ; le remplissage 2 en
dépense 265 (trames 735–999) et à sa 266e trame — la première trame de sortie
après la fin — le canal redevient inactif, le reste de ce tampon en silence. Le
rapport dit alors `1000 frames fed in 2 buffers`. La fin tombe au milieu d'un
remplissage, et ni le remplissage, ni la sonde, ni le rapport n'ont eu besoin de
le savoir d'avance : `frame_count` dit quand, et le canal redevient inactif
exactement là.

*Page traduite de la version anglaise `book/solutions/lesson-063/ex1.md`,
révision `95a98ec`.*

<!-- translation-source: book/solutions/lesson-063/ex1.md @ 95a98ec -->
