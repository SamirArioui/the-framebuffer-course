# Solution : exercice 2 — L'horloge du fondu

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — L'horloge du fondu](../../lessons/part-5/lesson-096-screens.md) de la leçon 096.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-096/ex2.patch}}
```

## Visite guidée

**La prédiction, écrite d'abord.** Le monde de l'écran de mort est figé —
l'échelle de l'état est nulle, donc le pas lit `0.000 ms` à chaque frame de
l'écran. Le fondu tourne à l'horloge murale (l'affirmation de la leçon), donc
il monte quand même : aux frames cadencées de l'exécution (~43 ms chacune) il
devrait gagner `0.043` par frame, atteindre ses `0.30 s` en sept frames
environ, et se poser exactement sur la couleur de l'écran. Les deux horloges
côte à côte — une figée, une qui tourne.

**L'exécution** correspond, ligne pour ligne :

```
engine: state play -> death (the hero's health reached zero)
engine: probe: fade 0.000 — step 0.000 ms, state death
engine: screen: death: "GAME OVER" / "SCORE 000424   TIME 0:06   WAVE 1/3" / "ENTER: TITLE"
engine: probe: fade 0.043 — step 0.000 ms, state death
engine: probe: fade 0.086 — step 0.000 ms, state death
engine: probe: fade 0.129 — step 0.000 ms, state death
…
engine: probe: fade 0.259 — step 0.000 ms, state death
engine: screen: death fade arrived at 56,16,16 (its own color)
engine: probe: fade 0.302 — step 0.000 ms, state death
```

`0.000, 0.043, 0.086, 0.129 … 0.259` — exactement la montée de `0.043` par
frame que donnent les frames cadencées — contre `step 0.000 ms` sur chaque
ligne. Sept frames après l'apparition de l'écran, le fondu arrive (`fade
arrived at 56,16,16`), et le `0.302` de la sonde après cela est le dépassement
de l'horloge borné à la cible — la valeur *se pose* sur la cible une fois
arrivée ; elle ne vagabonde pas au-delà.

**Le contrefactuel.** Si le fondu tournait au pas du jeu, chaque ligne lirait
`fade 0.000` : le temps de jeu de l'écran de mort est nul — la même gelée qui
retient le monde retient tout ce qui vit sur cette horloge — et l'écran
resterait suspendu au noir pour toujours, son fondu jamais commencé. La
séparation de la leçon 078, en entier :

- **Le temps de jeu** est pour la *simulation* — le monde, les vagues, les
  étincelles, les respirations des combats. Il gèle avec la pause, ralentit
  avec un hitstop, et c'est ce qui le rend juste pour tout ce que le jeu *est*.
- **L'horloge murale** est pour la *machinerie et la présentation* — les hooks
  du ressenti (qui doivent *finir* pendant que le jeu est arrêté), et désormais
  les fondus des écrans (qui doivent *commencer* pendant que le jeu est
  arrêté).

La règle empirique que le couple enseigne : demandez ce qui doit se produire
pendant que le monde est figé. Si la réponse est « cela doit continuer
d'avancer » — un effet qui se termine, un écran qui arrive —, c'est l'horloge
murale. Si la réponse est « cela doit attendre le joueur » — une vague, un
temps de recharge, une étincelle qui se pose —, c'est le temps de jeu. Se
tromper ici est invisible en test de jeu et évident en pause.

*Page traduite de la version anglaise `book/solutions/lesson-096/ex2.md`,
révision `ccbe035`.*

<!-- translation-source: book/solutions/lesson-096/ex2.md @ ccbe035 -->
