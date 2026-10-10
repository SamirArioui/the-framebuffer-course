# Solution : exercice 1 — Huit directions

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — Huit directions](../../lessons/part-1/lesson-034-first-frame.md) de la leçon 034.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-034/ex1.patch}}
```

## Visite guidée

La diagonale n'est pas un cinquième déplacement — c'est les deux axes lus en
même temps. La correction consiste à cesser de traiter les quatre touches comme
quatre déplacements exclusifs : lire chacune dans une composante de direction
(`dx`, `dy`), puis déplacer une seule fois du résultat. Deux touches maintenues
donnent `dx=1, dy=1` et le marqueur avance en diagonale ; les touches opposées
s'annulent (`dx=0`) ; une seule touche se comporte exactement comme avant.

Scripté, avec la combinaison Right+Down répétée cinq fois :

```
$ DISPLAY=:99 xdotool key --delay 50 --repeat 5 --window <id> Right+Down
engine: marker at 308,228
engine: marker at 316,228
engine: marker at 324,236
engine: marker at 324,244
engine: marker at 332,244
engine: marker at 340,252
engine: marker at 340,260
```

Regardez les états que chaque frame a trouvés : `(316,228)` a bougé sur x seul,
`(324,236)` sur les deux axes à la fois — un pas en diagonale — et `(324,244)`
sur y seul. La combinaison arrive en appuis et relâchements entrelacés, donc
toutes les frames ne trouvent pas les deux touches enfoncées ; chaque frame lit
l'état qu'elle trouve et bouge en conséquence. Maintenez deux flèches sur un
vrai clavier et l'auto-repeat garde les deux touches enfoncées entre les frames
— une diagonale régulière.

La forme par composantes répond aussi à la question que la forme à quatre `if`
ne peut pas traiter : que se passe-t-il quand gauche et droite sont enfoncées
en même temps ? `dx` somme à zéro — le marqueur ne bouge pas. La résolution
déterministe de l'entrée découle de l'arithmétique au lieu d'avoir besoin de ses
propres règles.

*Page traduite de la version anglaise `book/solutions/lesson-034/ex1.md`, révision `54451b2`.*

<!-- translation-source: book/solutions/lesson-034/ex1.md @ 54451b2 -->
