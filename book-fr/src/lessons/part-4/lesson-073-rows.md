# Leçon 073 — les entités en lignes

{{#include ../../stability-horizon.md}}

## Prose

La leçon 072 a laissé le jeu tenant une table de définitions et rien d'autre :
deux lignes de faits, chargées entières, complètes ou nommées. Les faits ne sont
pas des choses. Le héros à l'écran est toujours le sprite de la démo déplacé par
les propres `double`s de la démo, et la ligne de la table qui lui correspond est
de la donnée que rien n'a lue. L'idée de cette leçon est donc le pont entre les
deux : **une entité est une ligne que le jeu a créée à partir d'une définition**
— une chose vivante qui porte l'identité et les attributs de cette définition
dans des champs nommés que le jeu lit directement. Et écrit directement, ce qui
est la moitié qui compte.

### Une définition porte ses faits

Créer une entité doit répondre à toutes les questions la concernant à partir de
la définition seule — jamais à partir de suppositions sur le fichier, et jamais
à partir d'une seconde source de vérité. Une définition porte donc ce dont la
création a besoin :

```cpp
struct EntityDef {
    char name[TABLE_NAME_MAX];   /* the definition's identity */
    int x, y;                    /* where it starts, in world pixels */
    int facing;                  /* 0 right, 1 down, 2 left, 3 up */
    int speed;                   /* world pixels per second */
    int health;                  /* points */
    char sprite[TABLE_PATH_MAX]; /* the art file it draws */
    const Sprite *image;         /* that art, loaded at startup */
};
```

Sept de ces champs sont ceux du fichier, remplis par l'analyse. Le huitième —
`image` — est le seul fait que le fichier énonce comme un *nom* : la colonne
sprite avec laquelle personne ne peut dessiner. L'exécution le résout au
démarrage, en chargeant l'art de chaque définition à travers le chargeur que la
leçon 044 a écrit et en remettant son image à la définition, si bien qu'à partir
de là une définition connaît son propre art comme elle connaît sa propre
vitesse. Cette séparation est délibérée et c'est encore l'habitude des assets :
le fichier nomme ce qu'il veut ; le moteur le charge une fois et le garde (dans
l'arena, comme tout autre asset) ; la donnée porte le résultat.

### L'entité est une copie, pas une vue

`EntityFromDef` prend une définition et répond avec une entité :

```cpp
struct Entity {
    char name[TABLE_NAME_MAX]; /* the definition's identity, carried */
    int x, y;                  /* where it is, in world pixels */
    int facing;                /* 0 right, 1 down, 2 left, 3 up */
    int speed;                 /* world pixels per second */
    int health;                /* points */
    const Sprite *sprite;      /* the art it draws, from its row */
};
```

Les mêmes faits, un cran plus profond dans la structure — et des *copies* de
ceux-ci, chacune. Ce n'est pas de la comptabilité. Le jeu écrit ces champs : la position
du héros change à chaque frame où son joueur maintient une touche, sa vie change
quand quelque chose le frappe. Si l'entité était une vue sur la définition — des
pointeurs dans les lignes de la table — alors déplacer un héros réécrirait la
ligne, et tout autre héros créé depuis cette ligne bougerait avec lui, et les
valeurs énoncées par le fichier cesseraient d'être les valeurs énoncées par le
fichier avant la fin de l'exécution. La définition est l'endroit d'où une entité
*vient* ; l'entité est ce dans quoi elle *vit*. Deux entités d'un même type
commencent identiques et divergent dès leur première frame.

Les champs sont nommés et simples à dessein (le choix du design, conservé) : le
jeu lit `hero.speed` et écrit `hero.x` comme n'importe quelles autres valeurs —
pas de sac clé/valeur, pas de recherche par chaîne par attribut par frame. Quand
la partie 5 donnera un comportement aux ennemis, ce comportement lira ces mêmes
champs ; il n'y a aucune hiérarchie de structures par type à faire grandir.

### Demander une définition

Le jeu ne sait pas quelles lignes un fichier contient — c'est l'affaire du
fichier — donc en demander une est une requête avec une réponse typée :

```cpp
DefResult TableFind(const EntityTable &table, const char *name);
```

`DEF_OK` et la définition, ou `DEF_UNKNOWN` et rien. Une définition que la table
ne contient pas est **rapportée comme une valeur**, jamais répondue par « la
première ligne » ou « quelque chose d'approchant » : une entité aux attributs
supposés est pire qu'un échec, parce qu'elle tourne. L'exécution vérifie cet
échec exprès — en demandant `"dragon"` — et rapporte
`engine: table: "dragon" -> unknown`. La même forme que chaque chargement de ce
moteur : la réponse ou le nom de ce qui manque.

### Ce que cette exécution a vérifié, et ce qu'elle n'a pas vérifié

D'une vraie exécution de l'état final de cette leçon sur cette machine :

- **Une entité créée à partir d'une définition porte les valeurs de la table.**
  L'exécution imprime la ligne de la définition et l'entité côte à côte :
  `engine: def hero: x 312 y 232 facing 0 speed 240 health 3 sprite
  assets/sprite.ppm` et `engine: entity hero: x 312 y 232 facing 0 speed
  240 health 3 sprite 16x16`. Champ pour champ, les mêmes valeurs — avec le
  *nom* de la colonne sprite devenu l'art que le jeu peut dessiner (16×16, les
  dimensions propres du fichier).
- **Une définition que la table ne contient pas est un échec typé.** La requête
  `"dragon"` de l'exécution répond `unknown`, et la requête `hero` de
  l'exécution elle-même — faite de la même façon — terminerait par son nom si la
  ligne manquait.
- **La définition garde les valeurs du fichier.** L'entité est une copie ; les
  valeurs de la ligne sont ce que le fichier a énoncé, avant et après qu'une
  entité en est créée (l'exercice ci-dessous surveille exactement cela).

Ce que cette leçon ne fait **pas**, c'est garder l'entité quelque part. `hero`
est une locale de l'exécution : une entité, pas de place pour une seconde, et
rien qui les parcourt. C'est le travail de la leçon suivante — un magasin fixe
d'entités vivantes, pour que le jeu en tienne beaucoup sans inventer un stockage
par fonctionnalité — et rien ici ne bouge encore non plus (la leçon 076 donne au
héros son joueur).

## Étape de code

Un seul changement pour cette leçon, des définitions aux entités : `src/entity.h`
/ `src/entity.cpp` accueillent `Entity` et `EntityFromDef` — la ligne copiée en
une vie, des champs nommés que le jeu lit et écrit. `src/table.h` /
`src/table.cpp` accueillent `TableFind`, la recherche typée qu'un jeu fait quand
il veut un type d'entité, et `EntityDef` accueille le seul champ que l'exécution
remplit : l'image de la définition, chargée au démarrage depuis le fichier nommé
par la colonne sprite. `src/main.cpp` accueille le démarrage de l'exécution en
conséquence — l'art de chaque définition chargé et remis à sa définition,
l'entité du héros créée depuis sa ligne et rapportée champ pour champ, et la
requête au nom inconnu répondue comme une valeur. Le sprite propre de la démo,
la carte et la boucle ne sont pas touchés. Son état final est étiqueté
`lesson-073`.

```diff
diff --git a/src/entity.cpp b/src/entity.cpp
new file mode 100644
index 0000000..3bef96c
--- /dev/null
+++ b/src/entity.cpp
@@ -0,0 +1,26 @@
+// entity.cpp — entities from definitions: the row, copied into a life.
+//
+// Lesson 073: creating an entity is a copy. The definition keeps the
+// values the file stated — every entity of a kind starts from the same
+// row — and the entity carries its own, which the game is then free to
+// change.
+
+#include "entity.h"
+
+namespace engine {
+
+Entity EntityFromDef(const EntityDef &def)
+{
+    Entity entity = {};
+    for (int i = 0; i < TABLE_NAME_MAX; ++i)
+        entity.name[i] = def.name[i];
+    entity.x = def.x;
+    entity.y = def.y;
+    entity.facing = def.facing;
+    entity.speed = def.speed;
+    entity.health = def.health;
+    entity.sprite = def.image;
+    return entity;
+}
+
+} /* namespace engine */
diff --git a/src/entity.h b/src/entity.h
new file mode 100644
index 0000000..edcb991
--- /dev/null
+++ b/src/entity.h
@@ -0,0 +1,35 @@
+// entity.h — a live entity: the facts the game acts on.
+//
+// Lesson 073: an entity is a row the game created from a definition. It
+// carries that definition's identity and attributes in named fields the
+// game reads directly — and writes directly. The hero's position changes
+// every frame and its health changes when it is hit; what the game writes
+// is the entity's own copy of the facts, never the table's row. The
+// definition is where an entity comes from, not what it lives in.
+#ifndef ENTITY_H
+#define ENTITY_H
+
+#include "sprite.h"
+#include "table.h"
+
+namespace engine {
+
+/* One entity: the facts the game acts on, one struct of named fields.
+   No key/value bag, no lookup by string — the game reads entity.speed
+   and writes entity.x like any other values. */
+struct Entity {
+    char name[TABLE_NAME_MAX]; /* the definition's identity, carried */
+    int x, y;                  /* where it is, in world pixels */
+    int facing;                /* 0 right, 1 down, 2 left, 3 up */
+    int speed;                 /* world pixels per second */
+    int health;                /* points */
+    const Sprite *sprite;      /* the art it draws, from its row */
+};
+
+/* An entity created from a definition: every attribute its row states,
+   answered from the definition alone. */
+Entity EntityFromDef(const EntityDef &def);
+
+} /* namespace engine */
+
+#endif
diff --git a/src/main.cpp b/src/main.cpp
index 3f07b8b..abb8e6e 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -15,6 +15,7 @@
 #include "audio.h"
 #include "blit.h"
 #include "camera.h"
+#include "entity.h"
 #include "font.h"
 #include "framebuffer.h"
 #include "frame.h"
@@ -201,6 +202,54 @@ int Run(void)
                     def.sprite);
     }
 
+    /* Lesson 073: the definitions' art, loaded at startup. The table's
+       sprite column names the file; the run loads each one and hands the
+       definition its image, so an entity created from a definition is
+       answered from the definition alone. */
+    Sprite *images = (Sprite *)ArenaAlloc(
+        arena, (size_t)table.count * sizeof(Sprite), 4);
+    if (!images) {
+        std::fprintf(stderr, "engine: no room for the definitions' art\n");
+        platform::CloseWindow(opened.window);
+        ArenaRelease(arena);
+        return 1;
+    }
+    for (int i = 0; i < table.count; ++i) {
+        EntityDef &def = table.rows[i];
+        SpriteResult art = LoadSprite(arena, def.sprite);
+        if (art.error != SPRITE_OK) {
+            std::fprintf(stderr, "engine: %s: could not load\n", def.sprite);
+            platform::CloseWindow(opened.window);
+            ArenaRelease(arena);
+            return 1;
+        }
+        images[i] = art.sprite;
+        def.image = &images[i];
+    }
+
+    /* Lesson 073: the game's first entity — created from the hero's
+       definition, carrying the values its row states in named fields the
+       game reads directly. */
+    DefResult hero_def = TableFind(table, "hero");
+    if (hero_def.error != DEF_OK) {
+        std::fprintf(stderr,
+                     "engine: assets/entities.txt: no definition named \"hero\"\n");
+        platform::CloseWindow(opened.window);
+        ArenaRelease(arena);
+        return 1;
+    }
+    Entity hero = EntityFromDef(*hero_def.def);
+    std::printf("engine: entity %s: x %d y %d facing %d speed %d health %d sprite %dx%d\n",
+                hero.name, hero.x, hero.y, hero.facing, hero.speed, hero.health,
+                hero.sprite->width, hero.sprite->height);
+
+    /* The lookup's typed failure, checked on purpose: a definition the
+       table does not hold is a value — never an entity with assumed
+       attributes. */
+    DefResult unknown = TableFind(table, "dragon");
+    std::printf("engine: table: \"dragon\" -> %s\n",
+                unknown.error == DEF_OK ? "found" : "unknown");
+
     /* Lesson 066: the run's two sounds as files' bytes — the music that
        loops and the effect that plays once. Lesson 061's tone leaves the
        run here (it stays on disk: the file lessons 059-065 were built
diff --git a/src/table.cpp b/src/table.cpp
index c589b8d..ec1b59e 100644
--- a/src/table.cpp
+++ b/src/table.cpp
@@ -228,6 +228,7 @@ TableResult LoadTable(Arena &arena, const char *path)
         }
 
         EntityDef &def = defs[result.table.count];
+        def.image = 0; /* the run hands the definition its art, not the file */
         at = 0;
         for (int i = 0; ok && i < COL_COUNT; ++i) {
             switch (order[i]) {
@@ -289,4 +290,16 @@ TableResult LoadTable(Arena &arena, const char *path)
     return result;
 }
 
+DefResult TableFind(const EntityTable &table, const char *name)
+{
+    DefResult result = { 0, DEF_OK };
+    for (int i = 0; i < table.count; ++i)
+        if (SameText(table.rows[i].name, name)) {
+            result.def = &table.rows[i];
+            return result;
+        }
+    result.error = DEF_UNKNOWN;
+    return result;
+}
+
 } /* namespace engine */
diff --git a/src/table.h b/src/table.h
index c570c17..fae166d 100644
--- a/src/table.h
+++ b/src/table.h
@@ -19,6 +19,7 @@
 #define TABLE_H
 
 #include "arena.h"
+#include "sprite.h"
 
 namespace engine {
 
@@ -31,7 +32,11 @@ constexpr int TABLE_NAME_MAX = 16;
 constexpr int TABLE_PATH_MAX = 64;
 
 /* One definition: a row of the table, carrying every value its row
-   states — the identity and the attributes an entity is created from. */
+   states — the identity and the attributes an entity is created from.
+   The image is the one fact the file states as a name: the run loads the
+   art the sprite column names and hands the definition its image, so an
+   entity created from the definition is answered from the definition
+   alone. */
 struct EntityDef {
     char name[TABLE_NAME_MAX];   /* the definition's identity */
     int x, y;                    /* where it starts, in world pixels */
@@ -39,6 +44,7 @@ struct EntityDef {
     int speed;                   /* world pixels per second */
     int health;                  /* points */
     char sprite[TABLE_PATH_MAX]; /* the art file it draws */
+    const Sprite *image;         /* that art, loaded at startup */
 };
 
 /* A loaded table: one definition per row, in the arena — as many rows as
@@ -72,6 +78,24 @@ struct TableResult {
    to it: a load that refuses leaves nothing behind. */
 TableResult LoadTable(Arena &arena, const char *path);
 
+/* Lesson 073: a definition lookup — the request the game makes when it
+   wants an entity of a kind. The table either answers with the
+   definition or names what is missing: a definition the table does not
+   hold is a typed failure, never a row with assumed attributes. */
+enum DefError {
+    DEF_OK = 0,
+    DEF_UNKNOWN, /* the table holds no definition by that name */
+};
+
+struct DefResult {
+    const EntityDef *def; /* the definition, or 0 */
+    DefError error;       /* DEF_OK exactly when def is non-0 */
+};
+
+/* The definition named `name`, or the failure that says the table does
+   not hold one. */
+DefResult TableFind(const EntityTable &table, const char *name);
+
 } /* namespace engine */
 
 #endif
```

## Exercices

Deux exercices mixtes. Chacun se termine par sa solution — un diff contre l'état
final de cette leçon, plus une visite guidée — après l'énoncé.

### Exercice 1 — Deux héros, une ligne *(predict-the-output)*

Créez une seconde entité à partir de la même définition `hero`, à côté de la
première. Puis, avant de rapporter quoi que ce soit, changez la *première*
entité de l'exécution : mettez sa vie à 0 et son `x` à 0. Avant de lancer quoi
que ce soit, écrivez ce que le rapport dira pour le `x` et la vie de la seconde
entité, et ce que valent le `x` et la vie de la *définition* après coup — et une
phrase sur pourquoi. Lancez et réconciliez.

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-073/ex1.md)

### Exercice 2 — Chaque définition, une entité *(extend-the-code)*

Une entité par définition est la forme naturelle du jeu : la table liste les
types, l'exécution en fait vivre un de chaque. Faites que l'exécution crée une
entité à partir de *chaque* définition de la table — en rapportant les champs de
chacune comme ceux du héros le sont — au lieu de demander `hero` par son nom.
Puis ajoutez une troisième ligne à `assets/entities.txt` (votre propre type : un
`bat`, une `turret`, ce que votre jeu veut) et relancez **sans recompiler**.
Qu'apparaît-il dans le rapport, et qu'est-ce qui, dans le code de l'exécution, a
changé pour le faire apparaître ? Rapportez les deux exécutions.

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-073/ex2.md)

---

**Partie :** [Partie 4 — les services](../../index.md) ·
**Précédente :** [Leçon 072 — le chargement, complet ou nommé](lesson-072-load.md) ·
**Suivante :** [Leçon 074 — un magasin fixe](lesson-074-store.md) ·
**Étiquette de code :** [`lesson-073`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-073)

*Page traduite de la version anglaise `book/lessons/part-4/lesson-073-rows.md`,
révision `df91188`.*

<!-- translation-source: book/lessons/part-4/lesson-073-rows.md @ df91188 -->
