# Solution : exercice 2 — L'écouter sur de vrais haut-parleurs

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — L'écouter sur de vrais haut-parleurs](../../lessons/part-3/lesson-059-samples.md) de la leçon 059.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-059/ex2.patch}}
```

## Visite guidée

Le patch ajoute un rapport au chemin de succès d'`OpenAudioOutput`, dans
`src/platform_alsa.cpp` — le fichier autorisé à connaître le nom du périphérique.
Le nom est une affaire d'OS : il est imprimé là où il est connu et ne franchit
jamais la couture, donc `platform.h` reste tel quel et le contrôle de frontière
de la leçon 042 continue d'imprimer son compte rendu inchangé. Sur la machine de
l'auteur, l'exécution dit maintenant ce qu'elle a ouvert :

```
platform: audio output: null (44100 Hz, 2 device ch)
engine: tone: 22050 frames at 44100 Hz, first frames: 0 513 1024 1531
engine: tone played
```

C'est toute la vérification que cette machine peut faire — `null` accepte et
jette les trames à travers les mêmes appels open/submit/close qu'un vrai
périphérique. Le reste de l'exercice se passe sur votre matériel :

- **Le build.** `libasound2-dev` installé (le prérequis du README), puis
  `./build.sh`. La ligne d'édition de liens veut `-lasound` ; rien d'autre ne
  change dans le build.
- **Le périphérique.** Sans `ALSA_DEVICE`, `DeviceName` répond `default` — quoi
  que votre machine entende par là. `aplay -l` liste ce qui existe ;
  `ALSA_DEVICE=<name> ./build/game` en choisit un exprès (un casque USB,
  disons), et la nouvelle ligne de rapport consigne le choix dans le journal.
- **L'écoute.** La tonalité est une demi-seconde de la 440 — 440 Hz à
  l'amplitude 0,25 — soumise au démarrage, avant la boucle de frames, et
  `CloseAudioOutput` vide la file avant de fermer, donc chaque trame soumise est
  jouée même si la fenêtre se ferme juste après. Attendez-vous à une note discrète,
  stable et clairement définie en hauteur, durant environ une demi-seconde.
- **Les chemins d'échec, tels qu'ils s'impriment.** Pas de sortie utilisable,
  c'est l'échec typé — la ligne propre à ALSA, puis
  `engine: no audio output on this machine` /
  `engine: continuing without sound` — et l'exécution se poursuit jusqu'à une
  fermeture propre. Un périphérique qui s'ouvre mais refuse de prendre les
  trames imprime `engine: the output would not take the samples`. Ni l'un ni
  l'autre n'est un plantage ; tous deux sont des conditions nommées.
- **La subtilité du volume.** L'exécution ne règle aucun volume de mixeur :
  l'amplitude 0,25 est un quart de l'étendue du *format*, pas un quart de celle
  de vos haut-parleurs. C'est dans `alsamixer` qu'une machine mystérieusement
  silencieuse reprend de la voix.

Rapportez ensuite ce que vous avez entendu — la ligne du périphérique, la durée,
la hauteur, le volume — ainsi que tout ce qui diffère de ce que la leçon prédit.
Un la stable d'environ une demi-seconde est le résultat attendu ; si ce que vous
avez entendu n'est pas cela (vitesse fausse, clic au début ou à la fin, silence
malgré `tone played`), l'écart est plus intéressant qu'une réussite. Les
vérifications de l'auteur s'arrêtent là où s'arrête le périphérique `null` ;
c'est dans cet exercice que « est-ce que ça sonne vraiment juste ? » trouve
enfin un témoin.

Une remarque sur la surcharge d'environnement, puisqu'un portage la rencontrera :
`ALSA_DEVICE` est lue à l'intérieur du fichier ALSA et ne fait pas partie du
contrat de la couture — le fichier d'un second OS choisit son périphérique à sa
façon. Le moteur se comporte à l'identique quel que soit le périphérique ouvert ;
seule la ligne de rapport diffère.

*Page traduite de la version anglaise `book/solutions/lesson-059/ex2.md`,
révision `8c2591b`.*

<!-- translation-source: book/solutions/lesson-059/ex2.md @ 8c2591b -->
