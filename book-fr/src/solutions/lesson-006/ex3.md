# Solution : exercice 3 — Trois sorts

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 3 — Trois sorts](../../lessons/part-0/lesson-006-undefined-behavior.md) de la leçon 006.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-006/ex3.patch}}
```

## Visite guidée

Le tri. *Indéfini* : le débordement signé (le `++big` de l'exercice 1) et
l'accès hors limites (la lecture à l'envers de l'exercice 2) — la norme
annule son contrat entièrement. *Défini mais faux* : le retournement
`size_t` — le doublement `buf->cap * 2` est spécifié pour revenir en boucle
sur un petit nombre ; le résultat est du non-sens sans qu'aucune règle ne
soit violée. *Indéterminé* : les lectures non initialisées, comme les locales
camelotes de la leçon 003.

Les trois vérifications se mappent sur les deux premiers sorts et un bug
voisin. `BufferAt` défend contre les accès hors limites : l'indice est testé
contre l'allocation avant l'accès, aussi le comportement indéfini devient-il
une erreur nommée et une sortie propre. La vérification de croissance défend
contre le retournement défini-mais-faux : elle calcule d'abord la valeur
doublée (le retournement, s'il y en a un, a déjà eu lieu — l'arithmétique non
signée est ainsi) et rejette `new_cap < buf->cap`. La vérification de `NULL`
sur `realloc` ne garde aucun comportement indéfini du tout — elle garde le
motif classique `p = realloc(p, n)` où l'échec perd l'ancien pointeur. Le sort
indéterminé est combattu par l'initialisation au lieu d'une vérification
(`BufferInit` met à zéro, `last` commence à `?`).

« Ça a marché quand je l'ai testé » ne répond à rien de tout cela : cela
enregistre le comportement d'une construction à des tailles que vous avez eu
l'occasion d'essayer. L'instrument montre les vérifications à l'œuvre à
chaque croissance :

```
checked growth: 0 -> 64
checked growth: 64 -> 128
...
checked growth: 4096 -> 8192
```

*Page traduite de la version anglaise `book/solutions/lesson-006/ex3.md`,
révision `51e4e75`.*

<!-- translation-source: book/solutions/lesson-006/ex3.md @ 51e4e75 -->
