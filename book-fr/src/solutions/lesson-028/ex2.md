# Solution : exercice 2 — Deux façons dont la nouvelle arrive

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — Deux façons dont la nouvelle arrive](../../lessons/part-1/lesson-028-event-pump.md) de la leçon 028.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-028/ex2.patch}}
```

## Visite guidée

Fermer une fenêtre ressemble à un seul événement ; sur X11, c'est une
conversation à deux issues possibles, et la pompe traite les deux parce que
l'une comme l'autre peut arriver :

- **La demande polie.** Un gestionnaire de fenêtres ne détruit pas une fenêtre
  dans le dos de son propriétaire. Il *demande* : un événement `ClientMessage`
  dont le premier mot est l'atome `WM_DELETE_WINDOW` — le protocole que
  `XSetWMProtocols` a enregistré à l'ouverture de la fenêtre. Le bouton de
  fermeture de votre bureau produit celle-ci.
- **Le passage à l'acte.** La fenêtre peut aussi simplement cesser d'exister —
  détruite directement, comme le fait `xdotool windowclose`, ou tuée par le
  serveur. La nouvelle arrive sous forme de `DestroyNotify`, et ce n'est pas
  une demande du tout : la ressource d'OS a déjà disparu.

Les deux se plient dans le même état, parce que pour le moteur elles
signifient la même chose : s'arrêter. Mais elles ne sont pas identiques au
moment du nettoyage — après `DestroyNotify`, l'identifiant de la fenêtre est
mort, et demander à X de la détruire encore serait une erreur X. C'est
pourquoi le pliage consigne la différence (`window->xwindow = 0`) alors que
`CloseRequested` ne le fait pas.

Avec la ligne d'instrumentation appliquée, les deux issues sont visibles.
L'abrupte :

```
platform: window destroyed (DestroyNotify)
engine: window 640x480 open — waiting for news
engine: close reported
engine: closed
```

La polie (ici envoyée par un gestionnaire de fenêtres de substitution ; sur
votre bureau, votre bouton de fermeture envoie le même message) :

```
platform: close request (ClientMessage)
engine: window 640x480 open — waiting for news
engine: close reported
engine: closed
```

(La ligne de la plateforme apparaît en premier dans ces transcriptions parce
que `stderr` n'est pas tamponné alors que `stdout` attendait dans son tampon
jusqu'à la sortie — le même comportement d'ordre des flux que les exercices
de la leçon 001 ont rencontré. Sur un terminal, les deux lignes s'entrelacent
en temps réel.)

Ce qui diffère sur votre propre machine, c'est *quelle* issue produit votre
geste de fermeture — c'est pourquoi cet exercice a la forme d'un portage. Ce
qui ne diffère pas, c'est le moteur : il a vu un drapeau passer à vrai et est
sorti de la même façon les deux fois.

*Page traduite de la version anglaise `book/solutions/lesson-028/ex2.md`, révision `8904d11`.*

<!-- translation-source: book/solutions/lesson-028/ex2.md @ 8904d11 -->
