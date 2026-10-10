# Solution : exercice 1 — Où passe le render ?

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — Où passe le render ?](../../lessons/part-2/lesson-046-movable-sprite.md) de la leçon 046.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-046/ex1.patch}}
```

## Visite guidée

La prédiction d'abord, à partir des nombres déjà imprimés par la leçon. Le
render faisait en moyenne 0,435 ms ; le dessin du sprite 0,001 ms. La seule
autre chose que contient la phase de rendu est `ClearBuffer` — et elle touche
640 × 480 = 307 200 pixels contre les 256 du sprite. La prédiction est donc
nette : le clear prend *essentiellement tout* le rendu, et les phases nommées se
partageront à peu près en 400 : 1. Un moteur de rendu qui dessine un petit
sprite par frame est surtout un peintre de fond.

Le patch donne à `clear` son propre champ dans `FrameRecord`, l'additionne dans
le cumul, et le chronomètre exactement comme `sprites` — deux lectures d'horloge
autour d'un appel. L'exécution confirme la prédiction et l'affine :

```
frame 1: update 0.000 ms, render 0.368 ms (sprites 0.001, clear 0.367), present 0.713 ms, total 1.081 ms
frame 2: update 0.000 ms, render 0.424 ms (sprites 0.001, clear 0.422), present 0.289 ms, total 0.713 ms
frame 3: update 0.000 ms, render 0.385 ms (sprites 0.001, clear 0.383), present 0.328 ms, total 0.713 ms
engine: 4 frames — avg 0.834 ms (update 0.000, render 0.401 incl. sprites 0.001, clear 0.399, present 0.433)
```

- **Le clear est le render.** 0,399 des 0,401 ms du render — 99,5 % — est
  `ClearBuffer`. C'est une copie comme une autre : 307 200 pixels × 4 octets =
  1 228 800 octets de fond écrits chaque frame, qu'on dessine dessus quelque
  chose ou non.
- **Le sprite est gratuit à cette taille.** 256 pixels à travers le blitter, une
  microseconde — sous le plancher de bruit de la frame. La question intéressante
  n'est pas son coût mais son *passage à l'échelle* : un second sprite ajoute à
  peu près la même microseconde ; un écran entier de sprites ne resterait pas
  gratuit. Le champ nommé est ce qui permet de répondre à cette question avec
  des nombres plutôt qu'au ressenti.
- **Les phases ne pavent pas parfaitement**, et c'est de la mesure, pas une
  erreur. `render − (sprites + clear)` est la comptabilité propre de la frame à
  l'intérieur de la phase : les deux lectures de `platform::Now()` autour de
  chaque sous-phase, la mise en place de la boucle. Quelques microsecondes de
  surcoût honnête — c'est pourquoi la règle de la leçon tient : les temps nommés
  vivent *à l'intérieur* de leur phase, et le nombre de la phase reste la vérité
  mesurée.

La question du second sprite posée par l'énoncé : ajouter un second appel
`BlitSprite` fait bouger `sprites` et laisse `clear` intact — l'attribution le
montrerait immédiatement, là où le nombre plat `render` l'aurait avalé. C'est
tout l'argument des lignes par sous-système de la table de budget de frames,
joué à l'échelle de la microseconde trois leçons avant que la table n'existe.

*Page traduite de la version anglaise `book/solutions/lesson-046/ex1.md`,
révision `cfc6de7`.*

<!-- translation-source: book/solutions/lesson-046/ex1.md @ cfc6de7 -->
