# Solution : exercice 2 — Le héros sur votre machine

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — Le héros sur votre machine](../../lessons/part-4/lesson-076-hero.md) de la leçon 076.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-076/ex2.patch}}
```

## Visite guidée

Le diff est le bilan du héros : la distance au sol que `distance` compte déjà,
le temps d'horloge murale de l'exécution, et le temps pendant lequel le héros a
été *invité* à bouger — les frames dont la requête de déplacement était non
nulle. Deux vitesses tombent de trois nombres : la distance sur l'exécution, et
la distance sur les seules frames en mouvement. Ce découpage est tout l'intérêt,
parce que les deux nombres répondent à des questions différentes.

L'exécution de la machine de l'auteur — douze Droites et six Bas scriptés,
aucun périphérique audio, affichage sans écran — rapporte :

```
engine: hero: walked 605 px in 4.099 s (2.520 moving) — 148 px/s over the run, 240 px/s while moving, 240 in the row
```

Lisez les deux vitesses. **240 px/s en mouvement** — le chiffre de la ligne, au
chiffre près. Le modèle est exact : chaque frame où la touche était enfoncée, le
héros a avancé de `speed × dt`, et la somme de ces pas sur la somme de ces `dt`
fait 240. **148 px/s sur l'exécution** — le même mouvement dilué par les 1,58
seconde où personne n'appuyait sur rien et par l'échantillonnage de cette
machine : sans périphérique sonore, la boucle dort entre les nouvelles de
l'entrée, donc chaque appui scripté réveille deux frames (appui et relâchement)
et seule celle qui trouve la touche enfoncée déplace le héros. Le héros n'est
pas plus lent que sa ligne ; il est *invité* à bouger moins que l'horloge murale
n'a tourné.

C'est la différence que fait le portage. Sur un vrai bureau — ou avec une sortie
audio fonctionnelle qui donne le tempo à la boucle à l'horizon du tampon — une
touche maintenue garde la requête de déplacement non nulle à chaque frame, la
boucle tourne à sa propre cadence au lieu d'attendre les nouvelles, et les deux
nombres convergent vers 240. Ce que vous devriez voir sur votre machine :

- **les lignes de position** à la cadence de la touche maintenue (une par frame,
  chaque pas de `240 × dt` — avec l'alimentation audio, `dt` est un ~16,7 ms
  régulier et les pas font ~4 pixels) ;
- **la cadence du journal de frames** — l'exécution de cette machine n'est
  réveillée que par l'entrée (`walk: 301 visits over 38 frames`), la vôtre
  tourne en continu ;
- **le bilan du héros** avec `while moving` à 240 px/s et `over the run` à ce
  que votre maintien de touche et le temps d'inactivité de votre exécution en
  font.

Et la diagonale : maintenez deux directions et mesurez. Avec le modèle de cette
leçon, les deux vitesses ne coïncideront *pas* — la diagonale est √2 fois plus
rapide en mouvement (la réponse de l'exercice 1) — et le chiffre mesuré le
prouve dans vos propres unités.

L'habitude est celle de l'instrument, comme toujours dans ce cours : une
affirmation comme « le héros se déplace à la vitesse de sa ligne » est une
*mesure* — distance sur temps, prise sur une machine nommée — et le bilan
ci-dessus est ce qui en fait une. Rapportez votre machine à côté de celle du
livre (CPU, affichage, périphérique sonore ou aucun), comme le font les revues
de la partie.

Rien ici ne touche au modèle de mouvement, à la marche ou à la boucle du jeu :
le bilan est une ligne à côté de celles de l'exécution.

*Page traduite de la version anglaise `book/solutions/lesson-076/ex2.md`,
révision `08d2195`.*

<!-- translation-source: book/solutions/lesson-076/ex2.md @ 08d2195 -->
