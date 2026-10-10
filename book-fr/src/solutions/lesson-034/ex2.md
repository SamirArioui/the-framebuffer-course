# Solution : exercice 2 — La vitesse qui appartient au clavier

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — La vitesse qui appartient au clavier](../../lessons/part-1/lesson-034-first-frame.md) de la leçon 034.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-034/ex2.patch}}
```

## Visite guidée

Maintenez une touche fléchée sur votre machine et le marqueur bouge — mais *à
quelle vitesse*, ce n'est pas encore le moteur qui répond. Les frames arrivent
quand les nouvelles arrivent, et tant qu'une touche est maintenue, la nouvelle
est l'auto-repeat : la vitesse du marqueur est donc la cadence de répétition de
votre clavier, avec son délai initial. Machine différente, vitesse différente.
Le moteur possède ses pixels, pas encore son temps.

Le compteur de frames d'instrumentation rend cela mesurable. Chaque ligne de
déplacement porte la frame où il a eu lieu :

```
$ DISPLAY=:99 xdotool key --delay 50 --repeat 4 --window <id> Left
engine: arrow keys move the marker; close the window to stop
engine: marker at 308,228
engine: frame 2: marker at 300,228
engine: frame 4: marker at 292,228
engine: frame 6: marker at 284,228
engine: frame 8: marker at 276,228
```

Comptez les frames entre les pas : une frame sur deux a déplacé le marqueur. Les
pas font huit pixels chacun et arrivent quand l'état scruté dit qu'une touche
est enfoncée — la *distance* appartient au moteur (`MARKER_STEP`), le *rythme*
appartient aux nouvelles. Maintenez la touche une seconde sur votre machine et
comptez les pas : c'est votre cadence d'auto-repeat, moins ce que votre
gestionnaire de fenêtres en a mangé.

Ce qui doit changer pour que le moteur possède sa vitesse est exactement ce que
construisent les deux leçons suivantes : une **horloge**. Quand une frame sait
combien de temps elle a pris, le pas devient `speed × elapsed` et le déplacement
est en pixels par seconde, peu importe qui réveille la boucle — la leçon 035
donne cette horloge au moteur, et la leçon 036 rend le coût de la frame visible
tant qu'elle est là.

*Page traduite de la version anglaise `book/solutions/lesson-034/ex2.md`, révision `54451b2`.*

<!-- translation-source: book/solutions/lesson-034/ex2.md @ 54451b2 -->
