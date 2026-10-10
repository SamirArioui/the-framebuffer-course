# Solution : exercice 1 — La respiration entre les vagues

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — La respiration entre les vagues](../../lessons/part-5/lesson-091-waves.md) de la leçon 091.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-091/ex1.patch}}
```

## Visite guidée

L'entracte est un minuteur dans l'état propre du jeu (`wave_wait`) et une
branche dans le combat des vagues : au nettoyage, la vague suivante n'apparaît
pas — le combat l'annonce et se met à respirer ; à chaque frame, la respiration
s'écoule **par le pas de temps de jeu de la frame**, si bien que la pause la
fige et qu'un hitstop la ralentit, comme chaque pas de la simulation. `GameWaves`
est la fonction du combat des vagues ; elle gagne simplement le `dt` de la frame
pour faire le décompte avec.

L'exécution, avec les lignes de vague horodatées par l'horloge de jeu :

```
engine: wave 1 begins — 2 enemies (t=0.000)
engine: wave 1 cleared — wave 2 incoming (t=0.333)
engine: wave 2: spawns bat at 312,232 — speed 20, health 1, chase
...
engine: wave 2 begins — 5 enemies (t=2.662)
engine: wave 2 cleared — wave 3 incoming (t=3.697)
```

Le nettoyage à `t=0.333` annonce la vague 2 ; la vague 2 commence à `t=2.662` —
une respiration de 2.3 secondes, la cible de deux secondes plus la frame dans
laquelle le changement atterrit (le même horizon d'une frame que chaque événement
de ce moteur a). La seconde respiration répète la chose : nettoyée à `3.697`, la
vague suivante apparaissant environ deux secondes plus tard.

**Pourquoi du temps de jeu et pas du temps mural.** La respiration fait partie
de la *simulation* — le rythme du combat — elle doit donc s'étirer quand le jeu
ralentit et s'arrêter quand le jeu s'arrête. Une respiration en temps mural
s'écoulerait pendant une pause : vous reprendriez directement dans la vague 2
sans aucune respiration, et un hitstop (qui existe pour faire atterrir un moment)
ne ferait rien sur elle. Le compteur `wave_wait -= dt` est exactement la règle
que la leçon 078 a posée pour tout ce que fait le monde — et, contrairement aux
hooks de rétroaction de la leçon 086 (qui doivent *se terminer* pendant que le
jeu est arrêté et tournent donc en temps mural), une respiration qui attend
pendant la pause est précisément juste. La séparation des deux horloges est celle
du jeu, et ceci est un minuteur côté simulation sur l'horloge de la simulation.

*Page traduite de la version anglaise `book/solutions/lesson-091/ex1.md`,
révision `af7ec3a`.*

<!-- translation-source: book/solutions/lesson-091/ex1.md @ af7ec3a -->
