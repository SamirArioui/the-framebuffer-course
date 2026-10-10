# Solution : exercice 2 — Le budget de frames

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — Le budget de frames](../../lessons/part-1/lesson-036-frame-time.md) de la leçon 036.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-036/ex2.patch}}
```

## Visite guidée

Une mesure devient une décision quand elle a une ligne dont on peut se
retrouver du mauvais côté. Le **budget de frames** est cette ligne : le temps
qu'une frame a le droit de prendre. La ligne des 60 fps est `1/60 = 16.7 ms`
— le nombre auquel un jeu se tient quand il veut que la fenêtre paraisse
vivante.

Le changement place le budget là où sont les données : `FRAME_BUDGET` à côté
de `FrameStats`, et le cumul compte chaque frame qui tient dedans. Le résumé
de sortie gagne une ligne :

```
$ DISPLAY=:99 xdotool key --delay 40 --repeat 8 --window <id> Right
engine: 17 frames — avg 1.070 ms (update 0.000, render 0.482, present 0.587)
engine: worst frame 1.842 ms (frame 2); present is 54% of the frame
engine: within the 16.7 ms budget: 17/17 frames
```

17 sur 17 — le résultat honnête pour un moteur qui dessine un seul carré : la
pire frame mesurée ici est sous les 2 ms, donc le budget n'est pas encore
intéressant. Il le devient exactement comme le cours grandit : la partie 2
remplit le framebuffer d'une scène, et le nombre se met à bouger ; la partie 5
transforme ce compteur en rapport de budget de frames — quelles frames
tiennent, lesquelles non, et où le temps est passé quand elles n'ont pas tenu.

C'est aussi pourquoi le budget vit dans le code d'enregistrement du moteur et
non dans la couche plateforme : le temps qu'une frame *peut* prendre relève de
la politique du jeu. Ce qu'une frame a *effectivement* pris se mesure sur
l'horloge de la plateforme. Les deux se rejoignent dans une ligne
d'arithmétique, et cette ligne est celle de cet exercice.

*Page traduite de la version anglaise `book/solutions/lesson-036/ex2.md`,
révision `5342420`.*

<!-- translation-source: book/solutions/lesson-036/ex2.md @ 5342420 -->
