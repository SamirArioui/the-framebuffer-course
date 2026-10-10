# Solution : exercice 3 — Relisez le remplissage d'un collègue

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 3 — Relisez le remplissage d'un collègue](../../lessons/part-0/lesson-018-optimizer-ub.md) de la leçon 018.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-018/ex3.patch}}
```

## Visite guidée

Le `FillGradient` collé est le bug de la leçon 017 avec un autre chapeau :
`while (i >= 0)` avec `i++` dedans, s'arrêtant seulement quand le compteur
retombe. Confirmer le rapport prend un aller-retour : `-O0` dessine la
rampe, écrit `paint.bmp`, sort 0 ; la construction `-O2` avertit
(`iteration 2147483647 invokes undefined behavior`, pointant `i++` dans
`FillGradient`) et `timeout 5 ./paint` revient avec 124. La correction est le
même remède : une borne de boucle qui est le compte d'octets du tampon —
`for (int i = 0; i < n; i++)` — si bien que la boucle se termine parce que
`i` atteint `n`, ce qu'aucun optimiseur ne peut réinterpréter. Les maths de la
rampe (`(i / 3) * 255 / (w * h - 1)`) ne sont pas touchées ; seul l'arrêt était
cassé. L'appel ajouté après `ClearBuffer`, les deux constructions impriment
désormais des sorties identiques et écrivent le même BMP valide — vérifié avec
un `diff` des deux exécutions et `file` sur le résultat. L'habitude de
relecture à garder : quand quelqu'un dit « même style que » un morceau de code
que vous avez déjà corrigé, relisez la *forme*, pas seulement l'arithmétique.

*Page traduite de la version anglaise `book/solutions/lesson-018/ex3.md`,
révision `2aa5b9e`.*

<!-- translation-source: book/solutions/lesson-018/ex3.md @ 2aa5b9e -->
