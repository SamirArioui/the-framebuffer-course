# Solution : exercice 1 — La vérification de la présentation

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — La vérification de la présentation](../../lessons/part-1/lesson-031-present.md) de la leçon 031.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-031/ex1.patch}}
```

## Visite guidée

Le contrat — *les pixels que le moteur a écrits sont les pixels que la fenêtre
montre* — mérite un vérificateur, et le vérificateur appartient au côté OS de
la couture, parce que lire les pixels d'une fenêtre est une opération d'OS.
`PresentedMatches` est ce vérificateur : `XGetImage` prend un instantané de la
fenêtre de la même façon que `XPutImage` l'a écrite — même géométrie, même
format de pixel — et la boucle compare chaque pixel avec les octets du dernier
`Present` du moteur. La mémoire de la relecture est à Xlib (contrairement au
tampon enveloppé dans `Present`) et est libérée avec un simple `XDestroyImage`.

Le moteur vérifie sa propre affirmation une fois, juste après la première
lumière :

```
$ DISPLAY=:99 ./build/game
engine: framebuffer 640x480, 1228800 bytes, stride 2560
engine: pixel (0,0) = 255 0 0
engine: pixel (639,479) = 0 255 0
engine: pixel (60,101) = 32 32 64
engine: presented
engine: presentation verified — window matches framebuffer
engine: close reported
engine: closed
```

Les 307 200 pixels ont tous correspondu — y compris les deux que nous avons
écrits à la main et celui écarté par la découpe qui n'a jamais atterri.

Un détail de comparaison qui mérite l'attention : le `XGetPixel` de la
relecture donne une valeur de pixel sur 32 bits, et seuls ses trois octets bas
sont de la couleur — le quatrième octet inutilisé du format de la couture ne
fait pas partie de ce que la fenêtre stocke. La comparaison le masque
(`& 0xFFFFFF`), et c'est exactement l'octet que l'exercice 2 de la leçon 030 a
identifié comme celui qu'aucune fenêtre ne nous rend.

Sur une vraie machine, ce vérificateur est la différence entre « je crois que
la fenêtre montre mes pixels » et « les pixels de la fenêtre égalent mon
tampon, octet pour octet ». C'est ce qui fait du scénario de la spec un
scénario plutôt qu'une promesse.

*Page traduite de la version anglaise `book/solutions/lesson-031/ex1.md`, révision `e5b6743`.*

<!-- translation-source: book/solutions/lesson-031/ex1.md @ e5b6743 -->
