# Solution : exercice 1 — Les bandeaux sortent du froid

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — Les bandeaux sortent du froid](../../lessons/part-5/lesson-097-debt.md) de la leçon 097.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-097/ex1.patch}}
```

## Visite guidée

Les cinq bandeaux sont les rapports de l'exécution comme n'importe quels autres
— des lignes qu'une démonstration de la checklist lit — donc le déménagement
suit la règle de la leçon elle-même : les printf voyagent entiers, `report.*`
gagne le nom qui leur manquait (`ReportBanners`, prenant le `World` qu'ils
décrivent), et `main.cpp` abandonne les deux includes (`font.h`, `tiles.h`) dont
les bandeaux étaient les derniers à avoir besoin. Le fichier de la boucle ne
contient plus que la fenêtre, la boucle et la clôture — et l'identité de
l'exécution est imprimée depuis la paire de rapports à côté de tous les autres
rapports.

L'exécution se présente encore exactement comme elle l'a toujours fait — mêmes
cinq lignes, mêmes mots, même ordre, d'une vraie exécution du build patché :

```
engine: part 4 done — the vertical slice: a hero walks the tilemap, the camera follows
engine: world 48x32 cells (768x512 px), 3 kinds; 96 glyphs; hero 32x16
engine: sound 132300-frame music looping on channel 0; effects of 3528/6615/17640 frames on the pool; one mixer of 16 channels
engine: arrows move the hero, 1 and 2 arm the weapons, space fires; close the window to stop
engine: hero at 312,232
```

Et la forme de la transcription ne bouge pas du tout : l'exécution réduite à
ses rapports (la méthode de l'exercice 2) est le même ensemble de gabarits que
celui du build non patché — les cinq gabarits des bandeaux parmi eux, dans le
même ordre de première apparition. Un détail à nommer pour vos propres
refactorisations : les bandeaux se placent entre le démarrage du monde et
l'ouverture de l'audio *à dessein* — `WorldStart` imprime les vérifications du
monde, les bandeaux nomment l'exécution, et seulement alors la couture ouvre sa
sortie — et le déménagement garde cet ordre, donc le chemin d'échec (`no audio
output on this machine`) s'imprime encore *après* l'identité, exactement là où
un lecteur l'attend.

*Page traduite de la version anglaise `book/solutions/lesson-097/ex1.md`,
révision `3e7f026`.*

<!-- translation-source: book/solutions/lesson-097/ex1.md @ 3e7f026 -->
