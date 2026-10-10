# Solution : exercice 2 — La colonne de la pire frame

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — La colonne de la pire frame](../../lessons/part-2/lesson-058-budget.md) de la leçon 058.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-058/ex2.patch}}
```

## Visite guidée

D'abord la prédiction, à partir du journal. L'en-tête nomme la pire frame
(`worst 4.162 ms (frame 17)` dans l'exécution de la leçon, `worst 2.768 ms
(frame 9)` dans celle-ci) ; trouvez la ligne de cette frame et lisez ses
phases. Dans cette exécution, la frame 9 :

```
frame 9: update 0.028 ms, render 1.317 ms (sprites 0.001, text 0.007, tilemap 0.887), present 1.422 ms, total 2.768 ms
```

Le render n'a rien de remarquable — 1,317 ms contre une moyenne de 1,5 ms, en
fait *en dessous*. La pire frame est un **événement de présentation** :
`present 1.422 ms`, plus du double de la moyenne de 0,699 ms. La prédiction à
écrire avant de vérifier : la pire frame n'est pas celle qui a le plus dessiné ;
c'est celle dont la copie vers la fenêtre a été la plus lente.

Le patch conserve l'enregistrement de la pire frame (`if (frame.total >
worst_frame.total) worst_frame = frame;`) et imprime son attribution sous la
table :

```
engine: frame budget — 26 frames, avg 2.230 ms, worst 2.768 ms (frame 9)
engine:   worst frame 9: update 0.028, render 1.317 (sprites 0.001, text 0.007, tilemap 0.887), present 1.422
```

Exactement la prédiction. Maintenant, *pourquoi* la forme de la pire frame
diffère de celle de la moyenne — trois raisons, toutes visibles dans l'histoire
de ce moteur :

- **Un travail différent, pas seulement plus de travail.** La frame moyenne est
  dominée par le render (le parcours du tilemap à chaque frame) ; la pire frame
  est dominée par le present. Les moyennes brouillent les phases entre elles ;
  la pire frame est un moment unique où une phase s'est mal comportée. C'est
  pourquoi le budget garde les phases séparées.
- **Les premières fois coûtent cher.** Les pires frames se regroupent au début
  (la leçon 036 de la partie 1 trouvait la frame 1 pire : premiers contacts
  avec le framebuffer, caches froids, premier vrai present de la fenêtre) — et
  à tout moment où le système rencontre du nouveau : une fenêtre
  redimensionnée, une région fraîchement défilée, l'OS qui replanifie le
  processus ailleurs.
- **Le present n'est pas à nous.** Le contrat de la leçon 031 rend le present
  synchrone — la copie est terminée quand il rend la main — donc son pire cas
  inclut tout ce que le système de fenêtres était en train de faire. Le moteur
  peut rendre ses propres phases prévisibles ; la queue du present appartient à
  la machine.

L'habitude que l'exercice vise : **ne jamais lire la moyenne d'un budget sans
sa ligne de pire frame.** Un jeu qui tourne à 2,2 ms de moyenne avec une pire
frame à 2,8 ms est un autre produit qu'un jeu à 40 ms de pire frame avec la
même moyenne — le joueur ressent la queue, pas la moyenne. Le rapport de budget
de frames de la partie 5 gardera les deux colonnes, et l'étape « corriger les
2 points chauds » du menu en trois passes lit d'abord les moyennes et les pires
frames immédiatement après.

*Page traduite de la version anglaise `book/solutions/lesson-058/ex2.md`,
révision `b62f67e`.*

<!-- translation-source: book/solutions/lesson-058/ex2.md @ b62f67e -->
