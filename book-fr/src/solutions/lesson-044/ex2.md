# Solution : exercice 2 — Sprite, voici la fenêtre

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — Sprite, voici la fenêtre](../../lessons/part-2/lesson-044-sprite-bytes.md) de la leçon 044.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-044/ex2.patch}}
```

## Visite guidée

Le patch ajoute `DrawSpriteRaw` — la boucle de copie de trois lignes qui est
exactement ce que la leçon 045 transforme en blitter : chaque pixel source
devient un `PutPixel`, couleur clé comprise. Elle dessine le sprite en `(32,32)`
une fois au démarrage pour la vérification, puis de nouveau dans la phase de
rendu après `ClearBuffer`, pour que le sprite soit à l'écran à côté du marqueur.

La vérification relit trois pixels et imprime les octets du fichier face à ceux
du framebuffer aux mêmes points :

```
engine: sprite pixel 0,0: file 255,0,255 framebuffer 255,0,255 (the key color drew itself)
engine: sprite pixel 8,8: file 220,40,40 framebuffer 220,40,40
engine: sprite pixel 0,1: file 255,0,255 framebuffer 255,0,255 (the key color drew itself)
```

Fichier et framebuffer sont d'accord à chaque point — la copie est fidèle. La
fenêtre aussi ; voici la même relecture côté OS :

```
$ DISPLAY=:99 ./winread "the framebuffer engine" 32 32 40 40 32 33 100 100
winread: window 4194305 is 640x480
winread: 32,32 -> r=255 g=0 b=255
winread: 40,40 -> r=220 g=40 b=40
winread: 32,33 -> r=255 g=0 b=255
winread: 100,100 -> r=32 g=32 b=64
```

La ligne intéressante est celle qui porte les parenthèses. Les pixels `(0,0)` et
`(0,1)` sont de couleur clé — le magenta que l'artiste voulait comme *rien* — et
la boucle les a quand même dessinés en magenta. Le fichier disait « ce pixel est
transparent » dans le seul langage qu'a un PPM, la couleur du pixel lui-même, et
la boucle brute n'a aucune idée qu'une telle règle existe. Le fond du sprite est
donc un carré magenta à l'écran, dessiné *par-dessus* ce qui s'y trouvait.

C'est exactement ce que la leçon 045 doit : une boucle de copie qui consulte la
couleur clé et n'écrit rien pour elle — la transparence — et qui abandonne les
pixels dont la destination tombe hors du framebuffer au lieu de laisser
`PutPixel` les découper silencieusement un par un — le découpage comme une seule
décision, par ligne, là où la copie peut le voir. Les deux changements se
produisent à l'intérieur de la boucle que vous venez d'écrire. Rien d'autre de
cet exercice ne survit : la boucle est le blitter, et le blitter est la seule
pièce de code que les cinq leçons suivantes mesurent, lisent et vectorisent.

*Page traduite de la version anglaise `book/solutions/lesson-044/ex2.md`,
révision `7e8ed7f`.*

<!-- translation-source: book/solutions/lesson-044/ex2.md @ 7e8ed7f -->
