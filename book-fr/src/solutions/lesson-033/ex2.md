# Solution : exercice 2 — La touche qui ne veut pas lâcher

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — La touche qui ne veut pas lâcher](../../lessons/part-1/lesson-033-latching.md) de la leçon 033.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-033/ex2.patch}}
```

## Visite guidée

La prédiction d'abord : alt-tab hors d'un jeu en maintenant la touche de
déplacement, et sans gestion du focus le marqueur continue de bouger — pour
toujours. Le relâchement se produit dans *une autre* fenêtre ; notre fenêtre ne
le voit jamais, donc `keys[KEY_LEFT]` reste à vrai jusqu'à ce que quelque chose
d'autre le change. Une touche bloquée est le symptôme classique, et la cause est
toujours la même : **de l'état de touche sans conscience du focus**.

Avec l'impression d'instrumentation en place, le pliage montre le moment qui le
produirait. Maintenez `left`, puis retirez le focus :

```
platform: focus out — dropping held keys
platform: focus in
platform: focus out — dropping held keys
engine: framebuffer 640x480, 1228800 bytes, stride 2560
engine: pixel (0,0) = 255 0 0
engine: pixel (639,479) = 0 255 0
engine: pixel (60,101) = 32 32 64
engine: presented
engine: polled: -
engine: focus gained
engine: polled: left
engine: pressed left
engine: polled: -
engine: focus lost
engine: polled: -
```

La touche était maintenue (`polled: left`), le focus est parti, et la scrutation
suivante est vide : la couche plateforme a abandonné chaque touche maintenue sur
`FocusOut`. Le relâchement arrivé pendant l'absence de focus n'a rien changé —
il n'y avait plus rien à relâcher. Reprendre le focus ne ressuscite pas la
touche : le focus n'est pas de l'état d'entrée.

Les impressions d'instrumentation sont les lignes `platform:` ; les lignes
`focus gained`/`focus lost` du moteur viennent de `HasFocus`, l'état que le
pliage maintient à côté des touches. Sur votre propre machine, faites-le avec un
vrai alt-tab — cliquez ailleurs en maintenant une touche de déplacement et
regardez l'abandon se produire dans le rapport, au lieu de regarder votre
marqueur dériver dans un mur.

*Page traduite de la version anglaise `book/solutions/lesson-033/ex2.md`, révision `3cf02bf`.*

<!-- translation-source: book/solutions/lesson-033/ex2.md @ 3cf02bf -->
