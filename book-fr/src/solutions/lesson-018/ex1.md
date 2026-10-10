# Solution : exercice 1 — Prédisez les dégâts

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — Prédisez les dégâts](../../lessons/part-0/lesson-018-optimizer-ub.md) de la leçon 018.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-018/ex1.patch}}
```

## Visite guidée

Le patch restaure le remplissage de la leçon 017 — la boucle qui s'arrête
quand son décalage devient négatif — et ajoute un `fprintf` à son entrée. Les
deux prédictions se jouent. La construction `-O2` incrimine l'incrément de
`ClearBuffer` :

```
warning: iteration 2147483647 invokes undefined behavior
[-Waggressive-loop-optimizations]
   162 |         i++;
```

et les exécutions se séparent exactement comme attendu. `-O0` imprime le
marqueur, puis `clear ended at offset -2147483648`, puis écrit le fichier et
sort 0. `timeout 5 ./paint` à `-O2` n'imprime *que* le marqueur —
`clear: entering the naive fill` — et `echo $?` dit `124` : l'exécution est
entrée dans le remplissage et n'en est jamais revenue. Le marqueur est le
changement confirmant qui compte : il prouve que le blocage est *à
l'intérieur* de la boucle, pas avant. La valeur retombée `-2147483648` dans
la sortie `-O0` est tout le diagnostic en un nombre — la boucle ne s'est
terminée que parce que le compteur a débordé, ce qui est précisément ce
qu'`-O2` a le droit de supposer impossible.

*Page traduite de la version anglaise `book/solutions/lesson-018/ex1.md`,
révision `2aa5b9e`.*

<!-- translation-source: book/solutions/lesson-018/ex1.md @ 2aa5b9e -->
