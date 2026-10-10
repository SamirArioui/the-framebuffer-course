# Solution : exercice 1 — Le vingt et unième appui

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — Le vingt et unième appui](../../lessons/part-3/lesson-065-allocation.md) de la leçon 065.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-065/ex1.patch}}
```

## Visite guidée

Le diff est une seule sonde : la séquence comme un script d'étapes — un
démarrage ou une fin — parcourue sur un mixeur d'essai à travers le `MixerPlay`
du moteur lui-même, avec chaque canal d'atterrissage, chaque fin et chaque son
abandonné imprimés. Le tableau `occupant` est la mémoire propre de la sonde :
quel effet détient quel canal ; un appel est rapporté comme un abandon
exactement quand le canal sur lequel il a atterri en détenait encore un. Une fin
est l'état que le mixage laisse à la fin d'un échantillon — le canal devient
inactif — donc la sonde pose le même drapeau que `ChannelFrame` pose à la fin de
l'échantillon, et aucun mixage n'est nécessaire pour libérer un canal.

La prédiction, avant toute exécution. Quinze appuis sur un mixeur neuf prennent
le premier canal libre et le trouvent à 1, 2, 3, et ainsi de suite jusqu'à 15,
estampillés `started` 1 à 15. La fin des effets 4 et 9 libère les canaux 4 et 9,
et le parcours prend le plus bas canal libre d'abord : l'effet 16 atterrit sur 4
et l'effet 17 sur 9, estampillés 16 et 17. L'effet 18 rencontre un pool plein —
le plus petit `started` vaut 1, sur le canal 1 — donc l'effet 1 est abandonné et
le canal 1 redémarre à `started` 18. L'effet 7 se termine, l'effet 19 prend le
canal 7 à 19. L'effet 20 rencontre encore le pool plein : le canal 1 détient
maintenant l'estampille la plus récente du pool, donc le plus petit `started`
est 2 et l'effet 2 tombe du canal 2 à 20. La séquence laisse `mixer.order` à
20 — un par son ayant jamais démarré.

| prédiction | réponse |
| --- | --- |
| les effets 1–15 | canaux 1–15, `started` 1–15 |
| l'effet 16 | canal 4 — premier libre, `started` 16 |
| l'effet 17 | canal 9 — premier libre, `started` 17 |
| l'effet 18 | canal 1, effet 1 abandonné — `started` 18 |
| l'effet 19 | canal 7 — premier libre, `started` 19 |
| l'effet 20 | canal 2, effet 2 abandonné — `started` 20 |
| `mixer.order` une fois la séquence terminée | 20 |
| l'effet 21 (la frontière) | canal 3, effet 3 abandonné — `started` 21 |

Le journal de l'exécution :

```
engine: alloc: effect  1 -> channel  1, order 1
engine: alloc: effect  2 -> channel  2, order 2
engine: alloc: effect  3 -> channel  3, order 3
...
engine: alloc: effect 15 -> channel 15, order 15
engine: alloc: effect  4 ends, channel  4 free
engine: alloc: effect  9 ends, channel  9 free
engine: alloc: effect 16 -> channel  4, order 16
engine: alloc: effect 17 -> channel  9, order 17
engine: alloc: effect 18 -> channel  1 (dropped effect  1), order 18
engine: alloc: effect  7 ends, channel  7 free
engine: alloc: effect 19 -> channel  7, order 19
engine: alloc: effect 20 -> channel  2 (dropped effect  2), order 20
engine: alloc: effect 21 -> channel  3 (dropped effect  3), order 21
```

Chaque ligne se réconcilie avec le parcours et le compteur, et trois détails
justifient cette séquence.

**L'ordre du parcours, pas l'ordre des fins.** Les effets 4 et 9 se sont
terminés dans cet ordre, mais l'effet 16 prend le canal 4 parce que le parcours
va par numéro de canal et s'arrête au premier libre — l'ordre dans lequel les
canaux ont été libérés n'est jamais consulté. Le premier libre est un fait sur
le pool, pas sur le temps.

**Un canal volé est estampillé à nouveau.** Après que l'effet 18 a pris le canal
1, ce canal détient le son le *plus récent* du pool — `started` 18 — donc le
prochain vol ne peut pas y atterrir de nouveau : l'effet 20 va au canal 2, le
suivant en ancienneté. Le compteur avance dans une seule direction et chaque
démarrage prend le numéro suivant, ce qui garde un pool occupé coupant à peu
près dans l'ordre des démarrages plutôt que de marteler un seul canal.

**Les fins ne vieillissent pas un canal.** Les canaux 4, 9 et 7 ont été libérés
puis remplis de nouveau pendant la séquence, et ils détiennent `started` 16, 17
et 19 — aussi récents que les sons qui viennent d'arriver. Seuls les démarrages
estampillent ; les fins ne font que libérer. « Le plus ancien » est un fait sur
le son qui joue maintenant, et rien sur ce qui a pu jouer avant sur ce canal.

La frontière est la même relation un appui plus tard. L'effet 21 rencontre un
pool plein et abandonne l'effet 3 — le troisième son de l'exécution, celui qui a
été entendu pendant toute la séquence — atterrissant sur le canal 3 à `started`
21. Et le canal 0 n'apparaît nulle part dans le journal : il n'est jamais
proposé à un effet ni jamais candidat au vol, même s'il ne détient aucun son du
tout. Avec de la musique dessus (la leçon 066), chaque ligne ci-dessus serait
identique.

Ce que la sonde ne peut pas cacher sur elle-même : elle marque les fins
directement plutôt que d'épuiser un échantillon — le même état, atteint à la
main — et chaque effet est la même tonalité, donc le journal ne dit rien de ce
que tout cela sonne. Cette question se répond en canaux et en compteurs, et
ceux-là, la sonde les montre exactement.

*Page traduite de la version anglaise `book/solutions/lesson-065/ex1.md`,
révision `74a32e1`.*

<!-- translation-source: book/solutions/lesson-065/ex1.md @ 74a32e1 -->
