# Solution : exercice 2 — La chronologie du planning

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — La chronologie du planning](../../lessons/part-5/lesson-090-boss.md) de la leçon 090.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-090/ex2.patch}}
```

## Visite guidée

**La prédiction.** Le schéma est poursuite (3 s) → garde (2 s) → fuite (1 s) →
répétition, à partir du démarrage, sur l'horloge du jeu. Donc depuis le début
de la partie :

| temps de jeu | Phase |
| ------------ | ----- |
| 0 – 3 s | poursuite |
| 3 – 5 s | garde de distance |
| 5 – 6 s | fuite |
| 6 – 9 s | poursuite |
| 9 – 11 s | garde de distance |
| 11 – 12 s | fuite |
| 12 – 15 s | poursuite |

Les *rapports* de phase se déclenchent quand une phase commence, donc les
quinze premières secondes de jeu devraient annoncer `keep` à t≈3, `flee` à
t≈5, `chase` à t≈6, `keep` à t≈9, `flee` à t≈11, `chase` à t≈12 — les phases
de poursuite surtout silencieuses (le schéma a commencé sur l'une d'elles), les
phases de garde de distance et de fuite s'annonçant toutes les six secondes de
cycle.

La courbe de distance sur un cycle complet : **en baisse** pendant la
poursuite (le boss se rapproche du héros), **en hausse vers la distance de
garde** pendant la garde (il recule jusqu'à son 160 et s'y tient), **en hausse
plus vite** pendant la fuite — puis de nouveau en baisse. En dents de scie,
avec un sommet plat.

**La sonde horodate la chronologie** avec l'horloge de jeu propre au schéma
(le `t=` ici est le temps de jeu que le schéma a vu, pas le temps mural) :

```
engine: boss: golem's pattern -> keep (2 s) at t=3.0
engine: boss: golem's pattern -> flee (1 s) at t=5.0
engine: boss: golem's pattern -> chase (3 s) at t=6.1
engine: boss: golem's pattern -> keep (2 s) at t=9.1
```

`keep at 3.0` — la première poursuite a couru ses 3 s pleines. `flee at 5.0` —
la garde a couru ses 2. `chase at 6.1` — la seconde de la fuite (le 0.1 est la
frame où le changement atterrit : le planning s'aperçoit quand le pas d'une
frame franchit la ligne, donc les événements atterrissent sur des frontières
de frames). `keep at 9.1` — les 3 s de la poursuite encore. La table de la
prédiction correspond, avec une gigue d'exactement une frame là où le pas
franchit la ligne.

**La mise en garde sur la frame longue** explique pourquoi le schéma tourne
sur le temps de jeu et compte *en montant* : la première frame de l'exécution
sans écran peut porter un pas de plusieurs secondes (celle de cette exécution
l'a fait), et un planning à rebours initialisé à zéro aurait sauté sa première
phase ou avancé au mauvais moment. Compter le temps de phase écoulé contre la
durée de la phase fait du planning une fonction du temps de jeu seul — le même
temps de jeu que tout le reste de la simulation utilise — et la pause le fige
aussi.

*Page traduite de la version anglaise `book/solutions/lesson-090/ex2.md`,
révision `7ce2cfe`.*

<!-- translation-source: book/solutions/lesson-090/ex2.md @ 7ce2cfe -->
