# Solution : exercice 1 — Vos propres touches

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — Vos propres touches](../../lessons/part-1/lesson-032-polled-input.md) de la leçon 032.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-032/ex1.patch}}
```

## Visite guidée

Ajouter des touches à la couture, ce sont trois petites retouches disciplinées
et rien d'autre. L'énum gagne les quatre nouvelles valeurs avant `KEY_COUNT` —
le compte est la taille du tableau d'état et de la boucle de scrutation, donc
l'ordre compte. La table de correspondance gagne quatre cas dans `KeyIndex`, le
seul endroit de la base de code où un code de touche de l'OS (`XK_w`) devient
une touche de la couture (`KEY_W`). Le rapport gagne quatre noms. Aucune logique
du moteur n'a changé, parce que le moteur n'a jamais vu un code de touche, pour
commencer.

Vérifié avec des touches maintenues, comme la leçon l'a fait pour les flèches :

```
$ DISPLAY=:99 xdotool keydown --window <id> w
$ DISPLAY=:99 xdotool keydown --window <id> d
$ DISPLAY=:99 xdotool keyup --window <id> w
$ DISPLAY=:99 xdotool keyup --window <id> d
engine: polled: w
engine: polled: w d
engine: polled: d
engine: polled: -
```

Les deux nouvelles touches sont suivies indépendamment : `w` maintenue pendant
que `d` arrive et repart, puis les deux relâchées.

Un détail que la correspondance révèle : `XK_w` est le keysym *minuscule* w. Ce
qu'un Shift+W produit, c'est `XK_W`, que `KeyIndex` ne suit pas — les touches de
la couture sont des touches plus ou moins physiques, pas des caractères, ce qui
est exactement ce qu'il faut pour une entrée de déplacement (le `snek` de la
leçon 024 associait des caractères et devait se soucier de la casse ; les
touches de déplacement d'un moteur ne devraient pas). Si vous voulez une entrée
avec Shift plus tard, c'est une seconde sorte d'état — du texte, pas des
touches — et un contrat différent.

*Page traduite de la version anglaise `book/solutions/lesson-032/ex1.md`, révision `a3b9017`.*

<!-- translation-source: book/solutions/lesson-032/ex1.md @ a3b9017 -->
