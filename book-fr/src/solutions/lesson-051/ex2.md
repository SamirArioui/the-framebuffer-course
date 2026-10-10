# Solution : exercice 2 — Les deux sortes de rien

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — Les deux sortes de rien](../../lessons/part-2/lesson-051-text.md) de la leçon 051.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-051/ex2.patch}}
```

## Visite guidée

La prédiction, à partir des deux moitiés de `FontGlyph` — l'arithmétique
d'index et la vérification de bornes. Pour `' '` : octet 32, index 0 — la
première cellule de la planche. Il est *dans* la planche ; la fonction retourne
un vrai sprite dont les 64 pixels se trouvent tous être de couleur clé. Pour
l'octet `200` : index 168, au-delà de `FONT_COUNT = 96` — la fonction retourne
`0`, et il n'y a aucun sprite. L'exécution :

```
engine: space: has a glyph; byte 200: no glyph
engine: "A B": 2 slots with ink, 1 without
```

Les deux réponses comme prévu, et `"A B"` rapporte la carte des emplacements
que produit la règle de la leçon : `A` se dessine, l'emplacement de l'espace
est vide, `B` se dessine **à `x + 2 × 8`** — sa position de disposition,
calculée à partir de son index, que l'emplacement vide du milieu ne touche pas.

Voyons maintenant pourquoi la distinction compte, même si les pixels
concordent. Les deux riens sont des *faits différents sur la police* :

- **Un espace est du contenu qui se trouve être invisible.** La planche dit
  « le caractère 32 existe et ressemble à rien ». Si vous recolorez ou
  redessinez la première cellule de la planche, l'espace devient visible — c'est
  du dessin comme n'importe quelle autre cellule.
- **Un octet manquant est un trou dans la planche.** La police n'a simplement
  aucune opinion sur ce caractère. La disposition réserve l'emplacement (pour
  que la forme de la chaîne soit stable) et le dessin saute (pour que rien de
  faux ne s'affiche) — et c'est *tout* ce que le moteur peut faire
  honnêtement.

La dernière question de l'énoncé — et si la police n'a pas un glyphe dont vous
avez besoin — n'a qu'une seule réponse sous ce format : **la planche est la
police**. Pas de repli, pas de glyphe synthétisé, pas de « dessine une boîte »
(une boîte serait un glyphe que la police *a* bien, comme la cellule DEL de la
planche à 127, et le moteur ne la dessinerait que si le *caractère* valait
127). Pour prendre en charge un nouveau caractère, vous redessinez la planche —
étendez la grille, déplacez la constante du format, et toute planche chargée
doit suivre. C'est le coût d'un format assez petit pour être défini à la main,
et la prose de la leçon 050 le disait déjà : les leçons suivantes étendent en
*lisant davantage*, jamais en réinterprétant.

Quand la table de budget de frames de la leçon 058 imprimera ses propres
nombres, elle construira ses chaînes à partir de chiffres et d'espaces — les
uns comme les autres dans la planche, les uns comme les autres sans ambiguïté.
Si un jour vous vous surprenez à taper un `µ` dans une chaîne de HUD, vous
savez maintenant exactement quels deux emplacements seront vides, et pourquoi.

*Page traduite de la version anglaise `book/solutions/lesson-051/ex2.md`,
révision `35b111c`.*

<!-- translation-source: book/solutions/lesson-051/ex2.md @ 35b111c -->
