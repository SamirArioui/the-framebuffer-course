# Solution : exercice 2 — Plus lourd à arrêter

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — Plus lourd à arrêter](../../lessons/part-5/lesson-085-hero-movement.md) de la leçon 085.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-085/ex2.patch}}
```

## Visite guidée

Une constante fait du départ et de l'arrêt des images miroir. Séparez-la et le
héros peut sembler vif au démarrage et pesant à l'arrêt :

```cpp
constexpr double HERO_ACCEL = 0.09; /* eases up to speed quickly */
constexpr double HERO_DECEL = 0.22; /* coasts to a stop */
```

La seule question est laquelle utiliser à chaque frame — le héros accélère-t-il
ou ralentit-il ? Le patch compare les *normes* : le carré de la longueur de
l'intention contre le carré de la longueur de la vitesse actuelle.

```cpp
double tau = (intent2 > move2) ? HERO_ACCEL : HERO_DECEL;
```

- **Accélération** — l'intention est plus rapide que le déplacement actuel du
  héros (pousser depuis le repos vers la pleine vitesse, ou un virage lent vers
  un plus rapide) : utilisez `HERO_ACCEL`, la rapide.
- **Ralentissement** — l'intention est nulle (relâché) ou plus lente que le
  mouvement actuel : utilisez `HERO_DECEL`, la lourde.

Comparer les normes est une règle propre et bon marché, et elle tombe directement
de l'easing qui calcule déjà les deux vecteurs. Ce n'est pas la seule règle
raisonnable — vous pourriez tester si l'intention *s'oppose* à la vitesse (un
produit scalaire) pour attraper un virage serré et le traiter comme un arrêt —
mais pour « départ net, arrêt lourd », le test de norme suffit. Dans un cas
comme dans l'autre, le choix est par frame et ne demande aucun état.

Deux invariants que l'exercice demandait de garder, et le patch les garde : la
**normalisation de la diagonale** est intacte (l'intention est toujours mise à
l'échelle par `1/√2`, donc la diagonale est toujours à la vitesse en ligne
droite — les constantes changent *quand* le héros atteint une vitesse, jamais
*quelle* vitesse), et l'**indépendance à la fréquence de frames** est intacte
(l'easing est toujours `dt / tau`, donc la courbe a la même forme à n'importe
quelle fréquence de frames — les constantes sont en secondes, pas en pas par
frame).

Le ressenti, par les nombres : avec `HERO_ACCEL = 0.09`, le héros atteint la
pleine vitesse environ un tiers plus tôt que le `0.12` symétrique (la courbe
d'accélération est plus serrée), et avec `HERO_DECEL = 0.22`, il met presque
deux fois plus longtemps à s'arrêter (la courbe de décélération est plus longue
— elle roule en roue libre). Lancez-le et maintenez-puis-relâchez : la montée
est visiblement plus courte que la descente. Si la roue libre semble trop
flottante ou le départ trop mou, ces deux nombres sont toute la surface de
réglage — ils sont le poids du héros, et ils sont de la donnée en attente (la
leçon 087 fait entrer ce ressenti dans la ligne de table du héros).

*Page traduite de la version anglaise `book/solutions/lesson-085/ex2.md`,
révision `82879a4`.*

<!-- translation-source: book/solutions/lesson-085/ex2.md @ 82879a4 -->
