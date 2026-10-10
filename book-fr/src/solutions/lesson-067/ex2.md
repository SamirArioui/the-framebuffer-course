# Solution : exercice 2 — L'effet qui ne doit pas s'empiler

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — L'effet qui ne doit pas s'empiler](../../lessons/part-3/lesson-067-effects.md) de la leçon 067.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-067/ex2.patch}}
```

## Visite guidée

Le diff rend la route solo réelle à côté de la route normale.
`MixerPlayEffectSolo` parcourt d'abord les canaux d'effet du pool, à la
recherche du même échantillon **déjà en train de sonner** — l'identité de
l'échantillon est son adresse, et le moteur garde une seule copie de chaque
échantillon dans l'arena, donc la comparaison de pointeurs est toute la
vérification. Si un tel échantillon est trouvé, l'appel répond
`AUDIO_ALREADY_PLAYING`, un nom pour le refus au lieu d'un `-1` silencieux ;
sinon, il délègue à
`MixerPlayEffect` inchangé — même pool, mêmes volumes, même politique de vol.
La barrière est un parcours ; la route reste celle de la leçon.

La sonde pilote les deux routes avec les trois mêmes appuis sur leurs propres
mixeurs, pour que la différence soit la seule variable. Sous la route solo,
l'appui 1 prend le premier canal libre ; les appuis 2 et 3, arrivant pendant
que l'effet sonne encore, sont refusés en le nommant. Puis treize tampons
défilent — douze pour la longueur de l'effet, un pour le tirage qui y met
fin — et l'appui 4 redémarre sur le premier canal libre. Sous
`MixerPlayEffect`, les trois mêmes appuis s'empilent : trois copies en vol en
même temps, sur trois canaux.

Le journal de l'exécution :

```
engine: solo: press 1 -> channel 1
engine: solo: press 2 refused — the sound is already playing
engine: solo: press 3 refused — the sound is already playing
engine: solo: press 4, after the effect's end -> channel 1
engine: stack: press 1 -> channel 1
engine: stack: press 2 -> channel 2
engine: stack: press 3 -> channel 3
```

Deux lignes rapportent les faits de la leçon elle-même. L'appui 4 atterrit sur
le canal 1 — le pool a rendu le canal du premier effet, exactement comme
l'exécution de la leçon l'a montré — et les refus ne coûtent rien : le pool est
intact après un appel refusé, donc le son suivant trouve encore un canal libre.

Puis la question de conception que les deux essais encadrent. L'empilement
n'est pas un bug en soi : trois bruits de pas d'affilée *doivent* se
chevaucher, et la route d'empilement est la bonne pour eux. La route solo est
pour les sons qui sont un événement — une porte qui claque une fois même si le
jeu l'a demandé trois fois, un saut qui sonne comme un seul saut. La route que
prend un son est un fait à propos du son, pas du mixeur, et c'est pourquoi les
deux vivent côte à côte et le choix reste dans la main de l'appelant.

Ce que la sonde ne peut pas trancher : qu'un claquement déclenché trois fois
sonne faux est un jugement d'oreille. Les canaux et les refus sont exacts ici ;
le son de l'empilement est à vous d'entendre.

*Page traduite de la version anglaise `book/solutions/lesson-067/ex2.md`,
révision `b3b0598`.*

<!-- translation-source: book/solutions/lesson-067/ex2.md @ b3b0598 -->
