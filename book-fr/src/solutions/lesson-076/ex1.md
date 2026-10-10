# Solution : exercice 1 — Le pas, prédit

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — Le pas, prédit](../../lessons/part-4/lesson-076-hero.md) de la leçon 076.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-076/ex1.patch}}
```

## Visite guidée

Le diff est une sonde jetable à côté de la création du héros : une copie de
l'entité du héros, sa requête de déplacement écrite à la main, et deux pas de
l'arithmétique exacte que la marche avance — `requête × vitesse × dt` — à un
`dt` choisi pour que les chiffres soient vérifiables : 0,5 seconde.

Les prédictions, avant toute exécution. Le héros part des `312,232` de sa ligne
et sa ligne dit vitesse 240. **Frame une** — Droite à `dt = 0.5` : `x += 1.0 ×
240 × 0.5` = 120 pixels, donc `432.0, 232.0`. **Frame deux** — Droite *et* Bas à
`dt = 0.5` : les deux axes avancent de 120 pixels, donc `552.0, 352.0`. La
seconde réponse est celle qui mérite une phrase : se déplacer en diagonale
couvre 120 pixels *sur chaque axe* — 169,7 pixels de terrain réel, à 339 px/s —
tandis qu'un déplacement droit en couvre 120 à 240 px/s. La diagonale est √2
fois plus rapide, parce que le modèle traite les deux axes comme indépendants.

Les exécutions, à partir de l'état final de cette leçon plus le patch :

```
engine: probe: Right at dt 0.5 -> 432.0,232.0
engine: probe: Right+Down at dt 0.5 -> 552.0,352.0
```

Exactement comme prédit.

Maintenant la question. **Ce qui change :** la requête est normalisée avant de
devenir du mouvement — quand les deux composantes sont non nulles, divisez les
deux par √2 (ou écrivez la requête comme un vecteur unitaire d'emblée). Alors
Droite+Bas à `dt = 0.5` avance de 84,85 pixels par axe : 120 pixels de terrain,
la même chose qu'un déplacement droit. **Où :** c'est un choix de conception
avec deux réponses défendables. Faites-le là où la *requête* s'écrit — l'étape
d'entrée ou l'IA — et chaque mover décide ce que sa propre direction signifie
(un projectile pourrait vouloir la diagonale plus rapide ; un joueur, d'ordinaire,
non). Faites-le dans la *marche* — normalisez chaque requête qui arrive — et la
règle tient pour chaque entité sans répétition. Ce moteur le laisse au jeu à
dessein : le modèle de mouvement appartient au jeu, et la leçon du héros de la
partie 5 remplace tout ce pas par une sensation d'accélération et de
décélération, où « même vitesse en diagonale » est une ligne dans une idée bien
plus grande.

Une habitude à remarquer : la sonde copie le héros (`Entity probe = hero`) et
avance la copie — le vrai héros, le vrai magasin (store) et la vraie marche sont
intacts. Les sondes sont jetables ; les jeux gardent leur état.

Rien ici ne touche à la marche, à l'étape d'entrée ou à la boucle du jeu : la
sonde est deux frames forcées à côté de celles de l'exécution.

*Page traduite de la version anglaise `book/solutions/lesson-076/ex1.md`,
révision `08d2195`.*

<!-- translation-source: book/solutions/lesson-076/ex1.md @ 08d2195 -->
