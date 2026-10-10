# Solution : exercice 1 — La ligne qui n'est pas là

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — La ligne qui n'est pas là](../../lessons/part-2/lesson-058-budget.md) de la leçon 058.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-058/ex1.patch}}
```

## Visite guidée

Le patch calcule l'écart — `render − (sprites + text + tilemap)` — et l'imprime
comme une ligne à l'intérieur du render, là où vivent les autres lignes de
sous-système :

```
engine: frame budget — 26 frames, avg 2.252 ms, worst 3.392 ms (frame 19)
engine:   subsystem   avg ms    share
engine:   update       0.023       1%
engine:   render       1.530      68%
engine:     sprites    0.001       0%
engine:     text       0.007       0%
engine:     tilemap    1.033      46%
engine:     (rest)     0.488      22%
engine:   present      0.699      31%
engine:   total        2.252     100%
```

L'arithmétique se referme maintenant : `0.001 + 0.007 + 1.033 + 0.488 = 1.529`
contre les 1.530 de la ligne render (la dernière microseconde est l'arrondi des
moyennes par printf). Les `0.488 ms` — **22 % de la frame** — sont du temps
réel que la phase render passe sur des choses qu'aucune ligne de sous-système
ne nomme.

Où passe-t-il ? Deux choses, que les exercices précédents de la leçon ont déjà
rencontrées toutes les deux :

- **L'effacement.** `ClearBuffer` peint les 307 200 pixels de fond avant que
  quoi que ce soit ne dessine dessus — l'exercice 1 de la leçon 046 l'avait
  isolé à ~0,4 ms, et c'est le même 0,488 ms ici (la démo dessine plus
  par-dessus l'effacement, et la sonde de cache a disparu, mais le coût est
  celui de l'effacement).
- **La mesure elle-même.** Deux lectures `platform::Now()` par phase nommée, la
  préparation de la boucle — quelques microsecondes de comptabilité (l'exercice
  1 de la leçon 046 avait trouvé le même résidu à plus petite échelle).

La question que pose la ligne — l'effacement doit-il être un sous-système
nommé ? — a une vraie réponse des deux côtés :

- **Oui :** c'est le plus gros coût pixel par frame du moteur, c'est la ligne
  que le levier « copier moins » de la partie 5 ferait bouger, et un budget qui
  cache 22 % de la frame dans un « (rest) » anonyme n'est pas un budget. Le
  rapport final veut une ligne `clear`.
- **Non, pas comme *sous-système* :** l'effacement n'est pas une chose que le
  jeu fait — c'est une propriété de la stratégie de dessin (tout redessiner, à
  chaque frame). Le nommer comme un sous-système invite à le traiter comme tel.
  La ligne honnête pourrait s'appeler `background` ou `frame setup`, et la
  vérité profonde est qu'un moteur de rendu à rectangles modifiés (dirty
  rectangles) ferait *disparaître* la ligne plutôt que la réduire.

La réponse du cours pour le rapport final (la clôture de la partie 5) : la
ligne sera nommée — `clear` — parce qu'un budget nomme ce qu'il dépense, et
c'est dans le *texte* autour de la ligne que se dit « c'est une stratégie, pas
une fonctionnalité ». Un 22 % anonyme, c'est ainsi que les budgets fabriquent
de la fiction.

*Page traduite de la version anglaise `book/solutions/lesson-058/ex1.md`,
révision `b62f67e`.*

<!-- translation-source: book/solutions/lesson-058/ex1.md @ b62f67e -->
