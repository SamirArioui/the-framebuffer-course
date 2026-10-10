# Solution : exercice 2 — Le dessin à vide

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — Le dessin à vide](../../lessons/part-2/lesson-053-tiles.md) de la leçon 053.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-053/ex2.patch}}
```

## Visite guidée

La prédiction, à partir de ce que fait la marche à chaque placement. La boucle
visite les 48 × 32 = 1 536 cellules à *chaque* origine — elle calcule une
position par cellule et appelle `BlitSprite`, et la découpe du blitter décide
ce qui est réellement copié. Les trois cas ne diffèrent donc que par le nombre
de blits qui finissent par faire un vrai travail :

- **`(0, 0)`** — chaque cellule tombe dans la frame 640×480, sauf les marges
  droite et basse (la carte fait 768×512) : copie presque complète.
- **`(-400, -300)`** — la frame montre la région de la carte à partir de
  (400,300) ; environ un quart des cellules sont visibles. Les autres touchent
  la découpe et ne coûtent presque rien.
- **`(-4096, -4096)`** — chaque cellule est loin à l'extérieur ; le rectangle
  de découpe de chaque blit est vide. La marche tourne quand même ; la copie
  est nulle.

Forme prédite : le premier nombre est le coût complet, le deuxième une
fraction proportionnelle aux cellules visibles, le troisième un plancher — le
coût propre de la marche, sans aucune copie. L'exécution :

```
engine: map walk at 0,0: 0.977 ms
engine: map walk at -400,-300: 0.247 ms
engine: map walk at -4096,-4096: 0.005 ms
```

Le gradient correspond, et les nombres séparent proprement le coût :

- **La marche : ~0,005 ms** — visiter 1 536 cellules, calculer 1 536
  positions, appeler 1 536 blits qui se découpent immédiatement à rien. Soit
  0,5 % du dessin complet.
- **La copie : ~0,97 ms** — les pixels. Neuf cents microsecondes qui évoluent
  avec les *pixels visibles*, et c'est pourquoi le dessin au placement
  intermédiaire (0,247 ms) suit d'aussi près la fraction visible.

La découpe fait donc déjà le gros du travail : les tuiles hors écran sont bon
marché, et le coût fixe de la marche est négligeable à cette taille de carte.
Mais remarquez ce que « bon marché » n'est pas : ce n'est pas **gratuit**, et
cela évolue avec le nombre *total* de cellules, pas avec les cellules
visibles. Une carte 256×256 (65 536 cellules) dépenserait ~0,2 ms par frame
rien qu'en marche, même défilée entièrement hors de la vue.

Sauter plutôt que découper — la dernière question de l'énoncé — c'est le
**culling** : calculez le rectangle des cellules qui intersectent la frame
(`-x / TILE_SIZE` à `(FRAME_WIDTH - x) / TILE_SIZE`, borné à la carte) et ne
parcourez que celles-là. Le code interne ne change pas ; les bornes de la
boucle, si. C'est le levier « copier moins » de la liste de la leçon 047,
appliqué au niveau de la marche — et la passe d'optimisation de la partie 5
est l'endroit où une version de cela atterrira si le profileur dit que cette
ligne est chaude. La mesure ci-dessus est exactement la preuve dont cette
décision aura besoin, déjà dans la phase `tilemap` de l'enregistrement de
frame.

*Page traduite de la version anglaise `book/solutions/lesson-053/ex2.md`,
révision `2ac62b8`.*

<!-- translation-source: book/solutions/lesson-053/ex2.md @ 2ac62b8 -->
