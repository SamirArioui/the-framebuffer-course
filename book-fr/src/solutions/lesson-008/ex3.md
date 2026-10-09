# Solution : exercice 3 — Doubler contre un-à-la-fois

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 3 — Doubler contre un-à-la-fois](../../lessons/part-0/lesson-008-dynarray.md) de la leçon 008.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-008/ex3.patch}}
```

## Visite guidée

Les compteurs — `reallocs`, `model_elems` (éléments que la règle de
croissance fait re-stocker au tableau), `moved_elems` (éléments réellement
copiés quand un bloc a bougé) — racontent deux histoires différentes à la
fois. Mesurés sur la machine de l'auteur :

```
GROW_BY_ONE=0 N=1000000   reallocs=19        model_elems=1048572        moved_elems=1044480    real 0.01s
GROW_BY_ONE=1 N=1000000   reallocs=1000000   model_elems=499999500000   moved_elems=16374177   real 0.01s
GROW_BY_ONE=0 N=5000000   reallocs=22        model_elems=8388604        moved_elems=8384512    real 0.06s
GROW_BY_ONE=1 N=5000000   reallocs=5000000   model_elems=12499997500000 moved_elems=8715529    real 0.06s
```

Le verdict du modèle est brutal — un demi-billion d'éléments contre un
million, sept ordres de grandeur. Les horloges pourtant, ne le voient pas du
tout. Où est passée la copie quadratique ? Pas dans `moved_elems` : seulement
16 millions d'éléments ont réellement bougé. L'allocation esquive le modèle —
quand de l'espace libre suit le bloc, `realloc` l'étend sur place et ne copie
rien (la vérification de la solution de l'exercice 4 rend cela visible). Sur
un tas neuf, la croissance est presque gratuite quel que soit le calendrier.

Ne concluez pas que le calendrier n'a pas d'importance. L'échappatoire sur
place est un *aubaine*, pas un contrat : sur un tas fragmenté, ou avec une
autre allocation vivante derrière la vôtre, la croissance doit déplacer — et
là `model_elems` devient de la vraie copie. Le doublement borne ce travail
sous toutes les formes de tas (jamais plus de 2n éléments) ; la croissance
d'un emplacement parie tout sur la chance. Le seul compte d'appels à
`realloc` — 19 contre 1 000 000 — est un coût mesuré que vous payez dans les
deux cas.

*Page traduite de la version anglaise `book/solutions/lesson-008/ex3.md`,
révision `f4168c6`.*

<!-- translation-source: book/solutions/lesson-008/ex3.md @ f4168c6 -->
