# Solution : exercice 2 — Les quatre coins

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — Les quatre coins](../../lessons/part-2/lesson-045-blit.md) de la leçon 045.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-045/ex2.patch}}
```

## Visite guidée

Le patch replie la vérification de découpage dans une fonction `CheckClip` —
dessiner en `(x, y)`, recalculer le rectangle atterri de la même façon que
`BlitSprite`, relire chaque pixel atterri, puis balayer le reste de la frame à
la recherche de pixels touchés — et l'appelle aux quatre positions.

Les prédictions, par la seule arithmétique. Le sprite fait 16×16 ; le rectangle
atterri est la boîte du sprite croisée avec la frame 640×480, donc le compte est
`landed_width × landed_height` :

| Position | Boîte du sprite | Atterrit à l'intérieur | Prévu |
| -------- | --------------- | ---------------------- | ----- |
| `(-4, -4)` | x −4..11, y −4..11 | x 0..11, y 0..11 | 12 × 12 = **144** |
| `(632, 472)` | x 632..647, y 472..487 | x 632..639, y 472..479 | 8 × 8 = **64** |
| `(-100, 0)` | x −100..−85, y 0..15 | (aucun) | **0** |
| `(640, 480)` | x 640..655, y 480..495 | (aucun) | **0** |

L'exécution est d'accord sur les quatre :

```
engine: clip (-4,-4): 144 pixels landed, 0 wrong, 0 touched outside
engine: clip (632,472): 64 pixels landed, 0 wrong, 0 touched outside
engine: clip (-100,0): 0 pixels landed, 0 wrong, 0 touched outside
engine: clip (640,480): 0 pixels landed, 0 wrong, 0 touched outside
```

Notez ce que l'arithmétique n'est *pas* : ce n'est pas « tout ce qui se
recouvre ». La position `(640, 480)` se trouve exactement au coin extérieur de
la frame — sa boîte ne partage aucun pixel avec la frame, et une boucle naïve
« dessiner ce qui se recouvre » qui commencerait à itérer à l'origine du sprite
aurait tourné 256 fois et les aurait tous écrits hors limites. Le rectangle de
découpage est ce qui transforme « aucun recouvrement » en « aucune itération ».

La colonne `0 touched outside` est le détecteur de retournement (wrap), et elle
vaut la peine d'être imaginée allumée. Une boucle de copie qui abandonnerait les
pixels *après* avoir calculé leur destination — ou qui laisserait l'indice de
ligne sortir de la largeur de la frame — écrirait les pixels abandonnés ailleurs
dans le tampon : les colonnes de gauche du sprite apparaissant au bord droit de
la frame, ou les lignes du haut étalées dans le début de la ligne suivante. Dans
ces bugs, `wrong` peut même afficher `0` : chaque pixel atterri est correct, et
la corruption est entièrement dans des pixels qui n'auraient jamais dû être
touchés. Seul le balayage extérieur attrape cela, parce qu'il est la seule
vérification qui affirme une *absence*. C'est l'habitude : quand une affirmation
dit « rien ne s'est passé », la vérification doit partir chercher le rien.

*Page traduite de la version anglaise `book/solutions/lesson-045/ex2.md`,
révision `81027bf`.*

<!-- translation-source: book/solutions/lesson-045/ex2.md @ 81027bf -->
