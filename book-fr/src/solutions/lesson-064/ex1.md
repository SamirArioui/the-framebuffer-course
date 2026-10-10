# Solution : exercice 1 — Cinq canaux à la crête

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — Cinq canaux à la crête](../../lessons/part-3/lesson-064-mix.md) de la leçon 064.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-064/ex1.patch}}
```

## Visite guidée

Le diff est deux choses : les canaux de l'exécution — cinq sur la tonalité à
`AUDIO_VOLUME_FULL`, au lieu de deux à 128 et 64 — et la sonde : tout
l'échantillon à travers le `MixBuffer` du moteur lui-même sur un mixeur d'essai,
trame par trame, chaque trame que l'écrêtage change nommée (les premières) et
comptée (toutes).

La prédiction, avant toute exécution. À plein volume, la contribution d'un canal
est exacte — `(frame * 256) / 256` est la trame elle-même — donc la somme vaut
**cinq fois la trame de l'échantillon**, et le seuil de l'écrêtage tombe de la
limite du format : 5v > 32767 écrête côté positif à partir de v ≥ 6554,
5v < −32768 écrête côté négatif à partir de v ≤ −6554. Tout ce qui est à 6553
ou moins en magnitude passe intact.

| prédiction | nombre |
| --- | --- |
| les huit premières trames mixées | `0 2565 5120 7655 10160 12625 15045 17400` |
| l'écrêtage les touche-t-il ? | non — la plus grande, 17400, est loin sous 32767 |
| la crête de la tonalité (trame 25, échantillon 8191) | somme 40955 → 32767 ; retournée −24581 |
| le creux de la tonalité (trame 75, échantillon −8191) | somme −40955 → −32768 ; retournée 24581 |
| la plus grande somme qui passe intacte | 32735 — la trame 6547 de la tonalité |
| la première trame que l'écrêtage change | trame 15 (échantillon 6616, somme 33080) |
| les trames changées en tout | 9040 sur 22050 |

Le seuil est 6553, mais la tonalité ne l'atteint jamais : 5 × 6553 fait 32765,
la plus grande somme qui *pourrait* passer, et la plus grande trame qui passe de
cette tonalité est 6547 — la sinusoïde saute de là directement à 6554, qui
écrête. Le compte exige le même parcours sur les données de l'échantillon :
toute trame à 6554 ou plus en magnitude écrête, et une demi-seconde de 440 Hz
en contient beaucoup.

Le journal de l'exécution :

```
engine: sample: 22050 frames at 44100 Hz, 1 channel, first frames: 0 513 1024 1531 2032 2525 3009 3480
engine: mix: five channels at volume 256 of 256
engine: mix: first frames (summed): 0 2565 5120 7655 10160 12625 15045 17400
engine: clamp: frame 15: sum 33080 -> out 32767, wrapped -32456
engine: clamp: frame 16: sum 34530 -> out 32767, wrapped -31006
engine: clamp: frame 17: sum 35840 -> out 32767, wrapped -29696
engine: clamp: frame 18: sum 37015 -> out 32767, wrapped -28521
engine: clamp: frame 19: sum 38040 -> out 32767, wrapped -27496
engine: clamp: frame 20: sum 38915 -> out 32767, wrapped -26621
engine: clamp: 9040 of 22050 frames clamped
engine: sample: 22050 frames fed in 30 buffers — the sample's end; the stream is silence from here
```

Chaque nombre se réconcilie. Les huit premiers sont cinq fois les trames de
l'échantillon, chiffre pour chiffre — 513 × 5 = 2565, 3480 × 5 = 17400 — et
aucun d'eux n'écrête. La trame 15 est la première trame de la tonalité à 6554 ou
plus (sa 6616), et les six lignes de la sonde sont l'ascension de la sinusoïde
vers sa crête à la trame 25 — 8191 × 5 = 40955 — chacune atterrissant sur 32767.
Le compte dit à quel point ce mixage est fort : 9040 des 22050 trames, 41 % de
l'échantillon, dépassent le format et s'assoient sur une limite. Et le rapport
de fin n'est touché par rien de tout cela — `22050 frames fed in 30 buffers` —
parce qu'il compte le curseur du canal 0 et que les cinq canaux dépensent le
même échantillon au même rythme.

La colonne des valeurs retournées est l'artefact que l'écrêtage remplace. À la
trame 15, un accumulateur qui se retourne mettrait −32456 dans le flux —
l'ascension du son vers sa crête se change en trou profond dès qu'il devient
trop fort — et à la crête elle-même −24581, au creux +24581. Les passages forts
sont exactement là où le retournement distord le plus, et l'écrêtage y répond à
tous de la même façon : atterrir sur la limite.

Maintenant la frontière. Un échantillon d'amplitude 1,0 — sa crête 32767, le
bord du format lui-même — porte **un** canal à plein volume avant que l'écrêtage
s'enclenche : un canal émet exactement 32767 et passe intact, deux s'additionnent
à 65534 et atterrissent sur 32767. L'affirmation de la leçon 063 est donc
serrée : un canal seul ne quitte jamais le format, et la pleine échelle est
exactement la place d'un canal. C'est au mixage que la place s'épuise.

*Page traduite de la version anglaise `book/solutions/lesson-064/ex1.md`,
révision `36cc491`.*

<!-- translation-source: book/solutions/lesson-064/ex1.md @ 36cc491 -->
