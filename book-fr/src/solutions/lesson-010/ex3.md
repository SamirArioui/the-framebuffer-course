# Solution : exercice 3 — Supprimer au milieu

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 3 — Supprimer au milieu](../../lessons/part-0/lesson-010-void-pointer.md) de la leçon 010.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-010/ex3.patch}}
```

## Visite guidée

`DaRemoveAt` est toute arithmétique et un seul appel de bibliothèque. Les
éléments après la victime doivent glisser d'un pas chacun, et la source du
glissement et sa destination se chevauchent octet par octet au-delà du trou —
ce qui est précisément le cas pour lequel `memmove` existe et sur lequel
`memcpy` ne fait aucune promesse. `memmove` copie comme à travers un
temporaire, aussi le chevauchement est-il sûr ; avec `memcpy` sur un
glissement qui se chevauche, le comportement est indéfini et la corruption est
silencieuse.

La démonstration réutilise les cinq `long` du pilote — triés, donc
`10 20 30 40 50` — et supprime l'indice 2, qui est le `30` :

```
after removing index 2:
10
20
40
50
```

Deux règles de frontière rendent la fonction honnête. Supprimer le *dernier*
élément est un `memmove` de zéro octet qui décrémente seulement `len` —
l'arithmétique `(len - i - 1)` gère cela gratuitement. Et un indice égal ou
au-delà de `len` est rejeté avec un message au lieu de faire glisser des
octets qui n'existent pas. Notez ce qui ne se produit *pas* : aucune
réallocation. `len` rétrécit, `cap` non — la capacité payée dans la leçon
008 reste payée, prête pour le prochain envoi.

*Page traduite de la version anglaise `book/solutions/lesson-010/ex3.md`,
révision `60d447b`.*

<!-- translation-source: book/solutions/lesson-010/ex3.md @ 60d447b -->
