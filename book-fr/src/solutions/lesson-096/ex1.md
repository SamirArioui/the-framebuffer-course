# Solution : exercice 1 — La pause par-dessus le monde figé

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — La pause par-dessus le monde figé](../../lessons/part-5/lesson-096-screens.md) de la leçon 096.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-096/ex1.patch}}
```

## Visite guidée

Trois gestes font que la pause montre son monde. Le framebuffer grandit
`ClearRect` — `ClearBuffer` découpé à un rectangle, avec le même pli (la
partie d'un rectangle hors de la frame est abandonnée, jamais repliée). La
phase de render dessine le monde pour la pause exactement comme pour le jeu
(les sous-phases de la carte et des sprites s'exécutent ; le monde est *figé*,
pas disparu). Et le panneau de pause peint un rectangle de couleur de panneau
— apparu en fondu avec l'écran, si bien que le rectangle arrive avec le même
easing — derrière ses lignes, pour qu'elles restent lisibles par-dessus la
scène.

L'enregistrement de frame mesure la différence mieux que toute description.
Les frames de pause portent désormais les sous-phases du monde :

```
engine: state play -> pause (the player paused)
engine: screen: pause: "PAUSED" / "SCORE 000422   TIME 0:02   WAVE 1/3" / "ESCAPE: RESUME"
frame 80: step 0.000 ms, update 0.009 ms (entities 0.002), audio 0.000 ms, render 1.539 ms (sprites 0.004, text 0.160, tilemap 0.961), present 0.340 ms, total 1.888 ms
frame 81: step 0.000 ms, update 0.011 ms (entities 0.003), audio 0.030 ms, render 2.027 ms (sprites 0.180, text 0.158, tilemap 1.157), present 0.495 ms, total 2.562 ms
```

`tilemap 0.961` et `sprites 0.004` — la carte et les entités sont dessinées —
à côté de `step 0.000 ms`, le monde *figé* sous elles. Le coût propre du
panneau est `text 0.160` (le rectangle et trois lignes, plus que les `0.016` du
vieux panneau — un rectangle, c'est du vrai travail). Les autres écrans ne
dessinent toujours aucun monde — l'écran de mort, une page plus loin dans la
même exécution :

```
engine: screen: death: "GAME OVER" / "SCORE 000424   TIME 0:06   WAVE 1/3" / "ENTER: TITLE"
frame 212: step 0.000 ms, update 0.014 ms (entities 0.003), audio 0.037 ms, render 0.436 ms (sprites 0.000, text 0.016, tilemap 0.000), present 0.483 ms, total 0.971 ms
```

`sprites 0.000, tilemap 0.000` — les écrans de fin sont leur propre monde,
comme le dit le design.

Deux jugements dans le diff, qui valent d'être nommés. La pause garde le fond
du *jeu* (`32,32,64`) plutôt que celui du panneau — la scène est le sujet de
l'écran et le rectangle est la seule chose couleur panneau. Et le rectangle est
dessiné *avant* les lignes, à l'intérieur du dessin du panneau lui-même :
l'écran possède tout ce qu'il montre, et l'ordre de dessin à l'intérieur lui
appartient — le rectangle d'abord, le texte ensuite, exactement la règle propre à la phase de
render (ce qui est derrière se dessine en premier) appliquée un niveau plus
bas.

*Page traduite de la version anglaise `book/solutions/lesson-096/ex1.md`,
révision `ccbe035`.*

<!-- translation-source: book/solutions/lesson-096/ex1.md @ ccbe035 -->
