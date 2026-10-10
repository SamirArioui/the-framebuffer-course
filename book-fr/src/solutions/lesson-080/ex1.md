# Solution : exercice 1 — La caméra aux bords de la carte

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — La caméra aux bords de la carte](../../lessons/part-4/lesson-080-slice.md) de la leçon 080.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-080/ex1.patch}}
```

## Visite guidée

Le diff est une sonde de cinq frames : la position du héros forcée vers une
position de bord par frame, *avant* que le code de caméra de l'update ne
tourne — ainsi le bornage testé est celui qu'utilise le jeu, pas une copie. Le
rapport est la ligne de la caméra elle-même, imprimée à chaque changement.

Les prédictions, avant toute exécution. La base vaut `clamp(hero + 8 − 320, 0,
128)`, `clamp(hero + 8 − 240, 0, 32)` — cette carte défile de 128 pixels
horizontalement et de 32 vers le bas. Calculez chacune :

| héros | non bornée | base |
| ---- | --------- | ---- |
| `0,0` | −312, −232 | `0,0` — bornée, sur les deux axes |
| `400,232` | 88, 0 | `88,0` — qui suit en x, bornée en haut |
| `734,480` | 422, 248 | `128,32` — bornée, sur les deux axes (le coin éloigné de la carte) |
| `312,232` | 0, 0 | `0,0` — la frontière exacte : la vue centrée sur le héros |
| `734,232` | 422, 0 | `128,0` — bornée en x, bornée en haut |

Les exécutions, depuis l'état final de cette leçon plus le patch (la sonde
force les positions dans un ordre qui déplace la caméra à chaque frame, si
bien que chacune est rapportée) :

```
engine: camera base 88,0 (t=0.000)
engine: camera base 128,32 (t=2.003)
engine: camera base 0,0 (t=2.018)
engine: camera base 128,0 (t=2.034)
engine: camera base 0,0 (t=2.049)
```

Exactement les cinq prédictions, dans l'ordre de la sonde. Quelles réponses
sont le bornage qui parle et lesquelles la vue qui suit : `88,0` est la seule où
la vue *suit* — 88 est le x du héros poussé au centre de la frame — et même
elle est bornée en y, parce que cette carte n'est que 32 pixels plus haute que
la frame. `128,32`, `128,0`, `0,0` sont le bornage qui retient la vue à
l'intérieur du monde : le héros à `734,480` est au fond du coin inférieur droit
de la carte, et la caméra montre le coin de la carte plutôt que le vide
au-delà. Et `312,232` est la charnière — la seule position où le suivi et le
bornage s'accordent exactement à zéro.

Cette charnière mérite un instant, car c'est ce que signifie « réconcilié avec
les limites de la carte ». La caméra ne suit pas le héros aveuglément et elle
n'est pas épinglée à la carte ; elle est la *vue* du héros, bornée par le
monde. Quand le héros dépasse `312`, la vue commence à bouger ; quand il
dépasse `712` (`128 + 320 − 8`), la vue s'arrête et le héros continue
d'avancer. Les deux transitions sont dans l'arithmétique de cette carte et
toutes deux sont dans les rapports de l'exécution — c'est pourquoi la
vérification de la tranche est cette réconciliation et non une capture
d'écran.

Rien ici ne touche la caméra, le bornage ni la boucle du jeu : la sonde est
cinq positions forcées et le rapport propre de l'update.

*Page traduite de la version anglaise `book/solutions/lesson-080/ex1.md`,
révision `7d3e8c3`.*

<!-- translation-source: book/solutions/lesson-080/ex1.md @ 7d3e8c3 -->
