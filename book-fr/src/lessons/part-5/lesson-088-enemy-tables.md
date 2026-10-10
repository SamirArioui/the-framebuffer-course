# Leçon 088 — les tables d'archétypes des ennemis

{{#include ../../stability-horizon.md}}

## Prose

Le monde a un héros, un slime et un combat qui fonctionne. Ce qu'il n'a pas,
ce sont des ennemis — les trois types et le boss autour desquels le jeu a été
conçu. Cette leçon les amène, et elle porte entièrement sur *comment* ils
arrivent : **en lignes.** Aucun code d'ennemi, aucun attribut par type, aucun
switch sur le nom d'un type — l'effectif (roster) est de la donnée, et la
donnée est le format grandi de la leçon 087 faisant exactement ce pour quoi il
a grandi.

### L'effectif, ce sont des lignes

`assets/enemies.txt` est l'effectif du jeu. Son en-tête nomme les colonnes que
ses lignes utilisent — treize des quinze que le format connaît — et chaque
fait par type est une valeur dans une ligne :

```
name x y facing speed health sprite damage rate fires behavior wave count
bat 560 72 2 160 2 assets/bat.ppm 1 60 bolt chase 1 2
wisp 640 336 2 120 1 assets/wisp.ppm 1 20 bolt flee 1 1
spitter 120 400 0 96 3 assets/spitter.ppm 1 30 shell keep 2 2
golem 384 96 1 72 8 assets/golem.ppm 2 30 shell boss 3 1
```

Lisez les lignes comme le document de conception du jeu : le **bat** est
rapide et fragile et poursuit ; le **wisp** est fragile et fuit ; le
**spitter** est plus coriace, garde ses distances et crache des `shell` ; le
**golem** est le boss — lent, lourd (huit points de vie, deux dégâts), et son
`behavior` nomme la seule chose qui n'appartienne qu'à lui : le schéma. Leurs
attaques suivent aussi (`damage`, `rate`, `fires` — la leçon 090 les fera
tirer), et leurs faits de vague aussi (`wave`, `count` — la leçon 091 les
dépensera). Rien à propos de ces quatre types n'existe nulle part dans le
code.

Les colonnes qu'un fichier ne nomme *pas* se tiennent aux valeurs par défaut
du format — l'effectif ne nomme ni colonne `accel` ni colonne `range`, et
l'exécution le dit :

```
engine: table assets/enemies.txt: 4 definitions
engine: def bat: x 560 y 72 facing 2 speed 160 health 2 sprite assets/bat.ppm accel 120 damage 1 rate 60 fires bolt range 0 behavior chase wave 1 count 2
engine: def wisp: x 640 y 336 facing 2 speed 120 health 1 sprite assets/wisp.ppm accel 120 damage 1 rate 20 fires bolt range 0 behavior flee wave 1 count 1
engine: def spitter: x 120 y 400 facing 0 speed 96 health 3 sprite assets/spitter.ppm accel 120 damage 1 rate 30 fires shell range 0 behavior keep wave 2 count 2
engine: def golem: x 384 y 96 facing 1 speed 72 health 8 sprite assets/golem.ppm accel 120 damage 2 rate 30 fires shell range 0 behavior boss wave 3 count 1
```

`accel 120` et `range 0` sur chaque ligne — les valeurs par défaut, portées
là où le fichier reste silencieux.

### Chaque ennemi porte les valeurs de sa ligne

Une ligne devient une entité comme chaque entité depuis la leçon 073 :
`EntityFromDef` copie les valeurs de la ligne dans des champs nommés, et le
jeu lit et écrit ces champs directement. Le rapport de l'apparition imprime
chaque entité dans les mêmes mots que sa définition, si bien que le portage
se vérifie à l'œil contre le fichier ci-dessus :

```
engine: entity bat: x 560 y 72 facing 2 speed 160 health 2 sprite 16x16 accel 120 damage 1 rate 60 fires bolt range 0 behavior chase wave 1 count 2
engine: entity wisp: x 640 y 336 facing 2 speed 120 health 1 sprite 16x16 accel 120 damage 1 rate 20 fires bolt range 0 behavior flee wave 1 count 1
engine: entity spitter: x 120 y 400 facing 0 speed 96 health 3 sprite 16x16 accel 120 damage 1 rate 30 fires shell range 0 behavior keep wave 2 count 2
engine: entity golem: x 384 y 96 facing 1 speed 72 health 8 sprite 16x16 accel 120 damage 2 rate 30 fires shell range 0 behavior boss wave 3 count 1
engine: roster: 4 enemies from the table's rows, live 6 of 64
```

Ligne pour ligne : les mêmes positions, les mêmes vitesses, les mêmes points
de vie, les mêmes armes, les mêmes comportements, les mêmes faits de vague.
Le seul champ qui change de forme est le sprite — l'entité porte l'art de sa
ligne *chargé*, l'image que le chemin nommait, et imprime ses dimensions.
L'entité est la ligne, vivante.

### Aucune copie par type dans le code

L'apparition est une seule boucle sur les lignes de la table. Il n'y a aucun
`if (isBat)`, aucun cas particulier pour le golem, aucune constante nulle
part dans `src/` qui contienne une vitesse, des points de vie ou des dégâts
appartenant à un type. Les seuls noms que le code connaît sont ceux que le
*jeu* demande par leur nom — la ligne du héros, les lignes des armes — et la
recherche d'une ligne n'est pas une copie d'attribut.

L'affirmation se prouve le plus facilement en la dépensant : ajoutez un
cinquième type au fichier et ne changez rien d'autre. Depuis une copie
jetable de l'exécution, une ligne de plus —

```
swarmling 240 160 0 200 1 assets/spitter.ppm 1 90 bolt chase 1 3
```

— et l'exécution fait grandir un ennemi, portant les valeurs de sa ligne
comme tous les autres :

```
engine: table assets/enemies.txt: 5 definitions
engine: def swarmling: x 240 y 160 facing 0 speed 200 health 1 sprite assets/spitter.ppm accel 120 damage 1 rate 90 fires bolt range 0 behavior chase wave 1 count 3
engine: entity swarmling: x 240 y 160 facing 0 speed 200 health 1 sprite 16x16 accel 120 damage 1 rate 90 fires bolt range 0 behavior chase wave 1 count 3
engine: roster: 5 enemies from the table's rows, live 7 of 64
```

`git diff --stat src/` est vide : un nouvel ennemi est une nouvelle ligne.
C'est tout l'intérêt de la table d'archétypes, et c'est pourquoi les faits
par type n'ont jamais migré dans le code quand les types sont devenus
intéressants.

### Ce que cette exécution a vérifié, et ce qu'elle n'a pas vérifié

- **Chaque ennemi porte les valeurs de sa ligne** — l'apparition imprime
  chaque entité dans les mêmes mots que sa définition, et chaque champ
  correspond : positions, vitesses, points de vie, armes, comportements,
  faits de vague.
- **Aucune copie par type des attributs n'apparaît dans le code** —
  l'apparition est une seule boucle sur les lignes ; un cinquième type
  ajouté comme une simple ligne apparaît en portant ses valeurs, avec un
  diff vide dans `src/`.

Ce que cette exécution n'a **pas** vérifié, c'est que les ennemis fassent
quoi que ce soit. Leurs valeurs de `behavior` sont portées, pas encore mises
en œuvre — la leçon 089 transforme `chase`, `keep` et `flee` en mouvement à
travers le mover, et `boss` en le schéma qui les compose. Leurs `damage`,
`rate` et `fires` sont portés eux aussi, et la leçon 090 fait tirer un ennemi
armé. L'effectif immobile sur le `x` et le `y` d'une ligne, c'est exactement
autant de jeu que cette leçon le promettait : la donnée, portée, en attente.

## Étape de code

Un changement : l'effectif, en lignes. `assets/enemies.txt` est nouveau — les
trois types et le boss, chaque fait par type valeur de sa propre ligne — et
`assets/bat.ppm`, `assets/wisp.ppm`, `assets/spitter.ppm`, `assets/golem.ppm`
sont leur art (quatre images 16×16 à clé magenta, une couleur et un jeu
d'yeux par type). `src/entity.h/.cpp` portent les deux derniers faits de la
ligne (`wave`, `count`) comme ils portent tous les autres ; `src/main.cpp`
charge la table de l'effectif, fait apparaître une entité par ligne à travers
le magasin, et imprime chaque entité à côté de sa définition. Aucun autre
fichier ne change — et c'est la leçon. Son état final est étiqueté
`lesson-088`.

```diff
diff --git a/assets/enemies.txt b/assets/enemies.txt
new file mode 100644
index 0000000..ffd931d
--- /dev/null
+++ b/assets/enemies.txt
@@ -0,0 +1,5 @@
+name x y facing speed health sprite damage rate fires behavior wave count
+bat 560 72 2 160 2 assets/bat.ppm 1 60 bolt chase 1 2
+wisp 640 336 2 120 1 assets/wisp.ppm 1 20 bolt flee 1 1
+spitter 120 400 0 96 3 assets/spitter.ppm 1 30 shell keep 2 2
+golem 384 96 1 72 8 assets/golem.ppm 2 30 shell boss 3 1
diff --git a/src/entity.cpp b/src/entity.cpp
index 77cdd42..98511cc 100644
--- a/src/entity.cpp
+++ b/src/entity.cpp
@@ -27,6 +27,8 @@ Entity EntityFromDef(const EntityDef &def)
     entity.rate = def.rate;
     entity.range = def.range;
     entity.behavior = def.behavior;
+    entity.wave = def.wave;
+    entity.count = def.count;
     return entity;
 }
 
diff --git a/src/entity.h b/src/entity.h
index c214bb8..a6872fb 100644
--- a/src/entity.h
+++ b/src/entity.h
@@ -54,6 +54,9 @@ struct Entity {
                                   never hits its owner */
     int behavior;              /* BehaviorKind, its row's: what this
                                   entity does each frame */
+    int wave;                  /* lesson 088: which wave spawns this
+                                  kind — carried like every row value */
+    int count;                 /* and how many join that wave */
     double cooldown;           /* seconds until it may fire again */
 };
 
diff --git a/src/main.cpp b/src/main.cpp
index 58ff0bd..6a3320a 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -163,6 +163,20 @@ static void PrintDefs(const char *path, const EntityTable &table)
     }
 }
 
+/* Lesson 088: one live entity, carrying its row's values — printed in
+   the same words as the definition above, so the carrying is checkable
+   by eye against the file's rows. The sprite prints as its dimensions
+   because the entity carries the row's art *loaded* — the image, not
+   the path that named it. */
+static void PrintEntity(const char *kind, const Entity &e)
+{
+    std::printf("engine: %s %s: x %d y %d facing %d speed %d health %d sprite %dx%d accel %d damage %d rate %d fires %s range %d behavior %s wave %d count %d\n",
+                kind, e.name, (int)e.x, (int)e.y, e.facing, e.speed, e.health,
+                e.sprite->width, e.sprite->height, e.accel, e.damage, e.rate,
+                e.fires[0] ? e.fires : "none", e.range, BehaviorName(e.behavior),
+                e.wave, e.count);
+}
+
 int Run(void)
 {
     platform::WindowResult opened =
@@ -242,10 +256,13 @@ int Run(void)
        are rows that name the projectile kind they fire and carry their
        rate and damage; the projectile kinds are rows a fired shot is an
        entity of. Each file's header names the columns it uses — and only
-       those; what it leaves unnamed sits at the format's defaults. */
-    EntityTable weapons, shots;
+       those; what it leaves unnamed sits at the format's defaults.
+       Lesson 088: and the enemy roster — the three types and the boss,
+       every per-type fact its own row's value. */
+    EntityTable weapons, shots, foes;
     if (!LoadRunTable(arena, "assets/weapons.txt", weapons) ||
-        !LoadRunTable(arena, "assets/projectiles.txt", shots)) {
+        !LoadRunTable(arena, "assets/projectiles.txt", shots) ||
+        !LoadRunTable(arena, "assets/enemies.txt", foes)) {
         platform::CloseWindow(opened.window);
         ArenaRelease(arena);
         return 1;
@@ -260,10 +277,12 @@ int Run(void)
     PrintDefs("assets/entities.txt", table);
     PrintDefs("assets/weapons.txt", weapons);
     PrintDefs("assets/projectiles.txt", shots);
+    PrintDefs("assets/enemies.txt", foes);
 
     /* Lesson 073: the definitions' art, loaded at startup. A row that
        names no sprite (a weapon row) has no art and needs none. */
-    if (!LoadRunArt(arena, table) || !LoadRunArt(arena, shots)) {
+    if (!LoadRunArt(arena, table) || !LoadRunArt(arena, shots) ||
+        !LoadRunArt(arena, foes)) {
         platform::CloseWindow(opened.window);
         ArenaRelease(arena);
         return 1;
@@ -341,6 +360,25 @@ int Run(void)
     std::printf("engine: world: %d entities from the table's rows, live %d of %d\n",
                 created, store.live, ENTITY_CAP);
 
+    /* Lesson 088: the enemy roster is data. The three types and the
+       boss are rows of the game's table; each row becomes one entity,
+       carrying its row's values in named fields the game reads
+       directly. A new row is a new enemy — the run has no per-kind code
+       to grow, and no per-type copy of any attribute to keep honest. */
+    for (int i = 0; i < foes.count; ++i) {
+        EntityResult made = EntityCreate(store, foes.rows[i]);
+        if (made.error != ENTITY_OK) {
+            std::fprintf(stderr, "engine: the store refused %s\n",
+                         foes.rows[i].name);
+            platform::CloseWindow(opened.window);
+            ArenaRelease(arena);
+            return 1;
+        }
+        PrintEntity("entity", *made.entity);
+    }
+    std::printf("engine: roster: %d enemies from the table's rows, live %d of %d\n",
+                foes.count, store.live, ENTITY_CAP);
+
     /* Lesson 087: weapons are rows. The hero starts armed with the
        weapons table's first row; the number keys arm the rest (HeroFire).
        The demonstration stand-in arms the foe with the second row — its
```

## Exercices

Deux défis plus grands. Chacun se termine par sa solution — un diff contre
l'état final de cette leçon plus une visite guidée — après l'énoncé.

### Exercice 1 — Les types que personne n'a encore faits *(extend-the-code)*

Concevez les trois prochains types de l'effectif en **lignes seulement** : un
type de nuée qui arrive en nombre (son `count` le dit), un sprinteur rapide et
fragile, et un tank lent et lourd avec des points de vie à revendre. Donnez à
chacun une ligne et un art à vous (ou réutilisez l'art déjà sur le disque) —
puis prouvez l'affirmation de la leçon : l'exécution doit faire apparaître
chacun en portant les valeurs de sa ligne, et `git diff --stat src/` doit être
vide. Citez les lignes d'effectif de l'exécution et le diff. Qu'est-ce que les
colonnes vous ont laissé exprimer sur vos trois types que du code aurait
caché ?

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-088/ex1.md)

### Exercice 2 — Pourquoi le boss ne mérite pas de code *(explain-in-prose)*

L'instinct est ancien et fort : *le boss est spécial — donnez-lui sa propre
fonction d'update, ses propres champs, son propre fichier même.* Argumentez
contre lui avec vos propres mots, en utilisant les arguments du cours : ce que
coûte la règle de marche de la leçon 075 (chaque entité vivante exactement une
fois, une seule forme par entité) quand cinq types ont cinq fonctions
d'update ; ce qui arrive à cette forme quand les vagues de la leçon 091 font
apparaître les mêmes types par douzaines ; et ce que « les attributs par type
sont de la donnée » protège qu'une constante de code ne peut pas protéger.
Puis construisez son meilleur argument (steelman) : nommez la *seule* chose
dont un boss a réellement besoin qu'une ligne ne peut pas porter — et dites
quelle leçon de ce lot la lui donne à la place.

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-088/ex2.md)

---

**Partie :** [Partie 5 — le jeu](../../index.md) ·
**Précédente :** [Leçon 087 — les projectiles et les deux armes](lesson-087-projectiles-weapons.md) ·
**Suivante :** [Leçon 089 — l'IA ennemie](lesson-089-enemy-ai.md) ·
**Étiquette de code :** [`lesson-088`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-088)

*Page traduite de la version anglaise `book/lessons/part-5/lesson-088-enemy-tables.md`,
révision `ac121ed`.*

<!-- translation-source: book/lessons/part-5/lesson-088-enemy-tables.md @ ac121ed -->
