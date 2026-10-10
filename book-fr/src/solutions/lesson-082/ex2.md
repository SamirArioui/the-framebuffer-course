# Solution : exercice 2 — Pourquoi l'échelle

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — Pourquoi l'échelle](../../lessons/part-5/lesson-082-skeleton.md) de la leçon 082.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-082/ex2.patch}}
```

## Visite guidée

Le diff est un champ d'instrumentation : le journal de frames imprime l'échelle
du temps de jeu à côté du pas qu'elle a produit. Il rend la réponse visible — le
gel n'est pas dans la boucle qui saute du travail, il est dans un nombre.

L'exécution, depuis l'état final de cette leçon plus le patch — le titre, une
frame de jeu, puis la pause :

```
frame 1: scale 0.00, step 0.000 ms, update 0.001 ms (entities 0.001), … tilemap 0.000), …
frame 3: scale 1.00, step 20.362 ms, update 0.002 ms (entities 0.002), … tilemap 0.992), …
frame 4: scale 0.00, step 0.000 ms, update 0.012 ms (entities 0.002), … tilemap 0.000), …
```

**Pourquoi l'échelle et non un update sauté.** `step` est le temps de jeu — le
pas de l'horloge murale mis à l'échelle (leçon 078). À `scale 0.00`, le pas vaut
`0.000` : le monde n'a rien avancé. Mais regardez `update 0.012 ms` sur cette
même frame en pause. L'update *a tourné*. Il a lu l'entrée, mis à jour la
requête du héros, chronométré la marche, imprimé ses rapports — il a coûté du
vrai temps mural et avancé le monde de zéro seconde de jeu. Le gel vit dans le
*pas*, pas dans la boucle qui refuse d'appeler l'update. Supposez maintenant que
la boucle fasse plutôt `if (paused) continue;` avant l'update :

- **L'enregistrement de frame perdrait une phase.** Le contrat de la leçon 079
  est que l'enregistrement mesure l'horloge murale et n'est *pas* mis à l'échelle
  — une frame en pause est encore une frame, et le compte lui doit encore un
  nombre `update`. Sauter l'appel laisse la ligne vide ; mettre le pas à
  l'échelle garde la ligne honnête et montre le jeu immobile dans `step`, pas
  dans `update`. La paire ci-dessus — `update 0.012`, `step 0.000` — en est la
  preuve : la machine a travaillé, le monde non.
- **La mémorisation d'entrée mourrait de faim.** `KeyPressed` est un front que
  la pompe à événements pose et que l'update consomme. Si l'update ne tourne pas
  sur une frame en pause, l'appui arrivé pendant la pause est soit perdu, soit
  mémorisé sans personne pour le lire ; la touche de pause agirait deux fois ou
  jamais. C'est l'update qui tourne à chaque frame qui permet à Escape de
  reprendre proprement.
- **Une échelle fractionnaire serait impossible.** Le hitstop de la leçon 092
  est le même bouton à une fraction — `scale 0.25` ralentit le monde, il ne le
  fige pas. Un `continue` est binaire : marche ou arrêt. Il n'y a pas d'update
  « à moitié sauté ». L'échelle est un seul nombre qui couvre déjà la pause (0),
  le hitstop (une fraction) et le jeu (1) ; construire le gel comme un cas
  particulier jetterait le seul bouton dont la boîte à outils du juice a besoin.

**La question de mesure.** `update` coûtant du vrai temps pendant que `step` est
à zéro prouve que le gel est *en aval* de l'update — dans le pas par lequel il
avance le monde — et non dans l'update sauté. Si le gel était un update sauté,
`update` afficherait `0.000` lui aussi (il n'y aurait rien à mesurer). Ce n'est
pas le cas : il affiche `0.012 ms`. L'update tourne ; `GameTimeStep` multiplie
son `dt` par l'échelle, et l'échelle est zéro, donc le monde ne bouge pas.
C'est tout le mécanisme, et les deux colonnes du journal — `scale 0.00`,
`update 0.012` — le sont, mesurées plutôt qu'argumentées.

Rien ici ne change la machine : le patch ajoute seulement l'échelle à la ligne
que le journal de frames imprime déjà. La réponse était dans l'enregistrement
depuis le début ; ceci en fait une colonne.

*Page traduite de la version anglaise `book/solutions/lesson-082/ex2.md`,
révision `a4635cd`.*

<!-- translation-source: book/solutions/lesson-082/ex2.md @ a4635cd -->
