# Solution : exercice 1 — Les deux intérieurs de la ligne

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — Les deux intérieurs de la ligne](../../lessons/part-3/lesson-070-audio-row.md) de la leçon 070.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-070/ex1.patch}}
```

## Visite guidée

Le patch applique la discipline des phases nommées de la partie 2 au son :
l'enregistrement de frame accueille `mix` et `submit` à l'intérieur d'`audio`,
le cumul les somme comme tout autre champ, la ligne `frame N:` les porte comme
elle porte `sprites`, `text` et `tilemap`, et la table les imprime comme des
lignes en retrait sous la phase qu'elles expliquent — à l'intérieur d'elle,
jamais à sa place. La table de l'exécution :

```
engine: frame budget — 466 frames, avg 2.167 ms, worst 4.030 ms (frame 409)
engine:   subsystem   avg ms    share
engine:   update       0.001       0%
engine:   audio        0.039       2%
engine:     mix        0.032       1%
engine:     submit     0.006       0%
engine:   render       1.390      64%
engine:     sprites    0.001       0%
engine:     text       0.007       0%
engine:     tilemap    0.963      44%
engine:   present      0.737      34%
engine:   total        2.167     100%
```

Les lignes du journal portent le découpage par frame :

```
frame 1: update 0.000 ms, audio 0.032 ms (mix 0.023, submit 0.007), render 1.488 ms (sprites 0.001, text 0.005, tilemap 0.851), present 1.365 ms, total 2.886 ms
frame 2: update 0.000 ms, audio 0.030 ms (mix 0.025, submit 0.005), render 1.249 ms (sprites 0.001, text 0.007, tilemap 0.868), present 0.512 ms, total 1.792 ms
```

Réconciliez les deux lignes contre celle sous laquelle elles vivent, la même
vérification croisée que subit la table : sur les 466 frames de l'exécution, le
journal fait en moyenne `audio 0.0387`, `mix 0.0320`, `submit 0.0063` — les
intérieurs somment à 0.0383, et l'écart de 0.0004 ms est le travail de la phase
hors des deux appels : le déclenchement d'effet du rythme, la surveillance du
retournement, la comptabilité du calendrier d'alimentation, et les lectures
d'horloge elles-mêmes. C'est exactement la forme de la famille render : les
lignes nommées expliquent la phase ; elles ne la remplacent pas, et le résidu
est du travail nommé, pas une erreur.

Maintenant la question de la ligne. L'intérieur `submit` lit 0.006 ms ici — pas
zéro : même `null` coûte un appel dans la bibliothèque du périphérique pour
prendre les échantillons et les jeter. Sur du vrai matériel, c'est la ligne qui
grandit : une soumission peut attendre de la place dans le tampon du
périphérique, et quand elle le fait, l'attente atterrit ici, pas dans le
mixage. Son jumeau structurel est `present` — les deux lignes sont le moteur
qui tend des octets à l'OS et les deux portent la synchronisation de la
passation. `present` attend la copie de l'affichage ; `submit` attend le tampon
circulaire du périphérique sonore. Ces deux lignes sont les deux endroits de la
frame où c'est la machine, pas le moteur, qui décide du temps que prend la
phase — c'est pourquoi toutes deux appartiennent aux questions de l'exercice de
portage.

*Page traduite de la version anglaise `book/solutions/lesson-070/ex1.md`,
révision `cc7fcb3`.*

<!-- translation-source: book/solutions/lesson-070/ex1.md @ cc7fcb3 -->
