# Solution : exercice 2 — Le plan des vagues

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — Le plan des vagues](../../lessons/part-5/lesson-091-waves.md) de la leçon 091.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-091/ex2.patch}}
```

## Visite guidée

**La prédiction, à partir des lignes seules.** L'effectif dit :

| Type | `wave` | `count` |
| ---- | ------ | ------- |
| bat | 1 | 2 |
| wisp | 2 | 1 |
| spitter | 2 | 2 |
| golem | 3 | 1 |

et une vague fait apparaître chaque type dont la `wave` est arrivée — cette vague
et toutes les suivantes. Donc :

- **La vague 1** amène les bats : **2 ennemis**.
- **La vague 2** ramène les bats, plus le wisp et les spitters : 2 + 1 + 2 =
  **5 ennemis**.
- **La vague 3** amène tout, y compris le golem : 2 + 1 + 2 + 1 = **6 ennemis**
  — les trois types et le boss ensemble sur le terrain. C'est la vague autour de
  laquelle le jeu est construit : la dernière étape est tout l'effectif d'un
  coup, et le boss n'est pas une rencontre en solo mais le couronnement d'un
  champ de bataille déjà plein.

La sonde imprime exactement ce plan avant que quoi que ce soit n'apparaisse :

```
engine: plan: wave 1 brings bat x2
engine: plan: wave 1 total 2
engine: plan: wave 2 brings bat x2
engine: plan: wave 2 brings wisp x1
engine: plan: wave 2 brings spitter x2
engine: plan: wave 2 total 5
engine: plan: wave 3 brings bat x2
engine: plan: wave 3 brings wisp x1
engine: plan: wave 3 brings spitter x2
engine: plan: wave 3 brings golem x1
engine: plan: wave 3 total 6
```

et les exécutions de la leçon font apparaître précisément ceux-là : `wave 1
begins — 2 enemies`, `wave 2 begins — 5 enemies`, `wave 3 begins — 6 enemies`.

**Bougez une ligne.** Changez la `wave` du golem de `3` à `1` et le plan
s'inverse : la vague 1 devient le boss *et* son escorte (3 ennemis : golem ×1,
bat ×2), et la vague 3 n'est plus que les bats, le wisp et les spitters — le
climax arrive en premier. C'est tout l'intérêt de la composition qui vit dans les
lignes : la forme de la confrontation est un fait difféable et modifiable, et le
code qui la combat ne change jamais. (La sonde ci-dessus est une mesure jetable —
le vrai combat imprime les apparitions, pas le plan.)

*Page traduite de la version anglaise `book/solutions/lesson-091/ex2.md`,
révision `af7ec3a`.*

<!-- translation-source: book/solutions/lesson-091/ex2.md @ af7ec3a -->
