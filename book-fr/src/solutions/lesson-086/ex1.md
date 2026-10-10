# Solution : exercice 1 — La rétroaction commence avec l'événement

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — La rétroaction commence avec l'événement](../../lessons/part-5/lesson-086-feedback-animation.md) de la leçon 086.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-086/ex1.patch}}
```

## Visite guidée

La règle du game feel est qu'un effet de ressenti commence **dans la frame où
son événement déclencheur se produit** — le coup et son poids sont un seul
moment, pas un coup maintenant et une secousse un moment plus tard. Le
changement porte donc sur *ce qui déclenche les hooks*, pas sur les hooks
eux-mêmes : le gestionnaire de coup qui baissait déjà la vie du héros déclenche
maintenant aussi un court hitstop et un petit screenshake, juste là.

```cpp
hero.health -= 1;
FeelHitstop(feel, 0.25, 0.1);
FeelShake(feel, 4.0, 0.2);
```

`GameInput` gagne un `Feedback &feel` pour que le coup puisse atteindre les
hooks — l'entrée du jeu connaît déjà l'événement ; il lui faut seulement les
hooks à qui le dire. Le déclenchement-et-repos propre aux hooks est intact : ils
s'écoulent toujours sur leur propre temps mural et reviennent à la pleine
vitesse et à `0,0` tout seuls. Le script de démonstration sur le temps mural de
la leçon est désormais redondant — la boîte à outils déclenche les hooks depuis
les événements, pas depuis une horloge — donc supprimez-le (ou laissez-le et
regardez les deux chemins se déclencher).

Lancez-le et appuyez sur espace : l'exécution se lit

```
engine: hero takes a hit — health 2 (t=2.595)
engine: feel: shake fired ...   <- (with the demonstration removed, only the hit's firing)
engine: feel: hitstop rested — full speed again
engine: feel: shake rested at 0,0
```

Le coup et la rétroaction sont sur la même frame — le ralentissement et la
secousse démarrent avec la propre ligne `hero takes a hit` du coup, pas après
elle. C'est toute la règle : la cause et l'effet se lisent comme un seul moment
parce qu'ils *sont* une seule frame. (Le coup ici est encore le substitut de la
touche espace ; quand le combat arrive à la leçon 087, les deux mêmes lignes
passent de la touche au vrai coup et la règle tient sans changement.)

*Page traduite de la version anglaise `book/solutions/lesson-086/ex1.md`,
révision `4725cee`.*

<!-- translation-source: book/solutions/lesson-086/ex1.md @ 4725cee -->
