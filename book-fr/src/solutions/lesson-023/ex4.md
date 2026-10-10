# Solution : exercice 4 — Pourquoi des états, pas des drapeaux

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 4 — Pourquoi des états, pas des drapeaux](../../lessons/part-0/lesson-023-state-machine.md) de la leçon 023.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-023/ex4.patch}}
```

## Visite guidée

Le patch enveloppe chaque changement d'état dans un `SetState` qui journalise
la transition, et l'exécution scriptée de l'exercice 2 imprime toute l'histoire
de la machine :

```
state title -> play
state play -> dead
state dead -> play
state play -> dead
```

Quatre transitions, chacune faite à exactement un endroit, chacune visible.
C'est l'argument pour des états explicites. L'alternative — des drapeaux
`int playing, dead, paused` — *fonctionne* à trois états et pourrit à cinq :
chaque `if` doit demander après chaque drapeau, les combinaisons illégales
(`playing && dead`) deviennent exprimables, et « que se passe-t-il quand le
joueur presse espace ? » devient une table de cas dans la tête de quelqu'un.
Une seule variable `state` ne rend légales que les combinaisons déclarées,
rend les transitions énumérables (comme le montre le journal), et rend la
prochaine fonctionnalité — la pause de l'exercice 3, ou un écran de meilleur
score — une valeur d'énumération et une branche au lieu d'un nouveau drapeau
enfilé à travers le vieux code.

La promenade dans `Render` montre la forme : une branche par état, chacune
dessinant un écran complet. Comparez avec l'idée de répartiteur que le journal
laisse deviner — une fonction par état — et la table de commandes de la leçon
024 est le même motif appliqué à l'entrée : des données au lieu de branches.

*Page traduite de la version anglaise `book/solutions/lesson-023/ex4.md`,
révision `94b8aa1`.*

<!-- translation-source: book/solutions/lesson-023/ex4.md @ 94b8aa1 -->
