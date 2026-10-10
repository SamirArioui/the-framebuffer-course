# Solution : exercice 2 — L'entendre s'arrêter

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — L'entendre s'arrêter](../../lessons/part-3/lesson-062-playback.md) de la leçon 062.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-062/ex2.patch}}
```

## Visite guidée

Le patch est la preuve du journal : le compte rendu du flux à la fermeture de
l'exécution, imprimé à côté de la table du budget de frames. Les compteurs
voyagent avec l'alimentation — les tampons qui ont porté des trames
d'échantillons, les tampons qui n'en ont porté aucune — et l'horodatage de la
fin est pris là où la fin est nommée. Deux lignes, toutes deux issues d'une
vraie exécution de trois secondes face au périphérique `null` d'ALSA :

```
engine: sample: 22050 frames fed in 30 buffers — the sample's end; the stream is silence from here
engine: sample: 22050 frames in 30 buffers, then 145 buffers of silence over 2.5 s
```

Lisez-les comme la ligne de base du portage. La première dit que le canal a joué
exactement le nombre de trames de l'échantillon — 30 tampons × 735 trames =
22050 = `frame_count`, aucune trame manquante et aucune inventée. La seconde
dit que l'exécution ne s'est **pas** arrêtée quand le son s'est arrêté : 145
tampons de silence sur 2,5 secondes, un par horizon, jusqu'à la fermeture. Le
périphérique n'a cessé de recevoir ses tampons ; le silence est un flux. (La
branche `else` du compte rendu compte aussi : fermez la fenêtre tôt et il
rapporte une exécution qui se termine avant l'échantillon — une lecture
partielle, comptée honnêtement.)

Maintenant le portage, et la question à laquelle `null` ne peut pas répondre.
Emmenez la démo sur une machine dotée de vrai matériel sonore —
`libasound2-dev` installé, `./build.sh`, puis lancez avec `ALSA_DEVICE` non
défini (`default`) ou pointé sur ce que liste `aplay -l`. Ce que les nombres
prédisent avant que vous n'écoutiez :

- **La durée : exactement une demi-seconde.** 22050 trames à 44100 Hz font
  0,500 s, et trente horizons de 16,7 ms sont la même demi-seconde en tampons.
  Si ce que vous entendez est sensiblement plus long ou plus court, la fréquence
  propre du périphérique n'est pas 44100 et les temps de frame du journal
  montreront où l'écart s'est glissé.
- **La hauteur : 440 Hz, la note A4**, à amplitude 0,25 — une tonalité discrète,
  un quart de la plage du format : attendez-vous à une note douce plutôt qu'à un
  bip.
- **L'arrêt — le vrai sujet.** L'échantillon ne finit pas au repos : ses quatre
  dernières trames valent −2032, −1531, −1024, −513, là où le retournement (le
  fait de la leçon 059 qui rendait la boucle de la leçon 060 sans couture)
  aurait mis 0. Le silence commence à 0, donc le haut-parleur passe de −513 au
  repos — environ 1,6 % de la pleine échelle, un petit pas et non une chute.
  Écoutez à l'arrêt : est-ce propre, ou y a-t-il un léger clic ? Rapportez ce
  que vous entendez. Un clic ne serait ni un bug du chargeur ni un bug de
  l'alimentation ; ce serait la fin de cet échantillon lui-même, et c'est
  exactement le genre de question que seules des oreilles sur du matériel
  peuvent trancher.

Et l'honnêteté, gardée exactement là où ce cours la garde : aucun haut-parleur
de cette machine n'a émis de son — `null` prend les échantillons et les jette,
et tous les nombres ci-dessus sont le côté moteur de l'ordonnancement. Le
compte rendu que le patch imprime est de la comptabilité indépendante du
périphérique ; ce que le périphérique a fait de ces tampons est l'histoire que
votre machine a à raconter. Si vous voulez aussi le côté périphérique en
chiffres, la sonde de l'exercice 2 de la leçon 060 mesure les attentes que
`snd_pcm_writei` impose — même exécution, deux instruments.

Les chemins d'échec sont inchangés et méritent chacun une exécution sur le
portage : pointez `ALSA_DEVICE` vers un nom qui n'existe pas et l'exécution
rapporte l'échec typé puis continue sans son ; un périphérique qui s'ouvre mais
refuse les échantillons est nommé une fois et l'exécution retombe dans le
silence, son attente de nouveau non bornée. Ni l'un ni l'autre n'est un
plantage — et le compte rendu de l'échantillon à la fermeture montrera zéro
tampon alimenté, ce à quoi ressemble « pas de son » dans le journal.

*Page traduite de la version anglaise `book/solutions/lesson-062/ex2.md`,
révision `fb99fc9`.*

<!-- translation-source: book/solutions/lesson-062/ex2.md @ fb99fc9 -->
