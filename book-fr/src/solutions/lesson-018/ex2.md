# Solution : exercice 2 — La garde qui est supprimée

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — La garde qui est supprimée](../../lessons/part-0/lesson-018-optimizer-ub.md) de la leçon 018.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-018/ex2.patch}}
```

## Visite guidée

La boucle réécrite a un arrêt explicite — `if (i < 0) break;` — et
l'exécution `-O0` l'honore : `clear ended at offset -2147483648`, sortie 0.
L'exécution `-O2` l'ignore entièrement : `timeout 5 ./paint` n'imprime rien et
`echo $?` vaut 124. Même blocage, garde comprise. La raison est toute la thèse
de la leçon en une ligne : `i` commence à 0 et la boucle ne fait jamais que
`i++`, aussi `i` ne peut devenir négatif *que* par débordement signé — et le
débordement signé est indéfini, aussi le compilateur a-t-il le droit de
supposer qu'il n'arrive jamais. Sous cette hypothèse `i < 0` est inatteignable
et le `break` est du code mort, que l'optimiseur supprime avant que quoi que
ce soit ne tourne. L'autorité de la norme, pas l'humeur de gcc : tout
compilateur conforme peut faire cela. Ce qui aurait sauvé la boucle est un
arrêt qui existe dans le comportement défini — une vraie borne comme
`i < nbytes` (la correction de la leçon), ou une condition de sortie qu'aucune
hypothèse ne peut effacer. Les gardes contre l'« impossible » sont exactement
le code que l'hypothèse de l'impossible emporte avec elle.

*Page traduite de la version anglaise `book/solutions/lesson-018/ex2.md`,
révision `2aa5b9e`.*

<!-- translation-source: book/solutions/lesson-018/ex2.md @ 2aa5b9e -->
