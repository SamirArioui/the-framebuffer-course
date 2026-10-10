# Solution : exercice 1 — Les huit premières trames

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — Les huit premières trames](../../lessons/part-3/lesson-059-samples.md) de la leçon 059.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-059/ex1.patch}}
```

## Visite guidée

Le patch élargit la sonde de quatre trames à huit et ajoute une recherche sur le
tampon : l'échantillon le plus fort en valeur absolue, et le numéro de la trame
où il tombe pour la première fois. L'exécution imprime alors :

```
engine: tone: 22050 frames at 44100 Hz, first frames: 0 513 1024 1531 2032 2525 3009 3480
engine: tone: peak 8191 at frame 25
engine: tone played
```

Réconcilions la prédiction — la trame `i` est la mise à l'échelle de
`GenerateTone` écrite en toutes lettres,
`(short)(sin(2π · 440 · i / 44100) * 0.25 * 32767)` :

- **La trame 0 vaut exactement 0** — la sinusoïde part du repos, aucune
  arithmétique n'est nécessaire. Ce premier zéro est la position de repos du
  format, et c'est pourquoi une tonalité qui commence au début de son tampon
  commence en silence.
- **Les trames 1-3** (513, 1024, 1531) étaient sur la page pour servir de
  contrôle, et elles sont l'arithmétique : un pas de phase vaut 440/44100 de tour
  (environ 0,0627 radian), et près de zéro la sinusoïde est presque son propre
  argument — d'où des pas d'environ 512 unités au début.
- **Les trames 4-7** (2032, 2525, 3009, 3480) sont la prédiction que le contrôle
  récompense. Les pas ne cessent de rétrécir — 501, 493, 484, 471 — à mesure que
  l'onde se courbe vers sa crête. Si votre prédiction est tombée une unité trop
  haut (2033, 2526, 3481 …), c'est la conversion : `(short)` tronque vers zéro,
  il n'arrondit pas. La trame stockée est toujours le produit exact, sa fraction
  coupée.
- **Le pic tombe à la trame 25.** Le quart de période d'une onde à 440 Hz vaut
  `44100 / (4 * 440)` = 25,06 trames, donc la première crête tombe entre les
  trames 25 et 26 — et la trame 25 échantillonne 0,999994 de cette crête
  (8191,7 unités), que la conversion tronque à 8191, tandis que la trame 26 est
  déjà retombée de l'autre côté. La valeur est celle que la prose a dérivée :
  l'amplitude 0,25 place la crête à `0.25 * 32767` = 8191,75 unités, et aucune
  trame ne peut stocker au-delà de 8191. Le numéro de trame est le quart de
  période arrondi vers le bas — `25 / 44100` = 0,567 ms dans la tonalité.

Les creux atteignent −8191 aux mêmes amplitudes un demi-tour plus tard (la
conversion tronque vers zéro des deux côtés de l'onde, donc le −32768 du format
n'apparaît jamais à cette amplitude). La recherche porte sur la valeur absolue,
donc elle rapporte 8191 quel que soit le côté trouvé en premier — et la crête
positive est la première.

Rien de tout cela n'a besoin d'un périphérique : la sonde imprime avant
`SubmitSamples`, ce qui est tout son intérêt — les nombres se vérifient avant de
s'entendre. Et le fait de retournement de la prose referme l'arithmétique : la
trame 22050, une au-delà de la fin du tampon, serait de nouveau exactement la
trame 0 — 220 cycles entiers en 22050 trames. La crête se répète toutes les
100,23 trames jusqu'à ce bord.

*Page traduite de la version anglaise `book/solutions/lesson-059/ex1.md`,
révision `8c2591b`.*

<!-- translation-source: book/solutions/lesson-059/ex1.md @ 8c2591b -->
