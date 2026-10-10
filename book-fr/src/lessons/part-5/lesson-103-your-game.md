# Leçon 103 — maintenant, faites VOTRE jeu

{{#include ../../stability-horizon.md}}

## Prose

Le jeu est terminé, et la rétrospective a regardé en arrière honnêtement et nommé
l'horizon. C'est la dernière leçon du cours, et elle ne parle plus de ce jeu-ci.
Elle parle du **vôtre**. Le moteur et la boîte à outils sont finis — et un moteur
fini ne vaut quelque chose qu'une fois tourné vers un jeu que personne n'a encore
fait. Cette leçon fait donc la passation du moteur : **ce qu'il faut garder, ce
qu'il faut changer, où sont les coutures, ce qu'il faut consigner plutôt que
construire, et où mesurer.**

### Ce qu'il faut garder

Les services ne bougent pas. `Arena` (l'unique réservation ; aucune allocation
pendant que le jeu tourne), `EntityTable`/`LoadRunTable` (les tables, chargées
entières, complètes ou nommées), `EntityStore`/`CreateEntity`/`EntityRetire`
(le magasin fixe — refus typé, jamais de vol), `MoveEntity` (le mover),
`GameTime` (l'unique échelle), la tilemap et ses requêtes de collision, la
caméra, le framebuffer et l'unique boucle de copie, les seize canaux du mixeur,
et `FrameStats`/`PrintFrameBudget` (le compte et son rapport) — voilà le moteur
que les parties 1 à 4 ont construit, et la partie 5 les a consommés sans les
reconcevoir, pour la raison même qui doit valoir pour votre jeu : **un service
que vous gardez est un service que vous comprenez, et une refonte ne se justifie
que par un problème mesuré.** Gardez aussi les règles qui les ont rendus dignes
de confiance : la loi du langage (aucune allocation pendant que le jeu tourne,
aucune exception, rien dans le dos de la couture), des échecs typés à chaque
frontière, et `tools/check-boundary.sh` au vert avant que le moindre commit ne
quitte votre arbre.

### Ce qu'il faut changer

Les fichiers de la couche jeu et tout ce qui vit sous `assets/` sont à vous. Vos
types sont des lignes de table — un nouvel ennemi, un nouveau projectile, un
nouveau pickup sont une nouvelle ligne dans un fichier qui se charge déjà. Vos
cartes, votre art, vos sons sont des fichiers au même format que celui livré par
le cours (et le format est une couture, plus bas, si votre jeu le dépasse). Vos
règles vivent dans les paires de fichiers dont les noms disent ce qu'elles
contiennent : la machine à états et les vagues dans `game.*`, le ressenti du
déplacement dans `hero.*`, les armes et les coups dans `combat.*`, les
comportements dans `ai.*`, les quatre effets dans `feel.*`, les indicateurs dans
`hud.*`, la correspondance déclencheur-vers-son dans `sound.*`. Réécrivez
librement l'*intérieur* de ces fichiers — c'est ce à quoi ils servent. Leurs
*bords* sont les coutures.

### Où sont les coutures

L'étape de code de cette leçon en est une carte, laissée dans l'en-tête de
`src/game.h` pour qu'elle voyage avec le code. Quatre coutures :

1. **La couche plateforme.** `platform.h` est tout le contrat ; un OS y répond
   en deux fichiers. Un second OS — le module Windows de la carte de
   l'épilogue — implémente le même contrat dans ses propres fichiers et ne
   change aucun fichier du moteur. Ne laissez jamais un type d'OS franchir
   cette ligne ; le contrôle de frontière est l'audit.
2. **Le format des tables.** `table.*`/`load.*` plus les fichiers sous
   `assets/`. Ne faites grandir le format que par des colonnes nommées, de
   façon additive — tout fichier que vous avez jamais livré continue de se
   charger, sinon la croissance est fausse (la règle de la leçon 087). Un
   nouveau type est une ligne ; un nouveau fait sur les types peut être une
   colonne.
3. **Les fichiers de la couche jeu.** La liste ci-dessus. Tout ici est à vous
   de changer ; quand l'un de ces fichiers veut quelque chose que les services
   ne peuvent pas faire, c'est une question de *conception* (changer un service
   exprès, ou composer autrement), pas une permission de contourner la
   couture.
4. **Où mesurer.** Le compte de `frame.*` et le rapport de la clôture sont
   intégrés : les lignes nomment les phases, la ligne de budget vérifie les
   60 fps frame par frame, la ligne machine garde les nombres avec leur nom
   (D12). Quand vous avez besoin du nom d'une fonction plutôt que de celui
   d'une phase, `gprof` (la passe de mesure, leçon 098) ; quand vous avez
   besoin du comportement plutôt que du coût, la discipline de la transcription
   (l'exercice de la leçon 097) ; quand votre jeu gagne des points chauds, le
   menu en trois passes, sans détour : **mesurez, corrigez ce que vous avez
   mesuré, rapportez ce que vous n'avez pas corrigé.**

### La discipline qui survit au cours : consigner, ne pas ajouter

Votre jeu attirera des idées — de vous, plus vite que de n'importe qui. La
règle du MVD est l'outil pour cela, et elle n'expire pas : **une idée hors de la
checklist figée est un extra — consignée dans votre registre, jamais ajoutée.**
Écrivez la checklist figée de votre jeu *avant* sa première leçon (la
forme de `plan/target-game-mvd.md`, que vous avez maintenant lu en entier) : les
lignes qui le rendent fini, la boîte à outils à laquelle vous vous limitez, le
menu d'optimisation que vous exécuterez. Ensuite, chaque idée qui arrive reçoit
l'une de deux réponses honnêtes : elle est sur la checklist, ou elle va au
registre. Le registre des extras de la leçon 102 est l'exemple travaillé de
cette règle — y compris ses deux plus grosses entrées (le port GPU, le module
Windows), consignées exactement comme vos idées devraient l'être : nommées,
cadrées, et *non construites*. Le périmètre est une promesse ; la checklist
décide quand votre jeu est fini. « Plus de fonctionnalités » est un chapitre
d'extras ou du travail d'après-cours — jamais fini.

### Ce que cette leçon a vérifié, et ce qu'elle n'a pas vérifié

- **La passation est dans le code, pas seulement dans la prose** : l'étape de
  code ci-dessous est la carte des coutures sous forme de commentaire d'en-tête
  sur `src/game.h` — un commentaire, aucun comportement. Le build est sans
  avertissement dans cet état et le contrôle de frontière est propre (un
  commentaire qui nomme des OS ne déclenche aucune fausse alerte).
- **La page se construit** (`mdbook build`), et les solutions des exercices
  s'appliquent proprement sur l'état final de cette leçon — les deux patchs
  vérifiés avec `git apply --check` ici avant publication.

Ce que cette leçon n'a **pas** vérifié, c'est quoi que ce soit de votre jeu —
aucune exécution, aucune mesure, aucune affirmation. C'est toute la forme de la
passation : le moteur est mesuré, nommé et audité ; **votre jeu est la partie
non mesurée, et la mesurer est la première chose qui vous attend.**

## Étape de code

Un seul changement, et c'est un commentaire : la carte de la passation — les
quatre coutures et la règle « consigner, ne pas ajouter » — sous forme de bloc
d'en-tête sur `src/game.h`, le fichier d'entrée de la couche jeu. Aucun
comportement ne change ; `./build.sh` et `./tools/check-boundary.sh` tournent
exactement comme avant. C'est petit exprès : une passation est une carte, pas un
mécanisme, et la valeur de la carte est qu'elle vit là où les yeux de la
personne suivante sont déjà — en haut du fichier qui possède le jeu. Son état
final est étiqueté `lesson-103`.

```diff
diff --git a/src/game.h b/src/game.h
index b3bcccc..c340fbb 100644
--- a/src/game.h
+++ b/src/game.h
@@ -17,6 +17,32 @@
 // the services (table, store, mover, game-time, camera) never grow game
 // behavior, and the game never grows inside them. The play state's
 // gameplay stands on those services and invents nothing.
+//
+// Lesson 103: the hand-over — the engine and the toolkit are finished,
+// and now they are yours. This header names where the codebase is meant
+// to change and where it is meant to hold still: the four seams.
+//
+//   1. The platform layer — `platform.h` is the seam, and one OS
+//      answers it in two files (the window side, the sound side). A
+//      second OS implements the same contract in its own files, and no
+//      engine file changes when it does (tools/check-boundary.sh audits
+//      that); the Windows module of lesson 102's epilogue map is
+//      exactly this seam, a second time.
+//   2. The table format — `table.*`/`load.*` and the files under
+//      `assets/`. Your game's kinds are rows; grow the format only by
+//      named columns, additively, so every file you ship keeps loading.
+//   3. The game layer's files — game.*, hero.*, combat.*, ai.*, feel.*,
+//      hud.*, sound.*: your game's rules, yours to rewrite. The
+//      services under them hold still until measurement says otherwise
+//      (arena, table, entity, gametime, tilemap, audio, camera,
+//      framebuffer — the engine Parts 1-4 built).
+//   4. Where to measure — `frame.*`'s account and the closing report.
+//      Name the hotspot before you fix it (lesson 098's discipline):
+//      measure, fix what you measured, report what you did not.
+//
+// And one discipline more: an idea outside your game's frozen checklist
+// is extras — recorded in your ledger, never added. Lesson 102 kept
+// that rule for this course; keeping it for your game is now your job.
 #ifndef GAME_H
 #define GAME_H
 
```

## Exercices

Les deux dernières du cours, et toutes deux sont les premiers pas de votre jeu
plutôt que de celui-ci. Chacune se termine par sa solution — un diff contre
l'état final de cette leçon plus une visite guidée — juste après l'énoncé.

### Exercice 1 — Le premier changement de votre jeu *(extend-the-code)*

La passation est réelle quand le moteur se plie au jeu de quelqu'un d'autre.
Faites le premier changement de votre jeu, à travers les coutures : au moins un
changement de **données** (un type à vous sous forme de ligne de table — il peut
réutiliser l'art et les sons du cours jusqu'à ce que les vôtres existent) et au
moins un changement de **règle** dans les fichiers de la couche jeu (quelque
chose que l'ancien jeu faisait et que le vôtre ne fait pas — une règle de score,
un poids de comportement, une forme de vague). Règles à respecter : aucun
fichier de service ne change, le build reste sans avertissement, le contrôle de
frontière reste propre, et tout fichier livré par le cours continue de se
charger. Prouvez ensuite votre changement dans une exécution et montrez les
lignes de transcription qui l'attestent — et, à côté, le `git diff --stat` qui
prouve quels fichiers votre changement était *autorisé* à toucher.

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-103/ex1.md)

### Exercice 2 — La carte de passation de votre machine *(port-to-your-own-machine)*

Exécutez le moteur — avec le changement de votre exercice 1 dedans — sur votre
machine : une vraie fenêtre, un vrai périphérique sonore si vous en avez un.
Produisez la carte de passation : le rapport de budget de frames tel que votre
machine l'imprime, avec le nom de votre machine là où figure le nôtre (le plus
petit changement qui confirme est le nom ; le patch montre où il vit), ce que
votre machine a changé dans les nombres par rapport au rapport de notre machine
(nommez les lignes qui ont bougé et pourquoi), et la première entrée du registre
des extras de votre jeu — une idée que votre jeu vient d'avoir, **consignée, pas
ajoutée**. Dites ensuite ce que la carte vous demande de faire en premier. La
carte est le dernier artefact de ce cours et le premier artefact de la discipline
de mesure de votre jeu.

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-103/ex2.md)

---

**Partie :** [Partie 5 — le jeu](../../index.md) ·
**Précédente :** [Leçon 102 — la rétrospective : notre moteur face aux moteurs réels](lesson-102-retrospective.md) ·
**Suivante :** [l'accueil du cours](../../index.md) ·
**Étiquette de code :** [`lesson-103`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-103)

*Page traduite de la version anglaise `book/lessons/part-5/lesson-103-your-game.md`,
révision `cfeaecd`.*

<!-- translation-source: book/lessons/part-5/lesson-103-your-game.md @ cfeaecd -->
