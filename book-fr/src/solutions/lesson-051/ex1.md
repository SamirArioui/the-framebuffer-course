# Solution : exercice 1 — Le HUD qui compte

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — Le HUD qui compte](../../lessons/part-2/lesson-051-text.md) de la leçon 051.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-051/ex1.patch}}
```

## Visite guidée

Le patch construit la ligne là où se trouve la donnée — `snprintf` dans un
tampon de pile, `X %d Y %d` avec la position du sprite — et la dessine au
démarrage avec la même vérification que celle qu'utilisent les affirmations sur
le texte de la leçon : le nombre d'emplacements par `TextWidth`, la présence
d'encre par emplacement par `GetPixel`.

La prédiction, avant l'exécution. Le sprite démarre centré à `312,232`, donc la
ligne est `"X 312 Y 232"` — onze caractères, dont trois espaces. Les espaces
ont des glyphes (entièrement en couleur clé), donc ils ne dessinent rien :
**11 emplacements, 8 avec de l'encre.** L'exécution confirme :

```
engine: hud "X 312 Y 232": 11 slots, 8 with ink
```

Vient la question que pose le format — la position qui passe de `312` à `99`.
La ligne devient `"X 99 Y 99"` : neuf caractères, neuf emplacements. Deux
emplacements de *moins*, et les chiffres se décalent vers la gauche : le nombre
de chiffres fait partie de la chaîne, et la disposition est par caractère, donc
une chaîne plus courte est une ligne plus étroite. Rien dans `DrawText` ne
redimensionne, n'aligne à droite ni ne remplit — un HUD construit dessus se
recompose quand ses données changent de largeur.

Ce qui soulève la question fantôme : l'ancienne ligne, plus longue, laisse-t-elle
des pixels derrière elle quand la nouvelle, plus courte, se dessine ?
**`DrawText` dessine ; il n'efface jamais.** Dans ce moteur, la réponse est sûre
parce que chaque frame commence par `ClearBuffer` — toute la scène est
redessinée 60 fois par seconde et l'ancien HUD est effacé avant toute autre
chose. Mais remarquez la dépendance : un moteur de rendu qui cesse d'effacer
chaque pixel (un moteur de rendu dirty-rectangle, un des leviers d'optimisation
nommés de la partie 5) laisserait les fantômes des anciens chiffres à l'écran,
et il faudrait un système de « texte » qui connaisse l'effacement. L'effacement
de la frame fait aujourd'hui, gratuitement, le travail d'effacement du texte —
cela vaut la peine de savoir quels comportements de votre moteur portent quels
autres.

Si vous poussez l'exercice plus loin et dessinez la ligne à chaque frame dans
la phase de rendu (à l'intérieur du chronométrage `text`), le cumul montre le
coût qui bouge : la phase `text` nommée passe des 0,002 ms d'une seule
étiquette à environ le double pour deux lignes — et elle le dit en nombres au
lieu de rester un mystère à l'intérieur de `render`.

*Page traduite de la version anglaise `book/solutions/lesson-051/ex1.md`,
révision `35b111c`.*

<!-- translation-source: book/solutions/lesson-051/ex1.md @ 35b111c -->
