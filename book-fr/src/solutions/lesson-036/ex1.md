# Solution : exercice 1 — La copie, isolée

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — La copie, isolée](../../lessons/part-1/lesson-036-frame-time.md) de la leçon 036.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-036/ex1.patch}}
```

## Visite guidée

Le temps mesuré de `Present`, ce sont deux choses très différentes collées
ensemble : `XPutImage` — pousser les pixels vers le serveur — et `XSync` —
attendre que le serveur en ait fini avec eux (le contrat de la leçon 031 :
quand `Present` rend la main, les pixels sont à l'écran). L'instrument
chronomètre chaque moitié séparément.

```
platform: copy 0.446 ms, wait 0.054 ms
platform: copy 0.283 ms, wait 0.043 ms
platform: copy 0.403 ms, wait 0.064 ms
platform: copy 0.574 ms, wait 0.072 ms
platform: copy 0.365 ms, wait 0.084 ms
```

La copie est le coût — environ 0,3 à 0,9 ms pour du 640×480 sur la machine où
ceci a été rédigé — et l'attente est petite mais jamais nulle : c'est
l'aller-retour plus ce que le serveur avait encore à faire. Les deux sont du
vrai travail ; ni l'une ni l'autre n'est de la comptabilité.

Quelle moitié la mémoire partagée (MIT-SHM) supprimerait-elle ? La *copie*.
Avec un segment partagé, le serveur lit le framebuffer sur place, donc l'envoi
de `XPutImage` devient un pointeur et le seul coût restant est l'aller-retour
— c'est pourquoi la conception a nommé MIT-SHM comme optimisation ultérieure
plutôt que comme leçon de la partie 1 : voici la mesure à l'aune de laquelle
cette leçon ultérieure sera argumentée.

Une mise en garde que ces nombres méritent : ils viennent de Xvfb — un
affichage virtuel sans compositeur, sans autres clients et sans vrai écran.
Sur un bureau, les deux mêmes nombres bougent (taille de fenêtre, pilote,
composition). Ce qui ne bouge pas, c'est la *forme* : la copie de présentation
pèse une vraie part de la frame, elle a lieu à chaque frame, et elle est
désormais mesurée au lieu d'être supposée.

*Page traduite de la version anglaise `book/solutions/lesson-036/ex1.md`,
révision `5342420`.*

<!-- translation-source: book/solutions/lesson-036/ex1.md @ 5342420 -->
