# Solution : exercice 2 — La base, aux coins

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — La base, aux coins](../../lessons/part-5/lesson-083-tilemap-camera.md) de la leçon 083.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-083/ex2.patch}}
```

## Visite guidée

La sonde garde la base que la caméra *voulait* (`want_x`, `want_y`) à côté de
celle qu'elle a *obtenue* après le bornage, et imprime les deux :

```
engine: camera base %d,%d (wanted %d,%d)
```

Chaque rapport est donc une vérification de bornage : quand `base` diffère de
`wanted`, le bornage l'a retenue ; quand ils coïncident, la vue était libre de
suivre.

**Les cinq prédictions.** La base est `hero + half sprite − half frame`, bornée
à `x ∈ [0, 768−640] = [0,128]`, `y ∈ [0, 512−480] = [0,32]`. Avec un sprite
`16×16` et une frame `640×480`, la base centrée vaut `hero.x − 312` et `hero.y − 232`.
Le calcul à chaque endroit :

| héros (px monde) | base voulue | base bornée | ce que le bornage a fait |
| --------------- | ----------- | ------------ | ------------------ |
| `(0, 0)` | `(−312, −232)` | `(0, 0)` | les deux bornées vers le haut |
| `(752, 0)` | `(440, −232)` | `(128, 0)` | x bornée vers le bas, y bornée vers le haut |
| `(0, 496)` | `(−312, 264)` | `(0, 32)` | x bornée vers le haut, y bornée vers le bas |
| `(752, 496)` | `(440, 264)` | `(128, 32)` | les deux bornées vers le bas |
| `(312, 232)` | `(0, 0)` | `(0, 0)` | aucune — déjà au coin |

**Quel bornage sur les deux axes ?** Les quatre coins de la carte bornent sur
les *deux* axes — et la ligne `(312,232)` donne la clé du pourquoi. La carte
fait `768×512` et la frame `640×480`, donc la vue ne peut défiler que de **128
pixels horizontalement et 32 verticalement** dans toute sa vie. C'est une plage
minuscule. La base centrée vaut `hero.x − 312`, et la seule façon de ne pas être
borné est que `hero.x` se trouve dans `[312, 440]` et `hero.y` dans
`[232, 264]` — une boîte de 128×32 pixels au milieu du monde. À chaque coin de
la carte, le héros est loin en dehors de cette boîte, donc les deux axes
bornent. Le départ `(312,232)` est le seul endroit de votre liste qui tombe
exactement sur l'origine non bornée : la caméra se trouve à `(0,0)` là sans y
être forcée.

Les lignes de sonde de l'exécution elle-même confirment que le bornage est réel,
pas modélisé — ici, le héros poussé assez loin vers le haut a fait passer
`wanted` en négatif en `y`, et le bornage l'a retenu à zéro :

```
engine: camera base 2,0 (wanted 2,-1)
```

`wanted 2,−1` : la base en suivi libre aurait été d'un pixel au-dessus du bord
supérieur de la carte. `base 2,0` : le bornage a refusé. Exactement
l'arithmétique du tableau, mesurée sur la frame qui a tourné.

L'intuition à emporter : cette carte ne défile presque pas. Le *bornage* de la
caméra fait beaucoup de travail ici parce que le monde n'est que légèrement plus
grand que la fenêtre — sur une carte plus grande, la caméra suivrait librement
sur la majeure partie du monde et ne bornerait que près de ses bords. La sonde
`wanted`/`base` est l'outil qui vous montre dans quel régime vous êtes à tout
moment.

*Page traduite de la version anglaise `book/solutions/lesson-083/ex2.md`,
révision `7f209d4`.*

<!-- translation-source: book/solutions/lesson-083/ex2.md @ 7f209d4 -->
