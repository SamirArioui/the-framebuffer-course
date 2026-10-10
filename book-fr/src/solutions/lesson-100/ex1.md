# Solution : exercice 1 — Le prix d'un pixel

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — Le prix d'un pixel](../../lessons/part-5/lesson-100-clear.md) de la leçon 100.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-100/ex1.patch}}
```

## Visite guidée

Le banc est le banc d'essai de sprites de la leçon 048 tourné vers
le clear : trois tailles sur 200 effacements chacune, chronométrées
sur le vrai tampon au démarrage, pour que le prix par pixel apparaisse à
part de tout le reste que fait la frame. Et parce que les fichiers qu'il
touche (`report.*`, `main.cpp`) ne sont pas touchés par l'étape de code
de cette leçon, le même patch s'applique à `lesson-099` — le nombre
d'avant est mesuré par le *même* instrument, pas emprunté à la table de
la leçon.

À `lesson-099` (la boucle de stockages d'octets) :

```
engine: bench: clear 640x480 — 1.2 ns/pixel over 200 clears
engine: bench: clear 320x240 — 1.2 ns/pixel over 200 clears
engine: bench: clear 160x120 — 1.3 ns/pixel over 200 clears
```

À `lesson-100` (le remplissage par mot) :

```
engine: bench: clear 640x480 — 0.5 ns/pixel over 200 clears
engine: bench: clear 320x240 — 0.5 ns/pixel over 200 clears
engine: bench: clear 160x120 — 0.5 ns/pixel over 200 clears
```

**1.2 → 0.5 ns par pixel : une chute de 2,4×**, constante à travers les
tailles (le prix par pixel est celui de la boucle, pas du tampon).

Maintenant la décomposition que demande l'exercice. La prédiction par
comptage d'instructions était : quatre stockages d'octets plus un calcul
d'adresse par pixel deviennent un stockage de mot — un facteur d'environ
2,5-3 en `-O0` une fois comptée la comptabilité de boucle restante.
Mesuré : 2,4×. Donc :

- **Le compte de stockages possède la chute** — quatre stockages et
  l'arithmétique d'adresse par pixel disparus, c'est le gros des
  0.7 ns/pixel gagnés.
- **La comptabilité de boucle de `-O0` possède ce qui reste** — la
  comparaison d'indice, l'incrément, le trafic de pile que garde le
  compilateur non optimisé : le plancher de `0.5 ns/pixel` ici. Le
  recensement de la leçon montre ce qu'une vraie compilation fait de la
  même boucle — huit pixels par tour de stockage large — donc en `-O3` ce
  nombre baisse encore, et c'est la *forme* du correctif qui survit au
  drapeau, pas la milliseconde.

Une nuance à emporter dans chaque micro-banc que vous écrirez jamais :
ces nombres sont **à cache chaud** — 200 effacements consécutifs laissent
le tampon dans le cache entre les passes — tandis que la ligne `clear` du
compte de frame mesure le clear contre un tampon que la frame vient
de dessiner et que la couture vient de lire (plutôt froid). C'est
pourquoi le banc lit `0.5` là où la ligne de la frame implique
`0.78 ns/pixel` après le correctif (et `1.2` contre `1.48` avant). Aucun
des deux nombres n'est faux ; un banc chiffre la boucle, la frame chiffre
le jeu. Quand vous en citez un, nommez l'autre.

*Page traduite de la version anglaise `book/solutions/lesson-100/ex1.md`,
révision `72d9add`.*

<!-- translation-source: book/solutions/lesson-100/ex1.md @ 72d9add -->
