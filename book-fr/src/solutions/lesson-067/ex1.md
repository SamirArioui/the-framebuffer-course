# Solution : exercice 1 — Un échantillon, deux volumes

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — Un échantillon, deux volumes](../../lessons/part-3/lesson-067-effects.md) de la leçon 067.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-067/ex1.patch}}
```

## Visite guidée

Le diff est une sonde : `assets/effect.wav` déclenché deux fois au même instant
via le `MixerPlayEffect` du moteur lui-même — à 192 et à 48 sur 256 — sur un
mixeur d'essai, avec le tampon mixé relu trame par trame et la fin des canaux
rapportée par les canaux eux-mêmes.

La prédiction, avant toute exécution. Les deux appuis parcourent le même pool
que ceux de la leçon, et le pool est neuf : le premier prend le canal 1 et le
second le canal 2. Chaque canal émet sa propre contribution de chaque trame —
la trame de l'échantillon fois son volume sur `AUDIO_VOLUME_FULL`, tronquée
vers zéro **avant** la somme, l'ordre de la leçon 064 — et le mixage additionne
les deux. Avec les premières trames de l'échantillon `0 1229 2438 3607 4719
5757 6703 7544`, les contributions et leurs sommes sont :

| trame | à 192 sur 256 | à 48 sur 256 | mixé |
| --- | --- | --- | --- |
| 0 | 0 | 0 | 0 |
| 1229 | 921 | 230 | 1151 |
| 2438 | 1828 | 457 | 2285 |
| 3607 | 2705 | 676 | 3381 |
| 4719 | 3539 | 884 | 4423 |
| 5757 | 4317 | 1079 | 5396 |
| 6703 | 5027 | 1256 | 6283 |
| 7544 | 5658 | 1414 | 7072 |

Lisez une ligne deux fois. La trame 1229 à 192 vaut 921,75 et à 48 vaut
230,4375 — et le canal tronque les deux avant d'additionner, donc le mixage dit
1151, pas 1152. L'ordre de la troncature et de la somme est visible dans les
trois nombres d'une seule trame.

Le reste de la prédiction est le contrat du one-shot lui-même. L'effet fait
8820 trames et les tampons 735 — douze tampons exactement — donc après treize
tampons de la paire, les deux canaux ont joué jusqu'à leur fin, sont devenus
inactifs et sont retournés au pool ; le treizième tampon contient du silence,
parce que le silence est ce que laissent derrière eux les canaux au repos.

Le journal de l'exécution :

```
engine: volumes: channel 1 at 192 of 256, channel 2 at 48 of 256
engine: volumes: first frames (summed): 0 1151 2285 3381 4423 5396 6283 7072
engine: volumes: buffer 13 is silence, and channels 1 and 2 are free again
```

Chaque trame de la ligne du milieu est la colonne de droite du tableau, au
chiffre près. Et les deux volumes sont ce qui en fait des **sons différents** :
le même échantillon, la même longueur, le même départ — et le mixage contient
deux flux de nombres différents parce que chaque effet porte son propre volume
dans son canal. C'est pourquoi `MixerPlayEffect` prend un volume, tout court :
pas pour l'équilibre au mixeur, mais parce que le son est l'échantillon *et*
son volume.

Une chose que la sonde ne peut pas montrer est la différence qu'un volume fait
à une oreille. Les octets prouvent que les flux diffèrent ; que l'un soit à
trois quarts d'échelle et l'autre à moins d'un cinquième *sonne* comme deux
choses est à vérifier de votre côté sur du matériel qui fait du son.

*Page traduite de la version anglaise `book/solutions/lesson-067/ex1.md`,
révision `b3b0598`.*

<!-- translation-source: book/solutions/lesson-067/ex1.md @ b3b0598 -->
