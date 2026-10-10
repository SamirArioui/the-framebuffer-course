# Solution : exercice 1 — L'horizon, doublé

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — L'horizon, doublé](../../lessons/part-3/lesson-060-stream.md) de la leçon 060.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-060/ex1.patch}}
```

## Visite guidée

Le patch élargit le tampon qu'une alimentation remet au périphérique :
`CHUNK_FRAMES` devient `AUDIO_RATE / 30` — 1470 trames, un trentième de seconde —
et le commentaire à côté est redérivé plutôt que laissé périmé, parce que la
divisibilité du curseur est un fait qui porte sur *les deux* constantes. Le
rapport de démarrage suit la constante sans autre travail :

```
engine: stream: 1470-frame buffers, horizon 33.3 ms; the loop feeds one when it is due
```

Mesures, exécutions de même durée de trois secondes contre le périphérique
`null`, avant et après (les comptes de frames varient de un d'une exécution à
l'autre ; la cadence, non) :

|                          | un soixantième                          | un trentième                            |
| ------------------------ | --------------------------------------- | --------------------------------------- |
| frames en 2,985 s        | 176 (frames 1–176)                      | 89 (frames 1–89)                        |
| cadence                  | 59,0 frames/s                           | 29,8 frames/s                           |
| écart médian             | 17 ms                                   | 34 ms                                   |
| horizon                  | 16,7 ms                                 | 33,3 ms                                 |
| `audio` par frame d'alimentation | 0,004–0,038 ms (171 sur 176 en 0,004–0,007) | 0,007–0,069 ms (83 sur 89 en 0,007–0,010) |
| alimentations            | 176                                     | 89                                      |

Réconcilions les deux lignes sur lesquelles porte l'exercice :

- **La cadence est celle de l'horizon.** Doubler le tampon divise par deux la
  fréquence des alimentations : 735 trames à 44100 Hz font 16,7 ms et 1470
  trames en font 33,3, et les écarts mesurés (17 ms et 34 ms) tombent juste
  après chacun — l'arrondi supérieur à la milliseconde dans le délai d'attente
  de `PumpEvents`, le même arrondi que nomme la prose de la leçon. Chaque réveil
  arrive avec le tampon déjà dû, l'alimentation se fait aussitôt, et l'échéance
  suivante est à un horizon de là. Le compte de frames est de l'arithmétique :
  2,985 s d'horizons.
- **Une alimentation coûte plus cher, les alimentations sont plus rares.** La
  phase `audio` double à peu près par alimentation — 0,005 ms typiquement à un
  soixantième, 0,008–0,009 à un trentième : deux fois plus de trames à travers
  la copie intermédiaire et jusqu'au périphérique — tandis que le nombre
  d'alimentations est divisé par deux. Le travail audio total sur l'exécution
  est constant, et c'est là la partie intéressante : 0,90 ms sur 176
  alimentations contre 0,91 ms sur 89. Chaque alimentation coûte environ deux
  fois plus (la médiane passe de 0,005 ms à 0,008 ms) et il y en a moitié moins,
  donc l'exécution paie la même chose pour le même flux. Les frames qui n'ont
  rien alimenté lisent toujours `audio 0.000 ms` — la barrière est intacte, donc
  une frame réveillée par une nouvelle, à l'un ou l'autre horizon, ne met rien
  en file (l'exécution à entrées de la prose de la leçon en montre
  quarante-trois).

Et le contrôle du retournement : `TONE_FRAMES / CHUNK_FRAMES` vaut maintenant
22050 / 1470 = **15 exactement** — toujours un nombre entier de tampons par
tonalité, donc le curseur se retourne toujours sur une frontière de tampon et
aucune alimentation ne se scinde sur la fin de la tonalité. La règle générale
tombe des constantes : avec `CHUNK_FRAMES = AUDIO_RATE / k` et
`TONE_FRAMES = AUDIO_RATE / 2`, le rapport vaut `k / 2` — un nombre entier chaque
fois que `k` est pair, et voilà pourquoi 30 et 60 fonctionnent tous les deux.

Choisissez un `k` impair et regardez-le casser : c'est la mise en garde qui vaut
une expérience. `AUDIO_RATE / 7` vaut 6300 trames, et 22050 / 6300 = 3,5. Les
décalages du curseur cessent de tomber sur des frontières de tampon — une
alimentation aurait besoin d'une copie scindée, une partie de la queue de la
tonalité et une partie de sa tête — et la simple arithmétique du curseur fait
pire encore : un départ près de la fin du tampon remet à `SubmitSamples` une
plage qui court *au-delà* de la dernière trame de la tonalité. Le modulo retourne
le début de la copie, pas la copie. Le commentaire de divisibilité n'est pas de
la décoration ; c'est la précondition de l'alimentation en une ligne.

Rien ici ne touche la couture, la barrière ou la comptabilité d'échéance — c'est
l'autre leçon de l'exercice : l'horizon est une constante, et l'attente cadencée
le suit sans aucun autre changement.

*Page traduite de la version anglaise `book/solutions/lesson-060/ex1.md`,
révision `264e818`.*

<!-- translation-source: book/solutions/lesson-060/ex1.md @ 264e818 -->
