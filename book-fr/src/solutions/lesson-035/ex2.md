# Solution : exercice 2 — L'horloge qui ment

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — L'horloge qui ment](../../lessons/part-1/lesson-035-clock.md) de la leçon 035.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-035/ex2.patch}}
```

## Visite guidée

L'exercice « deux horloges » de la leçon 020, revisité derrière la couture.
L'instrument ajoute la seconde horloge — `CLOCK_REALTIME`, l'horloge murale —
à côté de la monotone, et l'auto-contrôle imprime les deux :

```
engine: clock never backwards over 100000 samples, finest step 20 ns
engine: monotonic 37725.274523 vs wall 1791253957.846222
```

Les nombres racontent déjà deux histoires différentes. La lecture monotone
vaut 37 725 secondes — un peu plus de dix heures, soit le temps depuis lequel
cette machine tourne ; l'horloge compte depuis un point de départ arbitraire,
et rien d'autre. La lecture murale vaut 1 791 253 957 secondes — des secondes
depuis le 1970-01-01, le nombre dont l'OS se sert pour vous dire quelle heure
il est.

Maintenant l'explication, dans les termes du moteur. Le pas d'une frame est
`dt = now − last`. Avec l'horloge monotone, `dt` est toujours le vrai temps
écoulé, parce que rien ne peut faire bouger l'horloge sinon le temps qui
passe — les 100 000 échantillons de l'auto-contrôle n'ont jamais reculé, et
ni un administrateur, ni une synchronisation NTP, ni un changement d'heure
d'été ne peut les faire reculer.

Avec l'horloge murale, `dt` hérite de chaque ajustement que le système
effectue :

- **l'horloge recule** (correction NTP, changement de date manuel) — `dt`
  devient *négatif*, et le marqueur recule à 240 pixels par seconde ;
- **l'horloge avance** — `dt` fait soudain des minutes de large, et une seule
  frame fait traverser tout l'écran au marqueur ;
- **les deux** sont muets : le moteur ne peut pas distinguer un ajustement
  d'une frame.

Rien de tout cela n'est hypothétique — la leçon 020 a vu une horloge murale
contredire une monotone sur cette machine même. L'horloge monotone n'est pas
une préférence ; c'est la seule des deux dont les différences sont des
*durées*. (L'horloge murale reste la bonne horloge pour les horodatages qu'un
humain lira — une question différente de « combien de temps cette frame
a-t-elle pris ».)

*Page traduite de la version anglaise `book/solutions/lesson-035/ex2.md`,
révision `9b3ec3d`.*

<!-- translation-source: book/solutions/lesson-035/ex2.md @ 9b3ec3d -->
