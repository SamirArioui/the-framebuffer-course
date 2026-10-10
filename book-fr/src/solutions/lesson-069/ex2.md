# Solution : exercice 2 — La table d'acceptation de la partie

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — La table d'acceptation de la partie](../../lessons/part-3/lesson-069-demo.md) de la leçon 069.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-069/ex2.patch}}
```

## Visite guidée

Le patch fait en sorte que la démo nomme ce qu'elle utilise — une ligne par
groupe de capacités, imprimée avant la première frame :

```
engine: drawing: one blit for sprites, glyphs, and tiles; clipping and transparency included
engine: text: strings laid out one slot per character, missing glyphs skipped
engine: world: the map loaded whole, drawn through the camera's summed offset; the mover gated by collision
engine: input: polled movement, one space tap shakes the camera
engine: sound: samples loaded whole, channels with cursors and volumes mixed per frame into one stream
engine: measurement: one record per frame, every phase named
```

Ces six lignes sont la colonne de gauche de la table. Le reste est à vous de
remplir — et la forme qui la rend digne d'être conservée est celle-ci (la
colonne des preuves vient des exécutions de cette leçon : la démo ci-dessus, et
une exécution contre un `music.wav` tronqué et un `sprite.ppm` supprimé dans
une copie jetable de `assets/`, donc les fichiers du dépôt n'ont jamais été
touchés) :

| Scénario de la spécification | Leçon | Preuves tirées d'une exécution |
| ---------------------------- | ----- | ------------------------------ |
| Le son se charge en octets entiers dans l'arena | 059, 061-062 | `music: 132300 frames at 44100 Hz, 1 channel, peak 10442, first frames: 0 277 554 831 …, last frame 0` |
| Le conteneur WAV est analysé à la main | 061 | le compte, la fréquence et les canaux de la ligne de faits sont les affirmations de l'en-tête lui-même, vérifiées contre les octets |
| Un fichier manquant ou malformé est un échec typé | 059, 061-062 | `assets/music.wav: could not load (malformed)` sur un fichier tronqué ; `assets/sprite.ppm: could not load` sur un fichier manquant |
| La lecture s'arrête là où l'échantillon finit | 062 | les 8820 trames de l'effet jouent jusqu'à leur fin et son canal revient — `effect 4 -> channel 1` |
| Un canal joue à son volume | 063 | `mix: effect 1 -> channel 1 (volume 64 of 256)` |
| Le mixage somme par frame et écrête | 064 | `first frames (music + effect 1, summed): 0 584 1163 1732 2286 2820 3330 3813` — réconciliable depuis les deux lignes de faits au chiffre près |
| Les canaux sont alloués par une politique fixe | 065 | la rafale prend 1, 2, 3 et rend le 1 ; le canal 0 de la musique n'est jamais proposé |
| La musique boucle au curseur de son canal | 066 | `loop: music wrapped on channel 0 — wrap 1, 133035 frames played, cursor 735 of 132300` |
| Les effets sont des one-shots sur des canaux du pool | 067 | vingt-cinq effets, des canaux libérés et repris à chaque fin |
| Musique et effets partagent un seul mixeur | 068 | trois retournements et vingt-cinq effets dans une même exécution ; les octets sommés contiennent les deux |
| La sortie du périphérique vit derrière la couture | 060 | `stream: 735-frame buffers, horizon 16.7 ms` ; `check-boundary.sh` ne nomme toujours aucun OS dans le code du moteur |
| L'attente cadencée garde le périphérique alimenté | 060 | un tampon par horizon ; `58.3 frames/s while awake, sound fed at 58.3 buffers/s` |
| Le mixage est mesuré, pas deviné | 060 | `audio 0.028 ms` dans chaque ligne `frame N:`, sur l'horloge de la plateforme |

La dernière question — quelles lignes cassent en premier quand le moteur
change — est là où la table gagne son utilité :

- **Les lignes exactes à l'octet cassent d'abord à tout changement de
  mixeur.** Touchez à l'ordre de la somme, à la troncature en virgule fixe du
  volume ou à l'écrêtage, et la ligne `first frames` cesse immédiatement de se
  réconcilier. C'est délibéré : ces lignes coûtent peu à relancer et disent
  précisément ce qui a cassé.
- **Les lignes de cadence cassent en silence.** Un changement de cadencement
  ou de retournement plante rarement quoi que ce soit — l'exécution cesse
  simplement de s'accorder avec sa propre arithmétique. Les `frames played` des
  retournements et les deux taux existent parce que « ça sonne bien » n'est pas
  une vérification.
- **Les lignes d'échec typé sont les moins chères à relancer et les plus
  faciles à oublier.** Elles ne se déclenchent que sur des fichiers cassés —
  ce qui est exactement le moment où vous en avez besoin.
- **Les lignes de cumul dérivent plutôt qu'elles ne cassent.** Les nombres
  bougent avec la machine et le périphérique ; ce qui ne doit pas changer,
  c'est la *forme* — une phase par frame, une ligne par phase, des sommes et
  des parts tirées de vraies frames.

Une limite que la table doit continuer de dire tout haut : chaque ligne de
preuve ci-dessus vient du périphérique `null` d'ALSA. Les octets sont
vérifiés ; l'écoute ne l'est pas, et la table n'est pas l'endroit pour prétendre
le contraire.

*Page traduite de la version anglaise `book/solutions/lesson-069/ex2.md`,
révision `f5e9029`.*

<!-- translation-source: book/solutions/lesson-069/ex2.md @ f5e9029 -->
