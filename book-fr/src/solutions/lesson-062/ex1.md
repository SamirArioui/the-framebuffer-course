# Solution : exercice 1 — L'échantillon qui ne fait pas trente tampons

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — L'échantillon qui ne fait pas trente tampons](../../lessons/part-3/lesson-062-playback.md) de la leçon 062.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-062/ex1.patch}}
```

## Visite guidée

D'abord le fichier : `assets/tone.wav` coupé à 44 + 2000 octets — 1000 trames
de la même tonalité — avec le champ de taille `data` réglé à 2000 et la taille
RIFF à 36 + 2000, si bien que les deux déclarations du conteneur suivent les
octets et que le fichier est bien formé mais court. (Ce sont les retouches d'octets de la
leçon 061 : une déclaration mensongère fait un fichier malformé ; un fichier
court honnête n'est qu'un échantillon court.)

La prédiction, avant toute exécution. `CHUNK_FRAMES` vaut 735 et le remplissage
prend `min(frame_count - cursor, CHUNK_FRAMES)` :

| alimentation | de l'échantillon | de silence | où |
| --- | --- | --- | --- |
| 1 | 735 | 0 | trames 0–734 de l'échantillon |
| 2 | 265 | 470 | trames 735–999, puis des zéros — le tampon de queue |
| 3 … | 0 | 735 | du silence, chaque tampon après |

L'alimentation 2 est celle où `sample_cursor` atteint `frame_count`, donc la
ligne de fin doit dire **1000 frames fed in 2 buffers** — le nombre de trames de
l'échantillon et le nombre de tampons qui en ont porté quoi que ce soit. Le
total pour le périphérique est de 1000 trames d'échantillons : exactement
`frame_count`, dont 470 trames du deuxième tampon déjà du silence.

Le patch est l'instrument : une ligne par alimentation, nommant combien de
trames viennent de l'échantillon et combien du silence. Lancé contre le fichier court,
le journal dit :

```
engine: feed: 735 frames from the sample, 0 of silence
engine: feed: 265 frames from the sample, 470 of silence
engine: sample: 1000 frames fed in 2 buffers — the sample's end; the stream is silence from here
engine: feed: 0 frames from the sample, 735 of silence
engine: feed: 0 frames from the sample, 735 of silence
...
```

Chaque nombre prédit se réconcilie : 265, c'est 1000 − 735 ; 470, c'est 735 −
265 ; la ligne de fin atterrit avec la trame de l'alimentation 2, et les 174
alimentations qui ont suivi dans l'exécution de trois secondes sont toutes du
silence pur. Rien dans la cadence n'a bougé non plus — la barrière s'est encore
fermée sur les frames réveillées tôt par une nouvelle de la fenêtre (elles ont
consigné `audio 0.000 ms`), et l'horizon fait toujours 16,7 ms.

Maintenant la question à laquelle le tampon de queue répond. Une alimentation
d'une ligne —

```cpp
platform::SubmitSamples(audio.output, sample.frames + sample_cursor, CHUNK_FRAMES);
```

— remet au périphérique 735 trames à partir de la trame 735, et seules 265
d'entre elles sont celles de l'échantillon. Les 470 autres sont tout ce que
l'arena contient après les trames de l'échantillon — les octets d'un autre
asset, le contenu précédent de la réservation — **joués comme du son**, et
l'exécution appellerait alors cela « l'échantillon ». Le canal jouerait plus que
le nombre de trames de l'échantillon et lirait mal la mémoire pour le faire. Le
remplissage de queue est ce qui rend la phrase de la spécification vraie pour
tous les fichiers, et pas seulement pour les longueurs qui se divisent par
chance : 22050 / 735 = 30 exactement, c'est l'arithmétique de *cet asset*, et
celle de 1000, c'est 735 + 265 + silence. `frame_count` dit quand ; le
remplissage dépense exactement ce nombre de trames, et pas une de plus.

*Page traduite de la version anglaise `book/solutions/lesson-062/ex1.md`,
révision `fb99fc9`.*

<!-- translation-source: book/solutions/lesson-062/ex1.md @ fb99fc9 -->
