# Solution : exercice 2 — Ce que coûte un jeu en pause

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — Ce que coûte un jeu en pause](../../lessons/part-4/lesson-079-wall-clock.md) de la leçon 079.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-079/ex2.patch}}
```

## Visite guidée

Le diff est une comptabilité par fenêtres : l'exécution compte chaque frame et
somme son `total` d'horloge murale, et chaque fenêtre se referme quand le
bouton tourne — le rapport nomme donc la frame moyenne à *chaque* échelle,
depuis la même exécution, sur la même machine, sans que rien d'autre ne change.

Les nombres, depuis une vraie exécution de l'état final de cette leçon plus le
patch sur la machine de rédaction (affichage sans écran, appuis scriptés, aucun
périphérique sonore) :

```
engine: window (play): 33 frames at scale 1.00 averaged 1.791 ms of wall clock
engine: window (hitstop): 16 frames at scale 0.25 averaged 1.772 ms of wall clock
engine: window (pause): 32 frames at scale 0.00 averaged 1.780 ms of wall clock
engine: window (the run's last): 64 frames at scale 1.00 averaged 1.715 ms of wall clock
```

**La pause n'est pas gratuite.** La fenêtre en pause a fait en moyenne 1.780 ms
par frame contre 1.791 ms pour le jeu — le même coût, dans le bruit de la
mesure. Le hitstop, une simulation au quart de vitesse, a fait en moyenne
1.772 ms. L'échelle ne fait rien au prix de la frame, ce qui est le but : la
frame est du travail de machine, et la machine ne sait pas que le jeu est
immobile.

**Où passe le temps**, c'est dans les phases de l'enregistrement (les lignes de
journal de la leçon 079) : `render` ~1.36 ms — la marche de la tilemap et les
blits, la même scène dessinée à chaque frame — et `present` ~0.5 ms — la copie
vers la fenêtre. `update`, la seule phase que l'échelle touche, est à ~0.01 ms :
la part de la frame que le jeu *pourrait* rendre gratuite est celle qui l'est
déjà presque. Le coût d'une frame en pause est le coût de montrer un jeu en
pause.

**Ce que coûterait d'arrêter aussi la présentation** : les lignes render et
present, ~1.85 ms de la frame de 1.78 ms — presque tout — plus ce que le
système de fenêtres fait d'une fenêtre qui ne se met jamais à jour. Le jeu
économiserait cela et perdrait l'écran de pause : pas de menu dessiné, pas de
fondu présenté, aucun moyen pour le joueur de voir ce qu'il a mis en pause. Les
nombres rendent l'arbitrage explicite plutôt qu'affaire de goût.

**La question de design.** Si un écran de pause veut être moins cher, que
doit-il changer ? L'**échelle** — non : elle est déjà à zéro et le coût n'est
pas là. Le **dessin** — oui, et c'est le vrai levier : dessiner *moins*, pas
rien. Un écran de pause qui dessine le monde figé une fois dans un tampon puis
ne dessine que les parties mobiles du menu peut retirer la marche de la tilemap
(la plus grosse ligne) de chaque frame ; c'est une décision de rendu, pas une
décision de temps. Le **calendrier de la machine** — parfois : une pause sur
une machine sur batterie peut baisser la fréquence de présentation (présenter
une frame sur deux, ou seulement quand le menu change), ce qui est la cadence
de boucle que la couche plateforme possède déjà (l'attente cadencée de la leçon
060), pas l'échelle du temps de jeu. Les trois leviers existent ; l'échelle est
le mauvais levier pour ce problème, et c'est la mesure qui le dit.

C'est la vraie leçon de l'exercice : **le bouton que vous avez n'est pas
toujours celui qui contrôle le coût que vous payez**. La discipline d'horloge
murale de l'enregistrement de frame est ce qui transforme « le jeu est en
pause » d'un ressenti en une ligne de nombres qui pointe vers le bon levier.

Rien ici ne touche l'échelle, l'enregistrement ou la boucle : la comptabilité
par fenêtres est une struct et une ligne à côté de celles de la leçon.

*Page traduite de la version anglaise `book/solutions/lesson-079/ex2.md`,
révision `4eaa176`.*

<!-- translation-source: book/solutions/lesson-079/ex2.md @ 4eaa176 -->
