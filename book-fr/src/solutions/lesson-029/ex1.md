# Solution : exercice 1 — La fenêtre qu'on ouvre deux fois

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — La fenêtre qu'on ouvre deux fois](../../lessons/part-1/lesson-029-clean-close.md) de la leçon 029.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-029/ex1.patch}}
```

## Visite guidée

Le bug d'abord. La couche plateforme garde l'état d'une seule fenêtre dans le
stockage statique, et `OpenWindow` distribue `&window_state` à quiconque le
demande. Un second appel alors que la première fenêtre est ouverte n'échoue
pas — il *écrase* : le handle de la première connexion au display est perdu
sans être fermé, et à partir de là `CloseWindow` nettoie la seconde fenêtre
tandis que la connexion de la première fuit pour toute la vie du processus.
Rien ne plante, et c'est exactement pourquoi c'est un bug qui mérite d'être
prouvé avant d'être corrigé.

La correction, c'est la règle de propriété, désormais appliquée : une fenêtre à
la fois. La première chose que fait `OpenWindow` est de vérifier si son état
est déjà pris, et si oui il retourne une panne typée — `OPEN_ALREADY_OPEN`,
ajoutée à l'`enum` comme toutes les autres raisons de refus de la couture.
Aucune ressource n'est touchée sur ce chemin, donc rien n'a besoin d'être
libéré : la panne arrive avant l'acquisition.

La preuve, dans le moteur :

```
$ DISPLAY=:99 ./build/game &
$ DISPLAY=:99 xdotool windowclose <id>
engine: window 640x480 open — waiting for news
engine: second open refused (platform error 3)
engine: close reported
engine: closed
```

La première fenêtre s'ouvre et se ferme normalement ; le second appel rapporte
la raison 3 — `OPEN_ALREADY_OPEN` — et ne prend rien. Le `switch` d'erreur du
moteur a gagné le cas correspondant pour que le refus soit rapporté par son
nom comme les autres.

Une fenêtre n'est pas une limitation à corriger plus tard — c'est le contrat,
et l'`enum` le dit. Quand le moteur aura un jour besoin d'une seconde fenêtre
(un éditeur de niveaux, un second viewport), la couture accueillera une vraie
collection de fenêtres avec une vraie propriété, et le refus deviendra ce
qu'il protégeait : une réponse bien typée au lieu d'un écrasement silencieux.

*Page traduite de la version anglaise `book/solutions/lesson-029/ex1.md`, révision `71a2428`.*

<!-- translation-source: book/solutions/lesson-029/ex1.md @ 71a2428 -->
