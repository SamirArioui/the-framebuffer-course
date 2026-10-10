# Solution : exercice 1 — La phase audio, mesurée

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — La phase audio, mesurée](../../lessons/part-3/lesson-068-together.md) de la leçon 068.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-068/ex1.patch}}
```

## Visite guidée

Le diff est une mesure, pas une fonctionnalité : deux mixeurs jetables — un
inactif, un aussi chargé que le pool le permet (la musique en boucle sur son
canal et tous les canaux d'effets qui jouent) — chacun mixé mille fois à
travers le `MixBuffer` du moteur, chronométré sur l'horloge de la plateforme,
la même horloge que celle de l'enregistrement de frame. Les canaux du mixeur
chargé bouclent tous pour qu'aucune mesure n'inclue accidentellement un canal
qui se termine : le chronométrage mesure le mixage, rien d'autre. Ce que les
canaux contiennent ne peut pas compter pour l'arithmétique — un tirage par
canal et par trame de sortie, quoi que le tirage renvoie — et les nombres
ci-dessous sont exactement là où cette affirmation se vérifie.

Le journal de l'exécution :

```
engine: cost: idle mixer, 1000 buffers of 735 frames x 16 channels: 20.558 ms total, 20.6 us per buffer
engine: cost: busiest mix, 1000 buffers of 735 frames x 16 channels: 37.143 ms total, 37.1 us per buffer, 3 ns per channel frame
```

Réconciliez la ligne du mixeur chargé à la main. Un tampon fait 735 trames de
sortie, chacune tirée depuis tous les `AUDIO_MIXER_CHANNELS` = 16 canaux —
11 760 trames de canal par tampon. 37.1 µs par tampon sur 11 760 trames de
canal, c'est environ 3,1 ns chacune, le 3 imprimé. Par trame *de sortie*, le
coût est 37.1 µs / 735 ≈ 50 ns — seize tirages, un écrêtage, un `short` écrit.

La ligne du mixeur inactif est la trouvaille. Un mixeur inactif est moins cher
mais pas gratuit : le parcours tourne quand même, 16 canaux × 735 trames, et
chaque canal répond zéro — 20.6 µs par tampon, environ 28 ns par trame de
sortie. Le mixage paie pour son **nombre de canaux**, pas pour son volume.
C'est la forme du coût d'un pool fixe : décidé à l'avance, payé à chaque
tampon, minuscule.

Maintenant l'enregistrement de frame, qui porte la phase `audio` depuis la
leçon 060. De l'exécution de cette leçon — la musique en boucle, les effets qui
tirent, un tampon alimenté par frame — les lignes du journal sont :

```
frame 1: update 0.001 ms, audio 0.033 ms, render 1.632 ms (sprites 0.001, text 0.006, tilemap 0.936), present 0.461 ms, total 2.127 ms
frame 2: update 0.001 ms, audio 0.034 ms, render 1.368 ms (sprites 0.001, text 0.008, tilemap 0.945), present 0.598 ms, total 2.001 ms
frame 4: update 0.001 ms, audio 0.072 ms, render 1.742 ms (sprites 0.001, text 0.007, tilemap 0.970), present 0.768 ms, total 2.583 ms
```

Sur les 702 frames de l'exécution, la phase fait en moyenne 0.036 ms, avec un
plancher à 0.025 ms et un pire cas à 0.331 ms. Deux faits à rapporter avec ça.
D'abord, dans cette exécution **chaque** frame a alimenté un tampon — l'attente
cadencée réveille la boucle à l'horizon du tampon, donc une frame qui n'aurait
rien alimenté ne s'est pas produite ; une frame qui ne fait aucun travail audio
montrerait le plancher de la phase : essentiellement les deux lectures d'horloge
autour d'un `if`. Ensuite, la phase enveloppe le mixage *et* sa soumission, et
sur le périphérique `null` la soumission revient immédiatement — donc la phase
se lit comme le coût du mixage, et 0.036 ms est le 37.1 µs de la sonde vu de
l'intérieur de la frame.

La question du budget se répond d'elle-même à partir de là : 0.036 ms sur la
frame moyenne de 2.082 ms, c'est environ 1,7 % — le mixage est invisible dans
le budget à 60 frames par seconde. Pour le rendre visible, il faudrait
beaucoup plus de canaux (le coût est linéaire en leur nombre) ou une machine à
peu près deux ordres de grandeur plus lente. Sur du vrai matériel, un terme de
plus rejoint la phase : une soumission peut attendre de la place dans le tampon
du périphérique, exactement comme `present` inclut la synchronisation de la
copie — cette partie-là n'est pas à cette machine de la mesurer.

Ce que cette mesure ne peut pas vous dire, c'est ce que les octets mixés
donnent à l'oreille. Les chronométrages sont exacts ici ; l'écoute est à vous.

*Page traduite de la version anglaise `book/solutions/lesson-068/ex1.md`,
révision `a6c470d`.*

<!-- translation-source: book/solutions/lesson-068/ex1.md @ a6c470d -->
