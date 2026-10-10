# Solution : exercice 1 — Le score sur les écrans de fin

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — Le score sur les écrans de fin](../../lessons/part-5/lesson-082-skeleton.md) de la leçon 082.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-082/ex1.patch}}
```

## Visite guidée

L'exercice est une seule idée, deux fois : le score appartient au jeu, et les
écrans de fin sont ceux du jeu. Deux gestes que le squelette était déjà prêt à
faire.

**Le score déménage dans `Game`.** C'était un local de `Run` — `double distance`,
accumulé par la marche et lu par le HUD du jeu. C'est un fait de la boucle, pas
du jeu, et c'est exactement pourquoi les écrans de fin ne pouvaient pas le voir :
ils sont dessinés par `GameDrawPanel`, dans `game.cpp`, et un local de `Run` n'y
arrive pas. Le score devient donc `Game::score`, un double auquel la marche
ajoute toujours (`game.score += …`) et qu'un jeu neuf remet à zéro sur la touche
de démarrage du titre — la même restauration que les points de vie du héros et
les vagues du jeu reçoivent déjà. Le jeu porte son propre résultat.

**Les écrans de fin le lisent.** Un petit auxiliaire `PanelScore` formate
`SCORE %06d` et le centre sous l'invite du panneau ; les cas de mort et de
victoire l'appellent avec `game.score`. Le dessin est du texte comme un autre —
il atterrit dans la phase `text` de la frame, et les panneaux restent des
panneaux (le monde n'est toujours pas dessiné derrière eux).

**Les états de fin acceptent aussi Escape.** Les cas de mort et de victoire
lisent désormais `KEY_ENTER` ou `KEY_ESCAPE`, toujours à travers la mémorisation
de la leçon 033, si bien qu'un appui est une action. Cela ne casse pas la règle
de la machine — seule l'entrée de l'état courant agit ; le changement est que
les états de fin listent maintenant *deux* touches parmi celles qu'ils
acceptent. Escape quitte déjà le jeu pour la pause, donc un joueur qui la
cherche sur l'écran de fin n'est pas surpris.

L'exécution, depuis l'état final de cette leçon plus le patch — le héros est
touché trois fois et meurt, puis Escape est pressée sur l'écran de mort :

```
engine: state title -> play (the player started)
engine: hero takes a hit — health 2 (t=0.020)
engine: hero takes a hit — health 1 (t=0.665)
engine: hero takes a hit — health 0 (t=1.109)
engine: state play -> death (the hero's health reached zero)
engine: state death -> title (the player returned to the title)
```

La dernière ligne est l'entrée élargie : `death -> title` s'est déclenchée
depuis l'écran de mort, et c'est Escape qui l'a déclenchée. Le score lui-même
est un visuel — le panneau de mort affiche maintenant `SCORE 000142` (la marche
du héros dans cette exécution) sous `GAME OVER` ; ouvrez la fenêtre pour le voir
sur les deux écrans de fin, de la même façon que le texte des panneaux se
vérifie.

Une chose à remarquer sur la remise à zéro : le score vit dans `Game` désormais,
donc `title -> play` le restaurant à zéro est la même restauration d'une ligne
que les points de vie et les vagues — les faits d'une exécution neuve sont tous
au même endroit. Quand le vrai combat et les vraies vagues arriveront (leçons
087 et 091), le score qu'ils attribuent écrira dans le même champ, et les écrans
de fin continueront de le rapporter sans savoir d'où il vient.

*Page traduite de la version anglaise `book/solutions/lesson-082/ex1.md`,
révision `a4635cd`.*

<!-- translation-source: book/solutions/lesson-082/ex1.md @ a4635cd -->
