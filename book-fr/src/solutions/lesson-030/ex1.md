# Solution : exercice 1 — FillRect, de retour de la Partie 0

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — FillRect, de retour de la Partie 0](../../lessons/part-1/lesson-030-framebuffer.md) de la leçon 030.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-030/ex1.patch}}
```

## Visite guidée

`FillRect` est celui de paint, avec le pliage de la leçon 015 déplacé là où il
appartient : la découpe se fait dans *l'espace du rectangle* — un seul bornage
du rectangle contre les bords du tampon — et les deux boucles internes ne
parcourent ensuite que des pixels qui existent. Une vérification remplace une
vérification par pixel, et le parcours est exactement le rectangle découpé.

Les trois rectangles exercent les trois cas de découpe :

```
$ DISPLAY=:99 ./build/game
engine: framebuffer 640x480, 1228800 bytes, stride 2560
engine: pixel (0,0) = 255 0 0
engine: pixel (639,479) = 200 0 200
engine: pixel (60,101) = 32 32 64
engine: pixel (150,150) = 200 120 40
engine: pixel (20,410) = 0 200 200
engine: pixel (630,470) = 200 0 200
engine: presented
```

`(150,150)` se trouve à l'intérieur du bloc plein de 200×100 en (100,100).
`(20,410)` se trouve à l'intérieur du rectangle découpé à gauche : il demandait
à commencer en x=-20, et le pliage a ramené sa première colonne à x=0 — les
pixels qui existent ont été peints, ceux qui n'ont jamais existé ne l'ont pas
été. `(630,470)` montre le rectangle de coin, qui demandait 100×100 en
(600,440) et en a obtenu 40×40.

Une chose que le rectangle de coin révèle : `(639,479)` n'est plus vert. Le
coin survivant du rectangle découpé le recouvre — les rectangles peignent dans
l'ordre où ils sont appelés, les derniers par-dessus les premiers, le même
ordre du peintre que le `paint` de la Partie 0 et la table de vues de la
leçon 025 utilisaient. Rien ici ne découpe *contre ce qui est déjà dessiné* ;
ça, c'est la composition, et c'est l'affaire de la Partie 2.

*Page traduite de la version anglaise `book/solutions/lesson-030/ex1.md`, révision `da55ef9`.*

<!-- translation-source: book/solutions/lesson-030/ex1.md @ da55ef9 -->
