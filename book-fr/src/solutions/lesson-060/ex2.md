# Solution : exercice 2 — La famine sur de vrais haut-parleurs

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — La famine sur de vrais haut-parleurs](../../lessons/part-3/lesson-060-stream.md) de la leçon 060.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-060/ex2.patch}}
```

## Visite guidée

Le patch ajoute l'instrument que demande l'exercice : une sonde dans
`src/platform_alsa.cpp` — le fichier autorisé à connaître le périphérique — qui
chronomètre chaque `snd_pcm_writei` et cumule ce que le périphérique a fait
attendre la boucle. Trois variables statiques à côté du tampon de préparation
(aucune allocation ; la loi du langage de la leçon 026 lie cette couche aussi),
une lecture d'horloge autour de chaque écriture, et un rapport à
`CloseAudioOutput`, après la vidange :

```
platform: audio: 176 writes, 0.360 ms waiting on the device (worst 0.012 ms)
```

C'est une vraie exécution au repos de trois secondes de l'état final de la leçon
plus ce patch, contre le périphérique `null` d'ALSA — et c'est le témoin de
référence contre lequel le portage se mesure. Lisez-le attentivement avant de
quitter cette machine :

- **176 écritures, c'est le nombre d'alimentations**, une par horizon — la sonde
  recoupe le compte rendu que l'attente cadencée fait elle-même de l'exécution.
  Si ce nombre et les frames d'alimentation du journal de frames divergent un
  jour, l'un des deux ment.
- **0,360 ms au total, 0,012 ms au pire — le périphérique n'a jamais repoussé.**
  `null` accepte et jette ; le peu qu'on mesure est la copie intermédiaire à
  l'intérieur de l'appel, pas un périphérique demandant à la boucle d'attendre.
  Une file qui ne se remplit jamais ne peut pas subir de famine ni appliquer de
  contre-pression. C'est exactement l'écart que nomme la section d'honnêteté de
  la leçon.

Maintenant le portage, et ce que la sonde devrait y montrer :

- **Le périphérique.** `libasound2-dev` installé, `./build.sh`, puis lancez avec
  `ALSA_DEVICE` non définie (`default`) ou pointée vers du vrai matériel —
  `aplay -l` liste ce qui existe. Lancez une fois au repos et une fois chargé :
  le sprite conduit dans un mur (une flèche maintenue), la caméra qui tremble
  (espace), pour un intervalle fixe à chaque fois.
- **La contre-pression est le nombre à comparer.** Sur du matériel, les
  écritures attendent quand le tampon du périphérique est plein : attendez-vous
  à des attentes totales et maximales *non nulles*, et à des attentes qui
  grandissent dans l'exécution chargée par rapport à celle au repos. La pire
  attente isolée est le chiffre le plus parlant — quand la boucle est en retard
  à l'alimentation, l'écriture bloque le temps d'obtenir la place qu'il lui
  faut, et une attente approchant un horizon (16,7 ms à la taille de tampon de
  la leçon) est le périphérique qui vous dit que la file a failli s'assécher.
  `null` n'en montre rien ; le matériel montre tout.
- **Les oreilles sont l'autre instrument.** La tonalité est un tampon continu
  d'une demi-seconde bouclé pour toujours à 440 Hz ; un flux alimenté est une
  note stable, et la famine est impossible à confondre — saccades, trous, ou la
  note qui décroche sous la charge et revient quand l'exécution se calme. La
  sonde consigne la cause en millisecondes ; la saccade est le même fait à
  l'échelle humaine. Rapportez les deux, et où cela s'est manifesté dans la
  comptabilité d'échéance : une saccade avec de faibles attentes signifie que
  *la boucle s'est réveillée tard* (vérifiez les écarts du journal de frames
  contre l'horizon) ; une saccade avec de fortes attentes maximales signifie que
  *le périphérique a retenu la boucle* — la contre-pression que l'attente
  cadencée existe pour respecter.
- **Les chemins d'échec s'impriment toujours comme ils sont.** Un périphérique
  manquant est l'échec typé de la leçon 059 et l'exécution continue sans son ;
  un périphérique qui s'ouvre mais refuse les échantillons est nommé une fois et
  l'exécution retombe dans le silence, son attente de nouveau non bornée. Ni
  l'un ni l'autre n'est un plantage.

Une remarque sur la surcharge d'environnement, puisqu'un portage la rencontre :
`ALSA_DEVICE` est lue à l'intérieur du fichier ALSA et ne fait pas partie du
contrat de la couture — le fichier d'un second OS choisit son périphérique à sa
façon, et c'est dans le `SubmitSamples` d'un second OS que vivrait sa propre
sonde. Le moteur se comporte à l'identique quel que soit le périphérique ouvert ;
seuls les nombres diffèrent.

Rapportez ensuite ce que vous avez trouvé : le périphérique, le repos contre la
charge, les attentes, ce que vous avez entendu, et si la comptabilité d'échéance
l'avait prédit. Le `0.360 ms` de l'exécution sur `null` est le plancher ; tout ce
qui est au-dessus, c'est votre machine qui vous apprend ce que « le périphérique
consomme à sa propre cadence » veut vraiment dire.

*Page traduite de la version anglaise `book/solutions/lesson-060/ex2.md`,
révision `264e818`.*

<!-- translation-source: book/solutions/lesson-060/ex2.md @ 264e818 -->
