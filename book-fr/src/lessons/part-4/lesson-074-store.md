# Leçon 074 — un magasin fixe

{{#include ../../stability-horizon.md}}

## Prose

La leçon 073 s'est terminée sur une entité et nulle part où la mettre : `hero`
était une locale de l'exécution, et la boucle de création de l'exécution
laissait tomber par terre tout ce qu'elle fabriquait. Un jeu n'est pas une seule
entité — le héros, les ennemis, les projectiles, les rafales — et « où
vivent les vivantes ? » a une seule réponse dans ce moteur, la même que celle de
chaque tampon ici. L'idée de cette leçon est donc : **un magasin (store) fixe
d'entités vivantes — capacité décidée à l'avance, création prenant le premier
emplacement libre, et un magasin plein refusant la requête comme une valeur
typée.** Jamais en volant une vivante.

### La capacité est une décision, pas un événement

```cpp
constexpr int ENTITY_CAP = 64;

struct EntityStore {
    Entity slots[ENTITY_CAP];
    int live; /* how many slots hold a live entity right now */
};
```

Soixante-quatre emplacements, chacun contenant une entité ou rien — décidé au
moment de la compilation, pas découvert à l'exécution. C'est l'habitude de
l'arena (leçon 041) appliquée un étage plus haut : la loi du langage de la leçon
026 dit aucune allocation pendant que le jeu tourne, donc la mémoire du magasin
est *prise* — une struct, 64 emplacements, les octets connus à l'instant où le
programme est compilé. Créer une entité écrit dans un emplacement qui existe
déjà ; rien ne demande quoi que ce soit à l'OS.

Le nombre 64 est une décision, et c'est cette leçon qui la prend : le héros, les
types d'ennemis et un écran de projectiles, la revue de clôture la nommant. Ce
qui empêche une mauvaise capacité d'être un bug de *correction*, c'est la
politique ci-dessous — un magasin qui refuse bruyamment est un magasin qu'on
peut régler, et un magasin qui prend silencieusement au mauvais endroit est un
bug avec un jeu greffé dessus.

### Le premier emplacement libre

`EntityCreate` parcourt les emplacements depuis le début et prend le premier qui
ne contient rien :

```cpp
EntityResult EntityCreate(EntityStore &store, const EntityDef &def);
```

Cette règle est courte à dessein, et elle tranche plus de choses qu'elle n'en a
l'air : les emplacements étant fixes, « le premier emplacement libre » est aussi
« l'emplacement libéré par une entité retirée, avant tout emplacement jamais
utilisé » — parce que l'indice d'un emplacement libéré est plus bas que celui de
tout emplacement jamais utilisé. La leçon 075 rendra cela visible avec le
retrait ; la règle ne change pas quand il arrive.

La création est une copie de struct (`EntityFromDef`, leçon 073) dans un
emplacement, plus le compte. Elle ne touche à aucune mémoire hors du magasin —
la vérification ci-dessous le montre en chiffre.

### Un magasin plein refuse ; il ne vole jamais

Quand chaque emplacement est occupé, la requête reçoit `ENTITY_FULL` et `entity`
à 0. C'est le jeu qui décide ce que cela signifie — abandonner le projectile,
arrêter la vague, punir le joueur — et le magasin ne décide que ceci : la
réponse n'est jamais l'entité de quelqu'un d'autre.

C'est la politique que le mixeur n'a *pas*, et la différence est enseignée à
dessein, côte à côte. `MixerPlayEffect` vole le canal d'effet le **plus
ancien** quand le pool est occupé (leçon 065), et c'est la bonne politique
là-bas : un son abandonné est inaudible — le joueur entend les effets par-dessus
la musique et ne remarquerait pas qu'il en manque un. Une entité volée n'est pas
inaudible. C'est un ennemi qui disparaît en plein écran, un projectile qui
s'évanouit avant d'atterrir, une partie de boss qui cesse d'exister parce qu'une
particule a demandé la place. Une politique convient à une ressource dont la
perte est imperceptible ; l'autre à une ressource dont la perte est un bug que
le joueur subit. Les deux politiques sont celles du moteur, toutes deux sont
typées, et aucune n'est une surprise.

### Ce que cette exécution a vérifié, et ce qu'elle n'a pas vérifié

D'une vraie exécution de l'état final de cette leçon sur cette machine — le
script de création de l'exécution, une requête après l'autre, les lignes de la
table à tour de rôle, jusqu'à ce que le magasin dise non :

- **Le cas occupé refuse comme une valeur typée.** Le script a demandé 64 fois
  et a obtenu 64 entités — le héros et 63 autres, `live 64 of 64` — et la
  soixante-cinquième requête a été refusée : `engine: store: creation refused
  (full), arena 1253648 -> 1253648 — creation allocates nothing`. Le refus est
  le champ d'erreur du résultat, `ENTITY_FULL` ; l'exécution le rapporte et
  continue. Une sonde jetable (pas du code du moteur) a fait cette
  soixante-cinquième requête puis a regardé : `past the full one: error 1,
  entity (nil), live 64` et `slots changed by the refused request: 0 of 64` —
  pas une seule entité vivante touchée. Rien n'a été écrasé ni volé : chacune
  des 64 contient encore ce que sa définition énonçait.
- **Créer des entités n'alloue rien.** Le compte `used` de l'arena sur tout le
  script — 64 créations — est le même nombre des deux côtés : 1253648. Les
  octets du magasin ont été pris quand la trame de pile de l'exécution a été
  faite, et aucune création n'a bougé le bump pointer.

Ce que cette leçon ne vérifie **pas**, c'est ce que le jeu fait *du* refus —
c'est la décision du jeu, et c'est l'exercice ci-dessous où vous la prenez. Et
rien ne parcourt encore le magasin : 64 entités vivantes siègent dans leurs
emplacements et aucun code ne les visite. C'est la leçon 075 — l'itération et la
durée de vie — et c'est ce qui transforme un magasin en quelque chose sur quoi
une frame peut agir.

## Étape de code

Un seul changement pour cette leçon, d'une locale à un magasin : `src/entity.h` /
`src/entity.cpp` accueillent `ENTITY_CAP`, `EntityStore`, l'`EntityResult` typé
et `EntityCreate` — le premier emplacement libre, le compte, et le refus qui ne
vole jamais. `Entity` accueille son drapeau `live`, l'état propre de
l'emplacement. `src/main.cpp` accueille le démarrage de l'exécution : l'entité
du héros est créée dans le magasin comme tout le reste, et le script de création
remplit le magasin depuis les lignes de la table jusqu'à ce que le refus typé
arrive — le rapport nommant le compte des vivantes et celui de l'arena,
inchangé. Le sprite de la démo, la carte et la boucle sont intacts. Son état
final est étiqueté `lesson-074`.

```diff
diff --git a/src/entity.cpp b/src/entity.cpp
index 3bef96c..36bb098 100644
--- a/src/entity.cpp
+++ b/src/entity.cpp
@@ -23,4 +23,29 @@ Entity EntityFromDef(const EntityDef &def)
     return entity;
 }
 
+EntityResult EntityCreate(EntityStore &store, const EntityDef &def)
+{
+    EntityResult result = { 0, ENTITY_OK };
+
+    /* The first free slot — the lowest one that holds nothing. With the
+       slots fixed in place that rule also answers which slot a new
+       entity takes after one is retired: the freed one, before any slot
+       that has never been used (lesson 075 makes that visible). */
+    for (int i = 0; i < ENTITY_CAP; ++i) {
+        if (store.slots[i].live)
+            continue;
+        store.slots[i] = EntityFromDef(def);
+        store.slots[i].live = true;
+        store.live += 1;
+        result.entity = &store.slots[i];
+        return result;
+    }
+
+    /* Every slot busy: the request is refused as a value. The game
+       decides what a refusal means; the store decides only this — that
+       it is never a stolen entity. */
+    result.error = ENTITY_FULL;
+    return result;
+}
+
 } /* namespace engine */
diff --git a/src/entity.h b/src/entity.h
index edcb991..d31886b 100644
--- a/src/entity.h
+++ b/src/entity.h
@@ -24,12 +24,47 @@ struct Entity {
     int speed;                 /* world pixels per second */
     int health;                /* points */
     const Sprite *sprite;      /* the art it draws, from its row */
+    bool live;                 /* lesson 074: this entity exists — the
+                                  slot's state, set by the store */
 };
 
 /* An entity created from a definition: every attribute its row states,
    answered from the definition alone. */
 Entity EntityFromDef(const EntityDef &def);
 
+/* Lesson 074: the store's capacity — a decision, made here and named in
+   the closing review. Sixty-four live entities: the hero, the enemy
+   types, and a screenful of projectiles. What the game does when it is
+   wrong is the store's policy below, not a surprise. */
+constexpr int ENTITY_CAP = 64;
+
+/* One fixed store of live entities: slots decided up front, each slot
+   holding one entity or nothing. Nothing is allocated while the game
+   runs — creation takes a slot and retirement gives it back. A store
+   with every slot zeroed is empty. */
+struct EntityStore {
+    Entity slots[ENTITY_CAP];
+    int live; /* how many slots hold a live entity right now */
+};
+
+/* The creation request, answered with the entity or with the typed
+   failure that says there is no room. */
+enum EntityError {
+    ENTITY_OK = 0,
+    ENTITY_FULL, /* every slot holds a live entity */
+};
+
+struct EntityResult {
+    Entity *entity;    /* the entity, or 0 */
+    EntityError error; /* ENTITY_OK exactly when entity is non-0 */
+};
+
+/* Creates an entity from `def` in the store's first free slot. When
+   every slot is busy the request is refused as a typed value — the
+   store never steals a live entity to make room. A stolen sound is
+   inaudible; a stolen enemy is a bug the player experiences. */
+EntityResult EntityCreate(EntityStore &store, const EntityDef &def);
+
 } /* namespace engine */
 
 #endif
diff --git a/src/main.cpp b/src/main.cpp
index abb8e6e..d84675f 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -229,7 +229,8 @@ int Run(void)
 
     /* Lesson 073: the game's first entity — created from the hero's
        definition, carrying the values its row states in named fields the
-       game reads directly. */
+       game reads directly. Lesson 074: it lives in the store now, in a
+       slot of the capacity decided up front. */
     DefResult hero_def = TableFind(table, "hero");
     if (hero_def.error != DEF_OK) {
         std::fprintf(stderr,
@@ -238,11 +239,36 @@ int Run(void)
         ArenaRelease(arena);
         return 1;
     }
-    Entity hero = EntityFromDef(*hero_def.def);
+    EntityStore store = {};
+    EntityResult hero_made = EntityCreate(store, *hero_def.def);
+    if (hero_made.error != ENTITY_OK) {
+        std::fprintf(stderr, "engine: the store refused the hero\n");
+        platform::CloseWindow(opened.window);
+        ArenaRelease(arena);
+        return 1;
+    }
+    Entity &hero = *hero_made.entity;
     std::printf("engine: entity %s: x %d y %d facing %d speed %d health %d sprite %dx%d\n",
                 hero.name, hero.x, hero.y, hero.facing, hero.speed, hero.health,
                 hero.sprite->width, hero.sprite->height);
 
+    /* Lesson 074: the creation script — request after request, each an
+       entity from the table's rows in turn, until the store answers with
+       its typed failure. This is the policy under load: the first free
+       slot, and never a live entity's. */
+    size_t before_script = arena.used;
+    int created = 0;
+    for (;;) {
+        EntityResult made = EntityCreate(store, table.rows[created % table.count]);
+        if (made.error != ENTITY_OK)
+            break;
+        created += 1;
+    }
+    std::printf("engine: store: live %d of %d — the hero and %d from the script\n",
+                store.live, ENTITY_CAP, created);
+    std::printf("engine: store: creation refused (full), arena %zu -> %zu — creation allocates nothing\n",
+                before_script, arena.used);
+
     /* The lookup's typed failure, checked on purpose: a definition the
        table does not hold is a value — never an entity with assumed
        attributes. */
```

## Exercices

Deux exercices mixtes. Chacun se termine par sa solution — un diff contre l'état
final de cette leçon, plus une visite guidée — après l'énoncé.

### Exercice 1 — Le refus qu'on ignore *(fix-the-crash)*

`EntityResult` porte l'entité *et* l'erreur, et le contrat veut qu'`entity` vaille
0 exactement quand l'erreur n'est pas `ENTITY_OK`. Faites en sorte que
l'exécution l'oublie : une fois que le script de création a rempli le magasin,
gardez le pointeur `entity` de la dernière requête et utilisez-le — imprimez son
`name` — sans regarder l'erreur. Reconstruisez avec l'instrument de la leçon 013
— AddressSanitizer, via les remplacements `CXXFLAGS` et `LDFLAGS` de
`./build.sh` — et lancez. Lisez ce que disent l'exécution et le sanitizer. Puis
corrigez l'exécution pour qu'une requête refusée soit *traitée* : la décision du
jeu, rapportée et emportée plus loin, et aucun chemin qui utilise l'entité d'une
requête avant que son erreur soit vérifiée. Prouvez que l'exécution survit au
magasin plein et se termine toujours proprement (`engine: closed`).

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-074/ex1.md)

### Exercice 2 — La capacité que vous choisiriez *(explain-in-prose)*

Soixante-quatre est la décision de cette leçon ; la vôtre peut différer. Nommez
la capacité dont *votre* jeu a besoin et montrez le compte : le héros, les
ennemis vivants à la fois, les projectiles en vol, les particules d'une rafale.
Répondez ensuite dans vos propres mots, le code sous les yeux : que fait votre
jeu quand la capacité est trop petite (quelles requêtes sont refusées, et que
devrait-il faire à ce sujet ?), et combien coûte-t-elle quand elle est trop
grande (les octets du magasin sont pris de toute façon — faites rapporter à
l'exécution `sizeof(Entity)` et le total du magasin, pour que le coût soit un
chiffre et non un haussement d'épaules) ? Et pourquoi une mauvaise capacité
est-elle ici un problème de *réglage* plutôt qu'un bug de correction — qu'est-ce
qui, dans la conception du magasin, rend cela vrai ?

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-074/ex2.md)

---

**Partie :** [Partie 4 — les services](../../index.md) ·
**Précédente :** [Leçon 073 — les entités en lignes](lesson-073-rows.md) ·
**Suivante :** [Leçon 075 — la marche et l'emplacement libre](lesson-075-lifetime.md) ·
**Étiquette de code :** [`lesson-074`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-074)

*Page traduite de la version anglaise `book/lessons/part-4/lesson-074-store.md`,
révision `cc57259`.*

<!-- translation-source: book/lessons/part-4/lesson-074-store.md @ cc57259 -->
