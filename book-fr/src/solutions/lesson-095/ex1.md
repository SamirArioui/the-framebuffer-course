# Solution : exercice 1 — Le volume suit la distance

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — Le volume suit la distance](../../lessons/part-5/lesson-095-audio.md) de la leçon 095.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-095/ex1.patch}}
```

## Visite guidée

La sonie tient en une fonction dans `sound.cpp` et un fait supplémentaire sur
chaque événement : **à quelle distance il s'est produit**. `Loudness` met à
l'échelle le volume de base du son par l'atténuation — plein à l'oreille du
héros, nul à `FADE` pixels — et elle travaille en distance *au carré*, si bien
qu'aucune racine carrée ne tourne pour mixer un son (la même règle que suivent
les vérifications de distance de l'IA) :

```cpp
double d2 = dx * dx + dy * dy;
double f2 = FADE * FADE;
if (d2 >= f2)
    return 0;
return (int)(base * (1.0 - d2 / f2));
```

Les événements transportent `(dx, dy)` depuis le héros : le tir du héros
lui-même passe `(0, 0)` — un tir à l'oreille du joueur —, les tirs des ennemis
portent le décalage de leur tireur, les coups et les morts le point où ils se
sont produits. Un son au bord de l'atténuation ne se déclenche pas du tout (et
le dit : `sound: … — silent at N px`), si bien qu'un événement à l'autre bout
de la carte ne coûte rien au pool.

L'exécution — un combat jetable avec deux sacs armés à des distances connues
(24 px et 150 px du héros) — rapporte exactement l'arithmétique de
l'atténuation :

```
engine: fire: near -> bolt (damage 0, range 160)
engine: sound: shot -> channel 1 (volume 63 of 256)
engine: fire: far -> bolt (damage 0, range 160)
engine: sound: shot -> channel 2 (volume 42 of 256)
engine: fire: hero -> bolt (damage 1, range 160)
engine: sound: shot -> channel 3 (volume 64 of 256)
engine: hit: bolt hits hero — damage 0, health 3 -> 3
engine: sound: hit -> channel 4 (volume 127 of 256)
```

- Le tir du héros lui-même : `volume 64` — le quart plein du bip, distance
  zéro.
- Le sac proche (24 px) : `64 × (1 − (24/256)²) = 63.4` → **63**.
- Le sac lointain (150 px) : `64 × (1 − (150/256)²) = 42.0` → **42**.
- Le coup sur le héros (quelques px) : `128 × (1 − ε) = 127` — la moitié
  pleine du bruit sourd, pour ainsi dire à l'oreille du joueur.

et le coup qui touche à l'autre bout du combat est plus discret encore :
`hit -> channel 3 (volume 92 of 256)` est le sac lointain qu'on tire à
~135 px (`128 × (1 − (135/256)²) ≈ 92`). Deux bips identiques, deux sonies
différentes — le mixeur n'a rien eu besoin de nouveau pour tout cela ; le
bouton de volume de chaque canal était déjà là, attendant que le jeu dise à
quel point chaque moment est fort.

Une note de réglage pour le jeu du lecteur : `FADE = 256` px a été choisi pour
que l'atténuation *morde à l'intérieur de la portée de combat de ce jeu* (les
ennemis tirent à moins de 160 px). Une atténuation plus longue que le monde
n'est pas une atténuation.

*Page traduite de la version anglaise `book/solutions/lesson-095/ex1.md`,
révision `3e7cd54`.*

<!-- translation-source: book/solutions/lesson-095/ex1.md @ 3e7cd54 -->
