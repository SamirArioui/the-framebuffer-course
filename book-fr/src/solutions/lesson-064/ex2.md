# Solution : exercice 2 — Ce que coûte le mixage

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — Ce que coûte le mixage](../../lessons/part-3/lesson-064-mix.md) de la leçon 064.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-064/ex2.patch}}
```

## Visite guidée

Le diff est une seule sonde : `MixBuffer` chronométré seul, sur un mixeur
d'essai, avec 0, 1, 2, 4, 8 et 16 canaux actifs. Deux détails font que les
nombres veulent dire ce qu'ils disent. Le mixeur est **réarmé avant chaque
essai** — `MixerInit`, puis un `ChannelPlay` par canal — pour que chaque tampon
chronométré ait exactement les canaux annoncés en train de jouer. Et un essai
fait 29 tampons, un de moins que les trente de la tonalité, pour qu'aucun canal
ne tombe à court d'échantillon en cours d'essai : sans ces deux gestes,
« 16 actifs » dégénérerait en « 16 qui s'éteignent vers le silence » et la
mesure dériverait au fil de l'essai. Chaque configuration totalise 5800
tampons — 200 essais × 29 — pour que le battement propre de l'horloge soit dans
le nombre, pas à côté.

La mesure, une exécution sur cette machine :

```
engine: mix cost:  0 channels — 0.0206 ms per 735-frame buffer, 0.0280 us per output frame
engine: mix cost:  1 channels — 0.0224 ms per 735-frame buffer, 0.0305 us per output frame
engine: mix cost:  2 channels — 0.0241 ms per 735-frame buffer, 0.0328 us per output frame
engine: mix cost:  4 channels — 0.0255 ms per 735-frame buffer, 0.0346 us per output frame
engine: mix cost:  8 channels — 0.0349 ms per 735-frame buffer, 0.0475 us per output frame
engine: mix cost: 16 channels — 0.0343 ms per 735-frame buffer, 0.0466 us per output frame
```

Ce que les nombres montrent, dans l'ordre où l'exercice les demande.

**Contre le budget de frame.** La pire configuration — les seize canaux qui
jouent — coûte 0,0343 ms d'une frame de 16,7 ms : 0,2 % du budget, 0,047 µs par
trame de sortie. Même un pool plein mixant à plein régime ne peut pas affamer la
boucle ; le mixage n'est pas là où passe le temps de frame de ce moteur.

**Ce qui grandit et ce qui ne grandit pas.** La partie fixe est le parcours du
pool : à zéro canal actif, le mixage coûte encore 0,0206 ms par tampon — seize
tirages par trame de sortie, 11760 appels à `ChannelFrame` par tampon, presque
tous ne renvoyant rien. Ce coût est celui de la taille du pool et ne bouge pas
quand des canaux se mettent à jouer. La partie active est le reste : de zéro à
seize canaux, le tampon grandit de 0,0137 ms, environ 0,8 µs par canal ajouté et
par tampon — la lecture de l'échantillon, la mise à l'échelle en virgule fixe,
le curseur. La croissance est réelle mais assez petite pour que les comptes
voisins soient dans le bruit d'une exécution à l'autre : une seconde exécution
sur cette machine a donné `0.0202 / 0.0218 / 0.0237 / 0.0278 / 0.0366 / 0.0346`
— la même forme, et 8 et 16 qui échangent encore leurs places.

**Contre le journal de frames.** Le registre de l'exécution elle-même est
d'accord avec la sonde. Cette frame, tirée de la même exécution que les nombres
ci-dessus :

```
frame 1: update 0.001 ms, audio 0.033 ms, render 1.567 ms (sprites 0.001, text 0.006, tilemap 0.872), present 1.190 ms, total 2.791 ms
```

La phase audio lors d'une alimentation atterrit autour de 0,025–0,035 ms sur cette
machine — les 0,0241 ms de mixage de la sonde aux deux canaux actifs de la démo,
plus la soumission. La ligne audio occasionnelle à 0,2–0,4 ms n'est pas le
mixage : la sonde borne le mixage à 0,035 ms même avec un pool plein, donc le
reste est le chemin de soumission et ce que l'ordonnanceur a fait cette
frame-là.

**Ce que la sonde ne peut pas cacher sur elle-même.** Les nombres absolus sont
ceux de ce build — `-O0 -g`, les options par défaut du cours — et un autre
niveau d'optimisation les déplace. L'échantillon est un seul tampon de 44 Ko
relu en boucle ; il est chaud en cache d'une façon que l'ensemble d'échantillons
d'un jeu n'est pas. Le chronométrage ne couvre que `MixBuffer` — pas de
périphérique, pas de soumission, pas de boucle autour de lui — donc il chiffre
le mixage, pas le chemin du son. Et c'est une seule machine : la seconde
exécution ci-dessus est la barre d'erreur honnête, une microseconde ou deux par
tampon.

Ce qui survit à tout cela, c'est la forme de la leçon : le parcours du pool se
paie qu'on joue ou non, les canaux ajoutent chacun un peu, et le total est petit
devant la frame — c'est pourquoi la leçon 070 pourra mettre la ligne audio dans
la table du budget de frames sans bataille de budget.

*Page traduite de la version anglaise `book/solutions/lesson-064/ex2.md`,
révision `36cc491`.*

<!-- translation-source: book/solutions/lesson-064/ex2.md @ 36cc491 -->
