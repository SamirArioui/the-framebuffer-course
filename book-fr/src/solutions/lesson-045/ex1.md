# Solution : exercice 1 — Votre propre clé

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — Votre propre clé](../../lessons/part-2/lesson-045-blit.md) de la leçon 045.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-045/ex1.patch}}
```

## Visite guidée

Le patch déplace la décision de clé depuis la table de conventions du chargeur
vers les pixels : `LoadSprite` lit les trois premiers octets de la copie — le
pixel en haut à gauche de l'image — et ceux-ci deviennent `key_r, key_g,
key_b`. Une ligne d'inspection montre ce que le sprite prétend désormais :

```
engine: pixel bytes sum to 125580
engine: key color from the sprite's corner: 255,0,255
engine: blit check: 130 opaque pixels drawn unchanged, 0 mismatches
engine: blit check: 126 key pixels wrote nothing over the background
engine: blit check: clip at -4,-4 landed 144 pixels, 0 wrong, 0 touched outside
```

Le sprite du cours garde la même clé qu'avant — son coin *est* magenta — et les
trois vérifications passent inchangées. La règle a bougé ; le comportement, non.

Maintenant la seconde moitié. Un sprite dont le coin est lime `(0, 255, 0)` et
dont l'art est *magenta* — l'exact inverse de la convention du cours — chargé
par le même code :

```
engine: pixel 0,0 = 0,255,0
engine: pixel 8,8 = 255,0,255
engine: pixel bytes sum to 111180
engine: key color from the sprite's corner: 0,255,0
engine: blit check: 180 opaque pixels drawn unchanged, 0 mismatches
engine: blit check: 76 key pixels wrote nothing over the background
engine: blit check: clip at -4,-4 landed 144 pixels, 0 wrong, 0 touched outside
```

La clé est lime, donc les 76 pixels lime n'écrivent rien et les **180 pixels
magenta sont dessinés comme art** — le fichier et le framebuffer étant d'accord
sur chacun d'eux. Le blit n'a jamais su qu'une règle avait changé ; il compare
simplement aux trois octets de clé du sprite lui-même.

Maintenant le coût. Toute convention de transparence sacrifie une couleur, et
celle-ci dépense le **pixel en haut à gauche** de l'image : quelle que soit la
couleur qui vit en `(0, 0)`, elle ne pourra jamais apparaître dans le dessin —
elle est déclarée comme rien. Une image qui a besoin de la couleur de son coin
comme art doit déplacer l'art (le coin est « l'échantillon de clé », comme la
note de palette d'un peintre), choisir une autre convention, ou faire grandir un
format capable de *dire* quelle couleur est la clé — un champ que le PPM n'a
pas. C'est pourquoi des formats comme le PNG portent un canal alpha au lieu
d'une couleur clé : la question « quel pixel est rien » se répond par pixel, pas
par image. Notre format est volontairement à notre mesure, donc la convention
est à nous de choisir — et à nous de payer.

*Page traduite de la version anglaise `book/solutions/lesson-045/ex1.md`,
révision `81027bf`.*

<!-- translation-source: book/solutions/lesson-045/ex1.md @ 81027bf -->
