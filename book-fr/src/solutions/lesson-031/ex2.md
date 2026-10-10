# Solution : exercice 2 — Recouvrir et révéler

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — Recouvrir et révéler](../../lessons/part-1/lesson-031-present.md) de la leçon 031.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-031/ex2.patch}}
```

## Visite guidée

Une fenêtre X11 n'est **pas conservée** : le serveur garde ce qui est
actuellement visible, mais quand une autre fenêtre recouvre la vôtre, les
pixels recouverts sont perdus. Quand votre fenêtre est révélée, le serveur ne
les reconstruit pas — il vous demande de redessiner, et cette demande est un
événement `Expose`. (Déjà à la leçon 022, la grille de caractères avait un
tampon `front` pour exactement ce problème ; ici, le framebuffer lui-même est
le tampon frontal.)

La prédiction à faire d'abord : avec un seul `Present` au démarrage — la forme
de la leçon 030 — une fenêtre recouverte puis révélée montre ce que le serveur
a bien voulu laisser derrière : du vide, du bruit, ou les restes d'une autre
fenêtre. Les pixels que le moteur a écrits ont disparu jusqu'à ce que quelque
chose les réécrive.

Avec l'impression d'instrumentation en place, la réparation est visible en
direct. La même fenêtre, dé-mappée puis mappée à nouveau (à quoi ressemblent
les dommages vus d'un client) :

```
platform: expose — the window needs its pixels
platform: expose — the window needs its pixels
engine: framebuffer 640x480, 1228800 bytes, stride 2560
engine: pixel (0,0) = 255 0 0
engine: pixel (639,479) = 0 255 0
engine: pixel (60,101) = 32 32 64
engine: presented
engine: close reported
engine: closed
```

Chaque `Expose` a réveillé la boucle du moteur, et la réponse de la boucle
n'était pas « traiter l'expose » — c'était simplement d'en refaire un
`Present`. Voilà tout le mécanisme de réparation : le moteur ne répare jamais
de régions, ne suit jamais les dommages, ne décide jamais quoi redessiner. Il
possède un framebuffer et en refait un `Present`. L'événement ne sert qu'à
réveiller la boucle ; les pixels font le reste.

La preuve que la réparation a marché est le vérificateur de l'exercice 1
lancé après la révélation : les pixels de la fenêtre correspondent toujours au
framebuffer, octet pour octet. Économe en machinerie, honnête en coût — chaque
nouveau `Present` est une copie complète, et c'est la leçon 036 qui mesurera
ce coût au lieu de le supposer.

*Page traduite de la version anglaise `book/solutions/lesson-031/ex2.md`, révision `e5b6743`.*

<!-- translation-source: book/solutions/lesson-031/ex2.md @ e5b6743 -->
