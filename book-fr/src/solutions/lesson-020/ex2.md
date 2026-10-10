# Solution : exercice 2 — Ce que coûte le plafond de frames

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — Ce que coûte le plafond de frames](../../lessons/part-0/lesson-020-timing.md) de la leçon 020.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-020/ex2.patch}}
```

## Visite guidée

Mettre le budget de frames à zéro rend le reste négatif à chaque frame, aussi
le sommeil ne se déclenche-t-il jamais — le plafond est éteint, et le
changement d'une ligne garde `SleepSec` référencée pour que `-Wextra` reste
tranquille. Sur une machine les nombres mesurés étaient : `./snek 100` prend
3,35 s avec plafond et 0,001 s sans — 100 frames passent en environ une
milliseconde quand rien ne les retient. C'est le plafond qui fait son travail :
il transforme les itérations de boucle en *temps*.

Les comptes de ticks font le point plus profond. Sans plafond,
`./snek 100000` finit en 0,052 s et rapporte `0 ticks`, et
`./snek 2000000` — deux millions de frames — finit en 0,9 s et rapporte
`9 ticks`, les mêmes neuf ticks qu'une exécution à 30 fps collecte en une
seconde. Le temps de jeu est le temps réel : l'accumulateur convertit les
secondes *écoulées* en ticks, si bien que le compte de frames et le compte de
ticks sont complètement découplés. C'est ce qui permet au serpent de bouger à
la même vitesse sur une machine rapide et sur une lente. Comparez aussi `dt`
dans les deux exécutions : avec plafond il se tient à 0,0334, sans plafond il
s'effondre à des valeurs de microsecondes — la machine n'est plus ce qui
rythme la boucle.

*Page traduite de la version anglaise `book/solutions/lesson-020/ex2.md`,
révision `203c219`.*

<!-- translation-source: book/solutions/lesson-020/ex2.md @ 203c219 -->
