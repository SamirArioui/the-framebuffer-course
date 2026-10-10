# Solution : exercice 1 — Notre moteur, honnêtement comparé

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — Notre moteur, honnêtement comparé](../../lessons/part-5/lesson-102-retrospective.md) de la leçon 102.*

## Le diff

Aucun, à dessein : cet exercice n'écrit pas de code, donc `ex1.patch` est
vide — il n'y a rien à appliquer et rien à vérifier. Le livrable est la
page de prose que demande l'énoncé. Une réponse modèle suit.

## Visite guidée

Une note d'une page modèle, sur les pixels — un des six sous-systèmes que
nomme l'énoncé. La vôtre peut en choisir n'importe lequel ; la forme
ci-dessous est celle à imiter : trois affirmations avec des nombres
mesurés, trois faits de l'autre côté, un jugement.

---

**Les pixels — le nôtre contre les leurs.**

1. *Chaque pixel d'une frame est celui du CPU.* `ClearBuffer` remplit les
   307 200 pixels (640×480) du framebuffer, `DrawTileMap` blitte les
   cellules de la caméra à travers `BlitSprite`, et les sprites et le
   texte de `FontGlyph` se posent par-dessus — mesuré à l'état de
   `lesson-101` sur WSL2/Xvfb en `-O0` : `clear 0.244 ms`,
   `tilemap 0.538 ms`, sprites et texte ensemble `0.017 ms` d'une frame
   de jeu de `1.313 ms` (l'exécution de dix manches de la leçon 101).
2. *La frame quitte le moteur par exactement une boucle de copie.*
   `platform::Present` porte tout le tampon vers la fenêtre une fois par
   frame : `0.438 ms` de temps mural pour `0.004 ms` de CPU (le partage
   de la couture de la leçon 098) — l'attente est celle du serveur X, pas
   la nôtre.
3. *Le coût est attribuable à la phase.* Les lignes du compte de frame
   s'additionnent au total sans reste non nommé (`clear + sprites + text
   + tilemap = render`), ce qui est la façon dont les deux points chauds
   ont été nommés et corrigés (leçons 098-100).

Leur côté, trois faits : les frames sont dessinées à travers un GPU — des
listes de commandes depuis le CPU, des textures en VRAM ; la copie de
l'affichage est un échange ; et le coût des frames quitte le budget CPU
pour un budget que l'équipe du moteur rencontre à travers des compteurs
de pilotes et des profileurs GPU plutôt qu'à travers son propre code.

Le jugement : l'écart leur achète l'échelle — résolution, nombres
d'effets, particules par milliers — et leur coûte une pile de pilotes,
des chaînes d'outils de shaders, et des frames difficiles à attribuer
ligne par ligne. Le nôtre achète l'inverse : chaque octet est lisible,
chaque phase est nommée, et une optimisation est un diff que vous pouvez
vérifier contre des lignes mesurées. Pour un cours — et pour un jeu de
cette taille — la lisibilité vaut plus que l'échelle du GPU ; pour les
ambitions de votre jeu, cet échange est à vous de rouvrir, et le compte
de frame est ce qui vous dira quand.

---

Trois choses que la note d'une page doit faire pour satisfaire l'énoncé,
quel que soit le sous-système qu'elle choisit :

1. **Chaque affirmation porte sa machine.** Les nombres ci-dessus disent
   `-O0`, WSL2, Xvfb — parce que D12 dit qu'une affirmation de
   performance sans sa machine est une rumeur. Un nombre cité d'une leçon
   hérite de la machine de cette leçon ; un nombre que vous mesurez
   vous-même nomme la vôtre.
2. **La comparaison est à choses comparables.** La frame d'un moteur de
   production n'est pas « plus rapide au même travail » — c'est un travail
   différent (du travail GPU que le CPU ne fait jamais). Les jugements
   qui comparent nos millisecondes à celles d'un jeu livré sans dire ce
   que chaque frame *fait* sont exactement la malhonnêteté que la
   rétrospective refuse.
3. **Le jugement dit ce que le nôtre achète.** La comparaison n'est pas
   une excuse. Chaque écart a un prix de leur côté et un achat du nôtre ;
   une note d'une page qui ne liste que ce qui nous manque a manqué ce
   que le cours a construit.

*Page traduite de la version anglaise `book/solutions/lesson-102/ex1.md`,
révision `4ef60c6`.*

<!-- translation-source: book/solutions/lesson-102/ex1.md @ 4ef60c6 -->
