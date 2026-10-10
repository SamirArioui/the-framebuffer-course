# Solution : exercice 1 — La secousse qui s'amortit

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — La secousse qui s'amortit](../../lessons/part-5/lesson-092-hitstop-shake.md) de la leçon 092.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-092/ex1.patch}}
```

## Visite guidée

L'enveloppe de la secousse est une division. `Feedback` gagne `shake_total` —
toute la vie de la secousse — enregistré là où `FeelShake` la déclenche, et le
pilotage dans `FeelUpdate` met à l'échelle la magnitude du décalage par ce qu'il
reste :

```cpp
double left = feel.shake_total > 0.0 ? feel.shake / feel.shake_total : 0.0;
double mag = feel.shake_mag * left;
camera.add_x = ((int)(feel.shake * 40.0) & 1) ? (int)mag : -(int)mag;
```

L'alternance n'est pas touchée (le signe bascule toujours tant que la secousse
tourne) et le repos n'est pas touché — le repos propre du hook à exactement
`0,0` est la règle qui devait tenir, et elle tient. Seule la *taille* des
oscillations diminue à mesure que la vie de la secousse s'épuise, si bien que la
caméra revient au repos au lieu de repartir d'un coup sur une oscillation pleine.

L'exécution — l'effectif jetable de l'extrait du coup fatal de la leçon, un type
fragile et immobile, la sonde imprimant le décalage à chaque frame où la secousse
le pilote — montre la descente (la secousse de 10 px de la mort) :

```
engine: hit: bolt hits bag — damage 1, health 1 -> 0
engine: feel: shake fired (5 px, 0.25s)
engine: feel: shake fired (10 px, 0.50s)
engine: probe: camera add -9,0 (shake 0.457s left)
engine: probe: camera add -8,0 (shake 0.414s left)
engine: probe: camera add -7,0 (shake 0.370s left)
engine: probe: camera add 6,0 (shake 0.327s left)
engine: probe: camera add 5,0 (shake 0.284s left)
engine: probe: camera add 4,0 (shake 0.241s left)
engine: probe: camera add 3,0 (shake 0.198s left)
engine: probe: camera add -3,0 (shake 0.155s left)
engine: probe: camera add -2,0 (shake 0.112s left)
engine: probe: camera add -1,0 (shake 0.069s left)
engine: probe: camera add 0,0 (shake 0.026s left)
engine: feel: shake rested at 0,0
```

`-9, -8, -7, 6, 5, 4, 3, -3, -2, -1, 0` — l'oscillation tombe de 9 px à moins
d'un pixel sur la demi-seconde de la secousse, toujours alternante, et les
dernières frames de la secousse déplacent la caméra de moins d'un pixel : la
secousse s'amortit jusqu'au repos au lieu de s'y couper. Puis la ligne de repos
propre du hook : `shake rested at 0,0` — exactement zéro, tout l'intérêt.

Deux choses à remarquer dans les nombres. La première ligne de sonde lit `-9`,
pas `-10` : la frame dans laquelle la secousse s'est déclenchée a déjà décru une
fois, si bien que la première oscillation est une frame de décroissance plus loin
— la même granularité de frame que la leçon a mesurée sur l'échéance du hitstop.
Et le `shake …s left` de la sonde est le compte à rebours propre du hook, qui
tourne toujours en temps mural et termine toujours la secousse tout seul —
l'amortissement a changé l'allure de la secousse pendant qu'elle dure, pas le
moment où elle se termine, ni le fait qu'elle se termine à exactement zéro.

*Page traduite de la version anglaise `book/solutions/lesson-092/ex1.md`,
révision `6f25fd1`.*

<!-- translation-source: book/solutions/lesson-092/ex1.md @ 6f25fd1 -->
