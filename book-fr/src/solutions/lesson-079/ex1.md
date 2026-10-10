# Solution : exercice 1 — La frame en pause, prédite

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — La frame en pause, prédite](../../lessons/part-4/lesson-079-wall-clock.md) de la leçon 079.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-079/ex1.patch}}
```

## Visite guidée

Le diff est deux instantanés étiquetés de la même table de budget de frames :
l'un imprimé à l'instant où le bouton passe en pause — la table telle qu'elle
se présente pour les frames de jeu et de hitstop — et l'autre à la fin de
l'exécution, une fois que les frames en pause ont rejoint le cumul.
`PrintFrameBudget` est cumulatif, donc les deux tables diffèrent exactement par
les frames entre elles.

La prédiction, avant toute exécution. **La ligne de journal de la frame en
pause** lit d'abord `step 0.000 ms` — la simulation n'a pas avancé — puis des
phases ordinaires : `update` quelques centièmes de milliseconde (la lecture
d'entrée, la marche), `render` environ 1.3 ms (toute la scène encore dessinée),
`present` environ 0.3-0.5 ms (la copie vers la fenêtre), `total` leur somme.
Rien dans la ligne n'est nul sauf le pas. **La table du budget après une
exécution en pause**, comparée à une exécution en jeu : les *lignes* sont la
même image — `render` autour de 1.36 ms, `tilemap` autour de 0.94 ms,
`present` autour de 0.5 ms, `update` autour de 0.01 ms — parce qu'une frame en
pause fait le même travail. Les nombres qui bougent sont ceux de la
comptabilité : le compte de frames grandit, et la moyenne bouge de ce qu'est la
moyenne des frames en pause (la même que le reste). Aucune ligne ne s'effondre
vers zéro.

Les exécutions, depuis l'état final de cette leçon plus le patch — une
exécution, le bouton tournant à trois, cinq et sept secondes :

```
engine: budget before the pause:
engine: frame budget — 49 frames, avg 1.874 ms, worst 3.078 ms (frame 1)
engine:   subsystem   avg ms    share
engine:   update       0.008       0%
engine:   audio        0.000       0%
engine:   render       1.364      73%
engine:     sprites    0.006       0%
engine:     text       0.007       0%
engine:     tilemap    0.942      50%
engine:   present      0.501      27%
...
engine: budget at the end of the run:
engine: frame budget — 145 frames, avg 1.899 ms, worst 4.291 ms (frame 132)
engine:   subsystem   avg ms    share
engine:   update       0.013       1%
engine:   audio        0.000       0%
engine:   render       1.364      72%
engine:     sprites    0.006       0%
engine:     text       0.006       0%
engine:     tilemap    0.938      49%
engine:   present      0.522      27%
```

Exactement comme prédit : `render` vaut 1.364 ms dans les *deux* tables — le
même nombre au chiffre près, parce que les frames en pause ont dessiné la même
scène — et `tilemap`, `sprites`, `text`, `present` sont tous dans le bruit de
leurs valeurs d'avant la pause. La moyenne est passée de 1.874 à 1.899 ms sur
96 frames de plus, dont 32 en pause.

La phrase unique que l'exercice demande : **la table de l'exécution en pause
prouve que « le jeu s'est arrêté » et « la machine s'est arrêtée » sont deux
affirmations différentes** — le pas du jeu était nul pour 32 de ces frames et
l'enregistrement les a facturées comme les autres, ce qui est la seule façon
pour le budget de vous dire ce que coûte réellement un écran de pause. Un
enregistrement mis à l'échelle aurait montré une pause comme un cadeau de
frames gratuites ; un enregistrement d'horloge murale la montre comme la chose
honnête — la présentation qui tourne, le monde qui attend.

Un détail à remarquer : `worst 4.291 ms (frame 132)` est le voisin d'une frame
*en pause* — la pire frame d'une exécution peut être n'importe où, et le
`worst_number` de l'enregistrement est ce qui vous permet d'aller regarder sa
ligne dans le journal et de voir à quelle échelle elle a tourné. C'est l'autre
travail du champ step : rendre chaque ligne auto-descriptive.

Rien ici ne touche l'enregistrement, l'échelle ou l'arithmétique du budget : le
patch est deux impressions étiquetées de la table que la leçon avait déjà.

*Page traduite de la version anglaise `book/solutions/lesson-079/ex1.md`,
révision `4eaa176`.*

<!-- translation-source: book/solutions/lesson-079/ex1.md @ 4eaa176 -->
