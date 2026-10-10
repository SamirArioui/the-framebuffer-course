# Solution : exercice 2 — L'additif qui annule

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — L'additif qui annule](../../lessons/part-2/lesson-054-camera.md) de la leçon 054.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-054/ex2.patch}}
```

## Visite guidée

La prédiction : les dessins ne voient jamais que `base + additive`, donc la
base `(100, 50)` avec l'additif `(−100, −50)` se somme en `(0, 0)` — la scène
doit être exactement identique à la vue sans aucune caméra. La valeur de la
base est invisible depuis le framebuffer ; l'additif l'a effacée. La
vérification à laquelle cela ressemble est celle de la caméra de la leçon 045
(« les mêmes pixels, déplacés »), exécutée à l'envers — au lieu de déplacer la
scène vers un nouveau décalage, les décalages ramènent la scène là où elle a
commencé.

Le patch ajoute le cas — dessiner la vue d'origine et en prendre un
instantané, puis dessiner la caméra qui annule et comparer — et l'exécution
confirme :

```
engine: camera check: additive (-100,-50) cancels the base — 307200 pixels compared, 0 mismatches
```

Les 307 200 pixels de la frame, tous identiques à la vue d'origine. La somme
est tout le contrat : à aucun stade la base n'est « plus réelle » que
l'additif.

D'où l'avertissement de la dernière question de l'énoncé. Si la boîte à outils
du juice peut annuler la base, alors elle peut *remplacer* la vue — une
secousse assez grande, ou avec un biais qui dérive, s'empare de fait de la
caméra du jeu. Ce que la boîte à outils doit promettre, et ce que la séparation
des champs encode, est :

1. **L'additif est transitoire.** Il revient exactement à zéro — toujours,
   sans exception. La base est alors de nouveau la vue, à l'octet près (le
   troisième scénario de la leçon est cette promesse, vérifiée).
2. **L'additif est borné.** Une secousse de ±6 est une rétroaction ; une
   secousse de ±300 est une autre vue. Les amplitudes des effets se choisissent
   par rapport à l'écran, on ne les laisse pas sans borne.
3. **L'additif n'écrit jamais la base.** Pas « en général » — la base est la
   déclaration du jeu sur l'endroit où regarde le joueur, et tout effet qui
   doit *changer* la vue n'est pas un effet, c'est du contrôle de caméra
   (l'affaire des cinématiques, ou le projet final de la partie 5).

Le moteur ne peut pas imposer ces règles — ce sont des contrats, comme ceux de
l'audit de la leçon 042, tenus par le code qui écrit le champ. Mais la
*structure* les rend pensables : une structure, deux champs nommés, une
fonction de sommation et des vérifications qui observent ce que chaque champ
fait aux pixels. C'est ce que l'obligation O2 demandait vraiment.

*Page traduite de la version anglaise `book/solutions/lesson-054/ex2.md`,
révision `b7f3628`.*

<!-- translation-source: book/solutions/lesson-054/ex2.md @ b7f3628 -->
