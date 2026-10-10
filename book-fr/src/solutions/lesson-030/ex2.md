# Solution : exercice 2 — Les quatre octets

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — Les quatre octets](../../lessons/part-1/lesson-030-framebuffer.md) de la leçon 030.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-030/ex2.patch}}
```

## Visite guidée

Deux lignes d'instrumentation lisent le tampon comme la leçon 013 lisait les
fichiers d'image : brut, à des offsets connus. La prédiction à faire avant de
lancer : quels quatre octets contiennent un pixel rouge et quels quatre
contiennent le fond.

```
$ DISPLAY=:99 ./build/game
engine: framebuffer 640x480, 1228800 bytes, stride 2560
engine: pixel (0,0) = 255 0 0
engine: pixel (639,479) = 0 255 0
engine: pixel (60,101) = 32 32 64
engine: bytes at (0,0): 00 00 ff 00
engine: bytes at (1,0): 40 20 20 00
engine: presented
```

Le pixel (0,0) est rouge et ses octets se lisent `00 00 ff 00` : **bleu
d'abord, vert ensuite, rouge en troisième, un octet inutilisé** — `PutPixel`
écrit `p[0]=b, p[1]=g, p[2]=r`, et le dump est la machine qui confirme l'ordre
au lieu du nom. Le fond en (1,0) se lit `40 20 20 00` — `0x40` vaut 64 (bleu),
`0x20` vaut 32 deux fois (vert et rouge) — même ordre, couleur différente.

Pourquoi cet ordre et pas rouge-vert-bleu ? Parce que le format est *le
contrat de la couture*, choisi pour être ce que le système de fenêtrage de la
machine de référence transporte nativement : sur x86-64 en petit boutisme, un
`ZPixmap` X11 à cette profondeur fait exactement ces quatre octets par pixel,
donc `XPutImage` copie notre tampon tel quel, sans traduire un seul octet. Une
seconde implémentation d'OS est libre de traduire en sortant — le contrat est
dans `platform.h`, pas dans X11.

Et pourquoi quatre octets quand trois portent la couleur ? L'alignement de la
leçon 007, qui paie maintenant à l'échelle du tampon : 4 octets par pixel
veulent dire que chaque ligne de 640 pixels fait 2560 octets — un nombre
entier de mots de 8 octets — donc aucune frontière de ligne ne chevauche
jamais un mot machine. L'octet inutilisé est le pas le moins coûteux possible.

*Page traduite de la version anglaise `book/solutions/lesson-030/ex2.md`, révision `da55ef9`.*

<!-- translation-source: book/solutions/lesson-030/ex2.md @ da55ef9 -->
