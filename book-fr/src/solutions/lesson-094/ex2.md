# Solution : exercice 2 — Pourquoi le HUD ne défile jamais

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — Pourquoi le HUD ne défile jamais](../../lessons/part-5/lesson-094-hud.md) de la leçon 094.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-094/ex2.patch}}
```

## Visite guidée

**L'argument.** Le dessin a deux espaces de coordonnées et une règle pour
chacun : la scène se dessine à `−camera` (elle *est* le monde, vu à travers
une fenêtre qui bouge), et le HUD se dessine en coordonnées écran (il *est* le
mobilier de la fenêtre elle-même). Si `HudDraw` appliquait les décalages de la
caméra comme le fait `GameDrawMap`, les indicateurs chevaucheraient le monde :
au défilement complet de la carte (`camera 128`, la frame faisant 640 de
large), la ligne `SCORE` — disposée à `x = 8` — se dessinerait à
`8 − 128 = −120`. Son texte de 64 pixels s'étendrait de `−120 … −56` :
**entièrement hors de la frame**. Le blitter découperait chaque glyphe et le
score ne serait tout simplement pas à l'écran au moment exact où le joueur est
le plus profond dans la carte. En chemin, il glisserait de côté sous l'œil du
lecteur, et la colonne de droite dériverait vers l'intérieur, de `568` à
`440`.

Ce n'est pas seulement laid — c'est faux sur ce qu'un HUD *est*. Un HUD n'est
pas du contenu de monde ; c'est le tableau de bord du joueur. Les instruments
sont boulonnés au cockpit, pas peints sur le terrain. La règle (celle de la
leçon 054) porte sur l'**appartenance** : tout ce que le jeu *possède* — le
héros, les ennemis, la carte — vit en coordonnées monde et défile ; tout ce
que le joueur *lit* vit en coordonnées écran et ne défile pas. À l'instant où
un indicateur défile avec le monde, il cesse d'être un indicateur et devient
une décoration.

**La mesure.** La sonde imprime les coordonnées de dessin réelles des colonnes
à côté de la caméra — et à côté du contrefactuel, ce que la caméra en
*ferait* :

```
engine: probe: hud columns at 8,8 and 568,8 — camera 8,0 would make them 0,8 and 560,8
engine: probe: hud columns at 8,8 and 568,8 — camera 13,0 would make them -5,8 and 555,8
engine: probe: hud columns at 8,8 and 568,8 — camera 20,0 would make them -12,8 and 548,8
engine: probe: hud columns at 8,8 and 568,8 — camera 47,0 would make them -39,8 and 521,8
…
engine: probe: hud columns at 8,8 and 568,8 — camera 128,0 would make them -120,8 and 440,8
```

La moitié gauche ne bouge jamais — `8,8 and 568,8` sur toute l'étendue de la
caméra. La moitié droite est le contrefactuel, et elle atterrit exactement où
l'argument l'annonçait : `−120` au défilement complet, les pixels de
l'indicateur au-delà du bord gauche de la frame. Les lignes `hud:` de
l'exécution concordent (`at 8,8 over camera 8,0 … 128,0`), et l'exécution de
la leçon a parcouru la carte de bout en bout pour le montrer.

Une nuance à nommer : le x de la colonne de *droite* est calculé
(`FRAME_WIDTH − margin − TextWidth(line)`) — il dépend du texte, pas du monde.
Un alignement à droite qui bouge quand les chiffres changent, c'est de la mise
en page ; un indicateur qui bouge parce que la *caméra* a bougé, c'est le bug.
La règle porte sur l'origine du nombre, pas sur le fait que x soit une
constante.

*Page traduite de la version anglaise `book/solutions/lesson-094/ex2.md`,
révision `bf9b9df`.*

<!-- translation-source: book/solutions/lesson-094/ex2.md @ bf9b9df -->
