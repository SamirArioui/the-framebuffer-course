# Leçon 026 — la base de code naît

{{#include ../../stability-horizon.md}}

## Prose

La partie 0 est terminée, et ses quatre programmes ont servi leur but :
`wordcount` a enseigné le pipeline et la mémoire, `ds-kit` la disposition et
l'édition de liens, `paint` les octets et les pixels, `snek` les boucles et
l'état. Aucun d'eux n'est le moteur. Aujourd'hui la base de code du moteur
naît — à partir d'un fichier vierge, dans un dépôt dont le `src/` est vide
depuis l'échafaudage — et elle grandit d'ici jusqu'à la fin du cours, une
étape de code par leçon, dans un sous-ensemble de C++ que vous pouvez lire
comme de l'assembleur.

### La naissance

Trois choses se produisent dans une seule étape de code, commitée avec cette
prose et étiquetée `lesson-026` :

1. **`src/main.cpp` apparaît.** Il contient un `main` vierge — le programme
   entier est `int main(void) { return 0; }` sous un commentaire d'en-tête. Il
   se compile, il se lie, il s'exécute, il se termine avec le code 0 sans
   aucune sortie. C'est volontaire : chaque ligne que le moteur gagne à partir d'aujourd'hui apparaît dans le diff d'une leçon, en partant de rien.
2. **`build.sh` entre en service.** Le script dort dans le dépôt depuis la
   pose de l'échafaudage ; aujourd'hui il a enfin quelque chose à compiler.
   C'est la construction en une commande pour tout le reste du cours — Make et
   CMake ne sont jamais au programme.
3. **`sandbox/` est supprimé, en bloc, dans la même étape.** C'est un abandon
   voulu, pas une perte : les programmes du bac à sable étaient jetables par
   contrat (« sandbox jetable de la partie 0 »), et à partir de la partie 1,
   le code vit dans `src/` et nulle part ailleurs. Chaque état de la partie 0
   reste récupérable — les étiquettes conservent tout. Si vous voulez le bac à
   sable de retour à un moment ou un autre :

   ```
   git checkout lesson-025 -- sandbox/
   ```

   La bannière d'horizon ci-dessus nomme déjà la bascule : les états de la
   partie 0 restaurent `sandbox/`, les états du moteur de la partie 1
   restaurent `src/`.

### La construction

`./build.sh` est délibérément assez petit pour se lire d'une traite. Il trouve
chaque source C et C++ sous `src/`, compile chacune vers son propre fichier
objet sous `build/obj/`, et lie le résultat en `build/game`. Rien de la
construction n'est caché dans un makefile généré ; quand la base de code gagne
un fichier, la construction s'en aperçoit, parce qu'elle n'est qu'un `find`
sur `src/`.

Les options sont celles sur lesquelles la leçon 025 s'est achevée, et elles
signifient exactement ce qu'elles y signifiaient :

```
-std=c++17 -O0 -g -Wall -Wextra
```

`-std=c++17` épingle la version du langage — le compilateur est d'accord avec
ce livre sur ce que signifie le code (et la loi du langage ci-dessous est
écrite contre cette norme). `-O0` tient l'optimiseur hors du chemin pendant
que le code se lit ; `-g` conserve les informations de débogage pour gdb ;
`-Wall -Wextra` gardent les avertissements actifs, et la construction reste à
zéro avertissement de la première ligne du moteur à la dernière.

La construction et l'exécution du jour, en entier :

```
$ ./build.sh
build: compiling 1 source(s) from src/
  CC  src/main.cpp
  LD  build/game
build: OK (1 source(s) compiled -> build/game)
$ ./build/game
$ echo $?
0
```

### La loi du langage

Le moteur s'apprête à porter tout le reste du cours — de l'ordre d'une
centaine de leçons de code empilées sur le fichier vierge d'aujourd'hui. Son
langage n'est donc pas « du C++ » en général. C'est un **sous-ensemble**,
choisi par la politique que la leçon 025 a enseignée, la machine sous chaque
décision restant en vue :

> **Une caractéristique du langage n'est admise que si nous savons expliquer
> en quoi elle se compile.**

Cette leçon consigne cette politique comme la loi du langage du moteur. Chaque
étape de code à partir d'ici lui obéit, et la politique est appliquée comme la
leçon 025 le décrivait : **par relecture, pas par outillage**. Rien ne vous
empêche d'enfreindre la loi dans votre propre copie ; dans le cours, c'est un
relecteur qui l'arrête.

**Les caractéristiques admises.** Exactement les cinq que la leçon 025 a
admises, chacune déjà expliquée par sa génération de code :

- **les références** — un pointeur que le compilateur déréférence pour vous ;
- **la surcharge** — un nom, plusieurs fonctions, séparées par le name
  mangling ;
- **les espaces de noms** — des noms qualifiés, coût nul à l'exécution ;
- **`constexpr`** — des valeurs que le compilateur plie avant que le programme
  existe ;
- **les classes avec vtables** — et toute la surface de classes qu'utilisait
  la leçon 025 tient dans ce seul point : constructeurs à listes
  d'initialisation de membres, contrôle d'accès `public:`/`private:`, héritage
  simple, fonctions virtuelles et purement virtuelles, et fonctions membres
  `const`. Chacune d'elles se compilait vers la simple disposition d'une
  struct, de simples fonctions, et les tables que vous avez regardé `nm`
  imprimer. La loi admet la surface qu'elle sait expliquer, pas une liste de
  mots-clés.

**Les espaces de noms.** Le code du moteur vit dans `namespace engine`.
`main` est l'unique fonction globale — l'exécution C++ cherche `::main` et
rien d'autre — et il transmet un appel dans l'espace de noms. La leçon 025
s'est achevée sur exactement ce motif avec `snek::Run` ; le moteur le garde
définitivement.

**Le nommage.** Les membres privés portent un souligné final : `commands_`,
`grid_`. Cette convention a été adoptée dans la leçon 025 et elle vaut pour le
moteur.

**Les modèles** restent hors du moteur — avec une seule dérogation précise :
ils peuvent apparaître dans du **code d'outillage clairement étiqueté** (les
outils de construction et de vérification sous `tools/`), jamais dans `src/`.
La leçon 025 disait que les modèles ne sont « jamais admis dans le code du
jeu » ; la formulation de la loi est celle-ci, parce qu'un outil n'est pas du
code de jeu et qu'un modèle dans un outil clairement étiqueté n'est pas une
échappatoire dans le moteur.

**Restent dehors jusqu'à ce que la politique les admette :** les exceptions,
les conteneurs et algorithmes de la STL, `new`/`delete`, et le RTTI
(`dynamic_cast`, `typeid`). Ce n'est pas de l'ascétisme — c'est le même test,
« expliquer en quoi ça se compile », appliqué à des caractéristiques dont le
moteur possédera lui-même la mécanique. Les grands tampons du moteur
viendront de réservations de mémoire au niveau de l'OS plus loin dans cette
partie, et du code qui possède sa mémoire de cette façon n'a aucune raison
d'entretenir un trafic de `new`/`delete`. (La leçon 025 listait les mêmes
exclusions pour `snek` ; rien n'a changé à la frontière.)

**La norme est C++17**, épinglée par la ligne de construction depuis la leçon
025.

**Où la loi mord.** Quand une étape de code voudrait ajouter une construction
hors de cette liste, la leçon l'admet d'abord — en expliquant en quoi elle se
compile — ou la construction reste dehors. Toute la procédure est là. Il n'y a
pas de `law.h`, pas de vérificateur, pas d'application dans la construction ;
la construction n'est pas l'endroit où vit une politique de langage. L'endroit
où elle vit, c'est ici, dans les leçons, et dans la relecture qui les lit.

### Ce que le fichier vierge achète

Un fichier vierge n'est pas une leçon vide. C'est la ligne de base qui rend
chaque affirmation ultérieure vérifiable : le diff de la prochaine leçon est
la couture plateforme et rien d'autre ; le diff de celle d'après est une
fenêtre ; et quand le moteur sera à cent fichiers empilés, `git diff lesson-026
lesson-027` est encore exactement une étape que vous pouvez lire. La continuité
est la machinerie que la partie 0 a construite — un historique linéaire, une
étiquette par leçon, prose et code commités ensemble — et elle est maintenant
pointée sur le moteur.

## Étape de code

Un seul changement pour cette leçon : `src/main.cpp` naît comme un `main`
vierge, `src/.gitkeep` — le marqueur qui maintenait le répertoire vide — meurt
avec lui, et `sandbox/` est supprimé dans la même étape. Son état final est
étiqueté `lesson-026`.

```diff
diff --git a/src/.gitkeep b/src/.gitkeep
deleted file mode 100644
index e69de29..0000000
diff --git a/src/main.cpp b/src/main.cpp
new file mode 100644
index 0000000..7ecbde3
--- /dev/null
+++ b/src/main.cpp
@@ -0,0 +1,13 @@
+// main.cpp — the engine, born.
+//
+// Lesson 026: the codebase is born from a blank file. This file obeys the
+// language law documented in the lesson: the admitted C++ subset of
+// lesson 025 — references, overloading, namespaces, constexpr, and classes
+// with vtables — and nothing else. `main` is the one global function the
+// runtime looks up; every engine symbol that follows lives in
+// `namespace engine`.
+
+int main(void)
+{
+    return 0;
+}
```

La suppression de `sandbox/` est le reste de l'étape — 13 fichiers et 1231
lignes de partie 0 qui passent la porte ensemble, résumés plutôt que collés :

```
 sandbox/README.md             |  29 ---
 sandbox/ds-kit/.gitkeep       |   0
 sandbox/ds-kit/dynarray.c     |  58 -----
 sandbox/ds-kit/dynarray.h     |  24 ---
 sandbox/ds-kit/hashtable.c    |  86 --------
 sandbox/ds-kit/hashtable.h    |  31 ---
 sandbox/ds-kit/main.c         |  81 -------
 sandbox/paint/.gitkeep        |   0
 sandbox/paint/paint.c         | 284 -------------------------
 sandbox/snek/.gitkeep         |   0
 sandbox/snek/snek.cpp         | 482 ------------------------------------------
 sandbox/wordcount/.gitkeep    |   0
 sandbox/wordcount/wordcount.c | 156 --------------
 13 files changed, 1231 deletions(-)
```

## Exercices

Deux extensions « faites-les vôtres ». Chacune se termine par sa solution — un
diff contre l'état final de cette leçon, plus une visite guidée — après
l'énoncé.

### Exercice 1 — Le moteur dit son nom *(extend-the-code)*

Faites vôtre la base de code qui vient de naître : donnez une voix au moteur,
dans la forme qu'exige la loi. Placez les premiers symboles du moteur dans
`namespace engine` — un nom `constexpr` et une version `constexpr` — faites
annoncer le moteur par `Run` sur une ligne au démarrage, et gardez `main`
comme l'unique global qui transmet dans l'espace de noms, exactement le motif
sur lequel la leçon 025 s'est achevée. Compilez, exécutez, puis vérifiez vos
ajouts contre la loi ligne par ligne : quelle caractéristique admise chaque
nouvelle ligne utilise-t-elle, et vers quoi s'est-elle compilée ?

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-026/ex1.md)

### Exercice 2 — Deux unités de traduction *(extend-the-code)*

Les prochaines leçons font grandir cette base de code fichier par fichier ;
faites donc vôtre cette forme dès maintenant : sortez le code du moteur de
`main.cpp` vers sa propre unité de traduction. Un en-tête déclare ce sur quoi
`main.cpp` peut compter, un fichier source définit `engine::Run`, et `main.cpp`
ne garde rien d'autre que le `main` global. Compilez avec `./build.sh` — il
doit compiler deux sources et lier un programme — et regardez ce que
`build/obj/` contient ensuite. Rien ne change dans le script de construction
pour tout cela ; expliquez pourquoi.

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-026/ex2.md)

---

**Partie :** [Partie 1 — la couche plateforme](../../index.md) ·
**Précédente :** [Leçon 025 — le sous-ensemble C++ : classes et vtables](../part-0/lesson-025-cpp-subset.md) ·
**Suivante :** [Leçon 027 — la couture plateforme et la première fenêtre X11](lesson-027-first-window.md) ·
**Étiquette de code :** [`lesson-026`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-026)

*Page traduite de la version anglaise `book/lessons/part-1/lesson-026-birth.md`, révision `100464c`.*

<!-- translation-source: book/lessons/part-1/lesson-026-birth.md @ 100464c -->
