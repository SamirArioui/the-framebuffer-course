# Solution : exercice 2 — La carte en caractères

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — La carte en caractères](../../lessons/part-2/lesson-052-tilemap.md) de la leçon 052.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-052/ex2.patch}}
```

## Visite guidée

Le patch construit chaque ligne comme une chaîne des caractères de type des
cellules — `TileAt` par cellule, la table des types faisant correspondre
l'index au caractère — et la dessine avec `DrawText`, une ligne de glyphes par
ligne de carte. Le monde en art ASCII, rendu par le moteur.

Vient la vérification, et la question de l'énoncé sur la précision. Première
idée envisagée : *les emplacements portant de l'encre*. Lancez-la et la réponse
n'aide guère — chaque caractère de carte de ce fichier (`#`, `.`, `w`) est un
glyphe avec de l'encre, donc chaque emplacement de chaque ligne rapporte de
l'encre : une vérification qui lit `20 of 20` pour une ligne dont le contenu
est `#..................#`. Elle ne vérifie presque rien.

La vérification plus précise compte les **pixels d'encre** et les compare à
l'arithmétique du fichier, glyphe par glyphe. Dans cette police, le glyphe `#`
porte 22 pixels d'encre et le glyphe `.` en porte 4 (un point de 2×2). Les
prédictions sont donc de l'arithmétique avant toute exécution :

- **Ligne 0** — `####################` : 20 × 22 = **440**
- **Ligne 1** — `#..................#` : 2 × 22 + 18 × 4 = **116**

L'exécution confirme exactement :

```
engine: map view: row 0 draws 440 ink pixels
engine: map view: row 1 draws 116 ink pixels
```

Pourquoi les comptes de pixels sont la vérification plus précise : ils
composent les *trois* faits qui doivent tous être vrais — les caractères du
fichier, les glyphes de la police et la fidélité du chemin de dessin — en un
seul nombre par ligne. Une ligne intervertie avec sa voisine, un glyphe découpé
à un pixel près, un blit qui dessine la couleur clé : tous les trois changent le
compte de pixels ; presque rien ne change « a de l'encre ». Les vérifications de
présence servent à l'existence ; les comptes servent à l'exactitude.

L'habitude se généralise au-delà de cet exercice : quand une vérification peut
compter quelque chose, comptez-le. Les sommes d'octets de la leçon 044, le
`130 + 126 = 256` de la leçon 045, le `164 + 72 + 4 = 240` de la vérification
de la carte — les comptes bouclent l'arithmétique, et un compte qui boucle est
une preuve qu'une vérification de présence ne pourra jamais donner.

Et la vue de débogage elle-même mérite de rester dans votre moteur. Une carte
dessinée en caractères est brute à côté du dessin de tuiles de la leçon 053 —
mais elle s'affiche sans le moindre dessin de tuile, fonctionne à n'importe
quelle taille de carte, et répond à « le fichier est-il ce que je crois ? »
avant qu'un seul sprite de tuile existe. Les vues de débogage ne sont pas
jetables quand elles vérifient de la donnée au lieu d'images.

*Page traduite de la version anglaise `book/solutions/lesson-052/ex2.md`,
révision `cc9b3fa`.*

<!-- translation-source: book/solutions/lesson-052/ex2.md @ cc9b3fa -->
