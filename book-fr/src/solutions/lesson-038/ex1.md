# Solution : exercice 1 — La capture d'écran

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — La capture d'écran](../../lessons/part-1/lesson-038-file-write.md) de la leçon 038.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-038/ex1.patch}}
```

## Visite guidée

La leçon 017 écrivait un fichier image à la main dans `paint` — le moteur peut
désormais faire la même chose avec ses propres pixels et sa propre couture.
`SaveScreenshot` construit un fichier PPM en mémoire : un en-tête de trois
lignes (`P6`, les dimensions, la valeur maximale) suivi d'un triplet RGB par
pixel. La conversion des octets bleu-vert-rouge-x du framebuffer vers les
octets rouge-vert-bleu du fichier, c'est le savoir sur l'ordre des octets de la
leçon 013 en plein travail : le format de la couture est pour la fenêtre, le
format du fichier est pour le fichier, et le moteur connaît les deux.

Un détail à remarquer dans le patch : la capture d'écran dessine la scène
*avant* de sauvegarder — effacer, marqueur, puis écrire. Au démarrage, le
framebuffer contient ce que contient le tableau statique (des zéros) ; les
pixels existent une fois que le rendu les a écrits. Une capture d'écran, c'est
une *frame*, pas un tampon.

Vérifié — l'exécution écrit le fichier, et le relire avec n'importe quel lecteur
PPM retrouve exactement la scène dessinée par le moteur :

```
$ DISPLAY=:99 ./build/game /tmp/opencode/xcheck/rt.bin /tmp/opencode/xcheck/shot.ppm
engine: round-trip ok: 256 bytes match
engine: saved screenshot to /tmp/opencode/xcheck/shot.ppm (640x480 PPM)
```

```
ppm header: b'P6\n640 480\n255\n'
payload bytes: 921600
pixel (308,228): (240, 220, 80)   # the marker's yellow, top-left corner
pixel (10,10):   (32, 32, 64)     # the background's blue-gray
```

921 600 octets, c'est exactement `640 × 480 × 3` — l'en-tête plus un triplet
RGB par pixel — et le pixel à la position rapportée du marqueur est de la
couleur du marqueur. Les pixels du moteur, écrits par le moteur, lisibles par
tout ce qui lit du PPM. Voilà à quoi servent les « écritures de fichier
entier ».

*Page traduite de la version anglaise `book/solutions/lesson-038/ex1.md`,
révision `dabf156`.*

<!-- translation-source: book/solutions/lesson-038/ex1.md @ dabf156 -->
