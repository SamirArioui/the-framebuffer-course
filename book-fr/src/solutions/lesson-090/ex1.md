# Solution : exercice 1 — Le schéma possède ses attaques

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — Le schéma possède ses attaques](../../lessons/part-5/lesson-090-boss.md) de la leçon 090.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-090/ex1.patch}}
```

## Visite guidée

La ligne d'attaque générique est celle de tout type armé — donc le boss s'en
retire (`e.behavior != BEHAVIOR_BOSS` dans la marche) et son schéma tire pour
lui-même : `AiBoss` gagne le magasin et la table de projectiles, et la phase
`keep` — le temps du maintien et du tir — appelle `CombatAttack` pendant que
les deux autres phases retiennent leur tir. La règle pour tout le monde reste
intacte.

Une exécution avec le boss seul, le héros planté dans sa ligne :

```
engine: boss: golem's pattern -> keep (2 s)
engine: fire: golem -> shell (damage 2, range 400)
engine: hit: shell hits hero — damage 2, health 3 -> 1
engine: boss: golem's pattern -> flee (1 s)
engine: boss: golem's pattern -> chase (3 s)
engine: boss: golem's pattern -> keep (2 s)
engine: fire: golem -> shell (damage 2, range 400)
engine: hit: shell hits hero — damage 2, health 1 -> 0
```

Chaque ligne `fire` tombe dans une phase `keep` et nulle part ailleurs : un
tir par phase de garde (la cadence de la ligne — un shell toutes les trois
secondes — est plus lente que le temps de deux secondes, donc chaque maintien
obtient son tir). La poursuite et la fuite sont silencieuses : le boss se
rapproche et le boss court, et le joueur peut lire quand le canon va se lever.
Le héros meurt au second tir — `damage 2` deux fois, exactement celui de la
ligne — ce qui fait que le combat se lit comme un combat : le schéma
*signifie* désormais quelque chose pour le joueur, pas seulement pour le
mouvement.

Voilà la forme de chaque schéma que ce jeu aura jamais : un planning qui
compose du travail partagé et possède son propre timing. La préparation,
l'enrage, la rafale, ce sont les mêmes deux lignes — une phase qui appelle les
fonctions partagées — et aucune machinerie nouvelle.

*Page traduite de la version anglaise `book/solutions/lesson-090/ex1.md`,
révision `7ce2cfe`.*

<!-- translation-source: book/solutions/lesson-090/ex1.md @ 7ce2cfe -->
