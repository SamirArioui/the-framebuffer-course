# Solution : exercice 1 — WASD, c'est quatre lignes

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — WASD, c'est quatre lignes](../../lessons/part-0/lesson-024-command-table.md) de la leçon 024.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-024/ex1.patch}}
```

## Visite guidée

Quatre lignes, aucune branche. `w`, `a`, `s`, `d` sont des touches ordinaires
dont les octets sont leurs codes, aussi chacune est-elle une entrée de table
liant l'octet à la *même* fonction de commande qu'utilisent les flèches —
`CmdUp`, `CmdLeft`, `CmdDown`, `CmdRight`. Le parseur, le balayage de
répartition, la garde d'état, et la règle de retournement dans `CmdTurn` sont
tous intacts : les commandes étaient déjà complètes ; seules les liaisons
manquaient.

Une tournée minutée le prouve —
`( printf ' w'; sleep 0.4; printf 'a'; sleep 0.4; printf 's'; sleep 0.4;
printf 'dq' ) | ./snek 60` — et la trace narre la promenade : `dir=up`, puis
`dir=left`, puis `dir=down`, puis `dir=right` dans les dernières frames avant
que `q` ne quitte (`done after 37 frames` dans une exécution, 38 dans une
autre — l'ordonnancement du tube, comme dans la leçon 021). Chaque virage est
perpendiculaire au précédent, aussi la garde de retournement ne se
déclenche-t-elle jamais ; essayez `w` puis `s` directement et vous verrez la
garde rejeter la seconde touche.

C'est tout l'argument de la leçon. Dans l'ancienne répartition par branches,
les touches de mots auraient signifié quatre bras `else if` de plus dans
`OnByte` ; dans la table elles signifient quatre lignes — des données — et le
code de répartition est exactement aussi long qu'avant que la fonctionnalité
n'existe.

*Page traduite de la version anglaise `book/solutions/lesson-024/ex1.md`,
révision `bd716a9`.*

<!-- translation-source: book/solutions/lesson-024/ex1.md @ bd716a9 -->
