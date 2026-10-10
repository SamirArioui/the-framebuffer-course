# Leçon 090 — le boss

{{#include ../../stability-horizon.md}}

## Prose

Le golem se tient au `x` et au `y` de sa ligne depuis deux leçons pendant que
tout le monde bouge. Son `behavior` dit `boss` — la seule valeur sur laquelle
le switch de la marche n'agit pas encore. Cette leçon dit ce que cette valeur
veut dire : **les trois comportements, composés par un schéma propre au boss —
son propre planning, et rien d'autre à lui.** Et pendant que le boss apprend à
bouger, le monde apprend à tirer : les lignes d'ennemis portent leurs armes
depuis la leçon 088, et aujourd'hui la marche les fait tirer.

### Un planning n'est pas de la machinerie de mouvement

L'instinct contre lequel la leçon sur l'effectif argumentait — *le boss mérite
son propre code* — avait à moitié raison. Ce que le boss mérite, c'est du
**temps** : des phases, une cadence, « poursuis maintenant, tiens et tire,
décroche, recommence ». Une ligne porte des faits ; elle ne peut pas porter un
planning. Le boss en reçoit donc un — de l'état et du timing par entité, le
`phase` et le `phase_t` de l'entité — et ce que le planning programme, ce sont
les trois mêmes fonctions que tout le monde utilise :

```cpp
e.phase_t += dt;                       /* how long this phase has run */
if (e.phase_t >= length) {
    e.phase_t = 0.0;
    e.phase = (e.phase + 1) % 3;       /* chase -> keep -> flee -> … */
    ...
}
if (e.phase == 0)      AiChase(e, hero);
else if (e.phase == 1) AiKeep(e, hero);
else                   AiFlee(e, hero);
```

Voilà tout le boss : un minuteur, un compteur et trois appels. Aucun calcul de
mouvement, aucun mover propre au boss, aucune physique par phase — les phases
*sont* `AiChase`, `AiKeep` et `AiFlee`, dans un ordre et pour des durées que le
schéma possède (3 s, 2 s, 1 s). Le minuteur compte en temps de jeu, donc la
pause de la leçon 078 fige le schéma aussi.

### Le schéma, mesuré

Une exécution avec le boss seul sur la carte (un effectif jetable ne contenant
que sa ligne — le boss est plus facile à observer sans compagnie) et le héros
immobile. Le schéma annonce ses phases, et les rapports d'entités montrent le
mouvement de chaque phase faisant exactement le travail de son comportement :

```
engine: golem at 384,172 — 93 px of the hero (t=1.504)
engine: golem at 365,190 — 67 px of the hero (t=1.866)
engine: golem at 352,212 — 44 px of the hero (t=2.297)
engine: golem at 337,232 — 25 px of the hero (t=2.857)
engine: boss: golem's pattern -> keep (2 s)
engine: golem at 358,217 — 48 px of the hero (t=3.849)
engine: golem at 376,200 — 71 px of the hero (t=4.196)
engine: golem at 393,182 — 95 px of the hero (t=4.541)
engine: golem at 411,165 — 119 px of the hero (t=4.885)
engine: golem at 428,147 — 144 px of the hero (t=5.231)
engine: boss: golem's pattern -> flee (1 s)
engine: golem at 446,129 — 168 px of the hero (t=5.576)
engine: golem at 464,112 — 193 px of the hero (t=5.923)
engine: golem at 481,94 — 218 px of the hero (t=6.268)
engine: boss: golem's pattern -> chase (3 s)
engine: golem at 464,112 — 193 px of the hero (t=6.701)
engine: golem at 446,129 — 168 px of the hero (t=7.046)
...
engine: golem at 338,233 — 26 px of the hero (t=9.161)
engine: boss: golem's pattern -> keep (2 s)
```

Lisez les distances : `93 → 67 → 44 → 25`, c'est la **poursuite** qui se
referme. `48 → 71 → 95 → 119 → 144`, c'est la **garde de distance** qui recule
jusqu'à son 160 — le même `AiKeep` que le spitter utilisait, la même bande, la
même math. `168 → 193 → 218`, c'est la **fuite** qui court. Puis `chase (3 s)`
et la distance retombe. Le boss, c'est trois comportements et une montre.

### Le monde riposte

Les lignes d'ennemis portent `damage`, `rate` et `fires` depuis la leçon 088.
Aujourd'hui, le travail par entité de la marche gagne une ligne de plus —
l'**attaque** : une entité armée tire l'arme de sa ligne sur le héros, à la
cadence de sa ligne, tant que le héros est à portée du tir. Le héros est
exempté (son déclencheur est celui du joueur) ; un type désarmé — comme le
slime, dont la ligne ne nomme aucune arme — ne tire rien.

Avec cela, le dernier substitut de la leçon 087 meurt. La touche `G` et le
slime armé ont disparu : ce qu'ils démontraient — un vrai projectile qui
touche, réduisant
les points de vie du héros des dégâts de la ligne — est maintenant le travail
propre des lignes d'ennemis. Une exécution avec le héros planté dans la ligne
du boss :

```
engine: hit: shell hits hero — damage 2, health 3 -> 1
engine: hit: shell hits hero — damage 2, health 1 -> 0
engine: state play -> death (the hero's health reached zero)
```

`damage 2` deux fois — la ligne du golem — et la condition nommée se
déclenche en combat réel, sans touche maintenue. (La couture rétrécit avec le
substitut : `KEY_G` a disparu de `platform.h`.)

### Ce que cette exécution a vérifié, et ce qu'elle n'a pas vérifié

- **Le boss compose les comportements plus son schéma** — les rapports de
  phases et les distances montrent `AiChase`, `AiKeep` et `AiFlee` faisant le
  déplacement du boss ; la machinerie propre du schéma est un minuteur et un
  compteur.
- **Aucune machinerie de mouvement séparée** — `AiBoss` ne contient aucun
  calcul de mouvement ; chaque pas du boss est la requête d'un comportement
  partagé à travers le même `MoveEntity` que tout le monde.
- **Les attaques des ennemis sont réelles** — les `shell` du boss touchent le
  héros pour les dégâts de sa ligne et les points de vie à zéro du héros sont
  la défaite du jeu.

Ce que cette exécution n'a **pas** vérifié, c'est le combat de boss comme un
*combat* — avec le héros qui tire en retour, esquive et gagne. Les huit points
de vie du golem sont un fait de ligne en attente d'un joueur ; savoir si les
nombres font un bon combat est le jugement du jeu, pas celui de la machine. Le
planning non plus ne connaît rien d'autre que ses trois phases : une seconde
moitié enragée, une préparation télégraphiée, une phase qui dépense trois
tirs — tous des schémas de la même forme (un planning qui compose du travail
partagé), et des exercices ci-dessous.

## Étape de code

Un changement : le schéma et les attaques. `src/ai.h/.cpp` grandissent
d'`AiBoss` — le planning (le `phase` et le `phase_t` de l'entité) composant
les trois comportements partagés ; `src/entity.h` porte cet état de schéma ;
`src/combat.h/.cpp` grandissent de `CombatAttack` (une entité armée tire
l'arme de sa ligne à sa cadence, sur le héros, à portée du tir) ; la marche de
`src/game.cpp` gagne le cas `boss` et la ligne d'attaque ; `src/main.cpp` et
`src/platform.h`/`src/platform_x11.cpp` abandonnent le substitut de tir ennemi
de la leçon 087 et sa touche — les lignes d'ennemis tirent pour de vrai.
`assets/enemies.txt` ajuste la cadence du golem au combat que cette leçon
démontre (un shell toutes les trois secondes). Son état final est étiqueté
`lesson-090`.

```diff
diff --git a/assets/enemies.txt b/assets/enemies.txt
index ffd931d..0659107 100644
--- a/assets/enemies.txt
+++ b/assets/enemies.txt
@@ -2,4 +2,4 @@ name x y facing speed health sprite damage rate fires behavior wave count
 bat 560 72 2 160 2 assets/bat.ppm 1 60 bolt chase 1 2
 wisp 640 336 2 120 1 assets/wisp.ppm 1 20 bolt flee 1 1
 spitter 120 400 0 96 3 assets/spitter.ppm 1 30 shell keep 2 2
-golem 384 96 1 72 8 assets/golem.ppm 2 30 shell boss 3 1
+golem 384 96 1 72 8 assets/golem.ppm 2 20 shell boss 3 1
diff --git a/src/ai.cpp b/src/ai.cpp
index 1ed2dc4..67b9ceb 100644
--- a/src/ai.cpp
+++ b/src/ai.cpp
@@ -8,6 +8,8 @@
 
 #include "ai.h"
 
+#include <cstdio>
+
 #include "combat.h" /* CombatAim — the same eight compass points */
 
 namespace engine {
@@ -45,4 +47,35 @@ void AiFlee(Entity &e, const Entity &hero)
     CombatAim(e.x - hero.x, e.y - hero.y, e.move_x, e.move_y);
 }
 
+void AiBoss(Entity &e, const Entity &hero, double dt)
+{
+    /* The schedule: one timer per entity (its `phase_t`) counting how
+       long the current phase has run, in game time — so a frozen world
+       freezes the pattern too — and the phase index cycling when a
+       phase's length is met. This is the whole of the boss's own
+       machinery: state and timing. */
+    e.phase_t += dt;
+    double length = e.phase == 0 ? BOSS_CHASE_S
+                                 : e.phase == 1 ? BOSS_KEEP_S : BOSS_FLEE_S;
+    if (e.phase_t >= length) {
+        e.phase_t = 0.0;
+        e.phase = (e.phase + 1) % 3;
+        length = e.phase == 0 ? BOSS_CHASE_S
+                              : e.phase == 1 ? BOSS_KEEP_S : BOSS_FLEE_S;
+        std::printf("engine: boss: %s's pattern -> %s (%.0f s)\n", e.name,
+                    e.phase == 0 ? "chase" : e.phase == 1 ? "keep" : "flee",
+                    length);
+    }
+
+    /* And what the schedule schedules: the same three behaviors every
+       other entity uses. The boss composes them; it does not own any
+       movement of its own. */
+    if (e.phase == 0)
+        AiChase(e, hero);
+    else if (e.phase == 1)
+        AiKeep(e, hero);
+    else
+        AiFlee(e, hero);
+}
+
 } /* namespace engine */
diff --git a/src/ai.h b/src/ai.h
index a2b1ee7..f5fd324 100644
--- a/src/ai.h
+++ b/src/ai.h
@@ -37,6 +37,21 @@ void AiKeep(Entity &e, const Entity &hero);
 /* Flee: the request points away from the hero, every frame. */
 void AiFlee(Entity &e, const Entity &hero);
 
+/* Lesson 090: the boss's pattern — its own schedule, and the one thing
+   a row cannot carry. The schedule is per-entity state and timing (the
+   entity's `phase` and `phase_t`), and what it schedules is the same
+   three behaviors above: the boss has no movement machinery of its
+   own. The phases' lengths are the pattern's own facts, here beside
+   the keep distance. */
+constexpr double BOSS_CHASE_S = 3.0;
+constexpr double BOSS_KEEP_S = 2.0;
+constexpr double BOSS_FLEE_S = 1.0;
+
+/* One boss's pattern, once per frame of game time: the schedule
+   advances, and the phase it lands on writes the request — through
+   AiChase, AiKeep, or AiFlee, like every other entity. */
+void AiBoss(Entity &e, const Entity &hero, double dt);
+
 } /* namespace engine */
 
 #endif
diff --git a/src/combat.cpp b/src/combat.cpp
index 905cfc7..0673f23 100644
--- a/src/combat.cpp
+++ b/src/combat.cpp
@@ -180,4 +180,36 @@ void CombatFly(const TileMap &map, EntityStore &store, const Entity &hero,
     }
 }
 
+void CombatAttack(EntityStore &store, const EntityTable &shots, Entity &e,
+                  const Entity &hero, double dt)
+{
+    /* An unarmed entity never fires — and the hero is never its own
+       attacker: its trigger is the player's (HeroFire). */
+    if (!e.fires[0] || e.rate <= 0 || &e == &hero)
+        return;
+    if (e.cooldown > 0.0) {
+        e.cooldown -= dt;
+        return;
+    }
+
+    /* The shot's reach is its kind's row: an attacker threatens only
+       as far as its shot flies, and stays quiet beyond it. */
+    DefResult kind = TableFind(shots, e.fires);
+    if (kind.error != DEF_OK)
+        return;
+    double dx = hero.x - e.x, dy = hero.y - e.y;
+    double reach = (double)kind.def->range;
+    if (dx * dx + dy * dy > reach * reach)
+        return;
+
+    /* The attack aims the way everything else does: the compass point
+       at the hero. */
+    double dir_x = 0.0, dir_y = 0.0;
+    CombatAim(dx, dy, dir_x, dir_y);
+    if (dir_x == 0.0 && dir_y == 0.0)
+        return;
+    if (CombatFire(store, shots, e, dir_x, dir_y))
+        e.cooldown = 60.0 / (double)e.rate;
+}
+
 } /* namespace engine */
diff --git a/src/combat.h b/src/combat.h
index b052eaf..77df2ba 100644
--- a/src/combat.h
+++ b/src/combat.h
@@ -60,6 +60,14 @@ bool CombatFire(EntityStore &store, const EntityTable &shots,
 void CombatFly(const TileMap &map, EntityStore &store, const Entity &hero,
                Entity &shot, double dt);
 
+/* Lesson 090: the enemy attack, once per frame of game time. An armed
+   entity — one whose row names a projectile kind — fires it at the
+   hero at its row's rate, while the hero is within the shot's reach.
+   The hero itself is never its own attacker: its trigger is the
+   player's. */
+void CombatAttack(EntityStore &store, const EntityTable &shots, Entity &e,
+                  const Entity &hero, double dt);
+
 } /* namespace engine */
 
 #endif
diff --git a/src/entity.h b/src/entity.h
index a6872fb..2797cc7 100644
--- a/src/entity.h
+++ b/src/entity.h
@@ -57,6 +57,10 @@ struct Entity {
     int wave;                  /* lesson 088: which wave spawns this
                                   kind — carried like every row value */
     int count;                 /* and how many join that wave */
+    int phase;                 /* lesson 090: where a pattern is in its
+                                  schedule — per-entity state, the one
+                                  thing a row cannot carry */
+    double phase_t;            /* and how long this phase has run */
     double cooldown;           /* seconds until it may fire again */
 };
 
diff --git a/src/game.cpp b/src/game.cpp
index 11276a1..f9350ae 100644
--- a/src/game.cpp
+++ b/src/game.cpp
@@ -22,15 +22,12 @@ namespace engine {
 
 /* The demonstration stand-ins, named so they cannot be mistaken for the
    game. Lesson 087 made the hit real: a projectile reduces its target's
-   health by its row's damage, and a zero-health entity is retired. What
-   still stands in is the *enemy's fire* — who shoots at the hero, and
-   when. Until the enemies' attacks land (lesson 090), the run's G key
-   makes the slime spit at the hero (a keyed stand-in in the run, like
-   the feel demonstration beside it), and ENTER in play below says the
-   game is complete — lesson 091's waves spend it for real. Both die
-   when the real triggers arrive; the transitions they fire are the
-   game's own (defeat on zero health, completion on no waves). Keyed,
-   they never fire on their own during a gameplay test. */
+   health by its row's damage, and a zero-health entity is retired.
+   Lesson 090 made the enemy fire real: the enemy rows carry their own
+   weapons and the walk's attack fires them at the hero — the `G` key's
+   stand-in is gone. What remains is ENTER in play below: the game's
+   completion stood in for, until lesson 091's waves spend it for real.
+   Keyed, it never fires on its own during a gameplay test. */
 
 /* A transition, named once here and printed the moment it happens, so a
    run shows the machine moving between states and why. */
@@ -228,7 +225,7 @@ void GameDrawSprites(const Game &game, Framebuffer &fb,
 }
 
 int GameWalk(EntityStore &store, const TileMap &map, const Entity &hero,
-             double dt)
+             const EntityTable &shots, double dt)
 {
     /* Lesson 084: the walk — every live entity, once per frame, in slot
        order, its movement resolved against the tilemap. The per-entity
@@ -267,12 +264,20 @@ int GameWalk(EntityStore &store, const TileMap &map, const Entity &hero,
         case BEHAVIOR_FLEE:
             AiFlee(e, hero);
             break;
+        case BEHAVIOR_BOSS:
+            AiBoss(e, hero, dt);
+            break;
         default:
             /* `none` stands where it stands — the request is its row's
-               (lesson 084's stand-in walk writes one) — and `boss` is
-               lesson 090's pattern, composed of these same behaviors. */
+               (lesson 084's stand-in walk used to write one). */
             break;
         }
+
+        /* Lesson 090: the attack, once per entity — an armed kind fires
+           its row's weapon at the hero at its rate. The hero is exempt
+           (its trigger is the player's); an unarmed kind fires
+           nothing. */
+        CombatAttack(store, shots, e, hero, dt);
         MoveEntity(map, e, e.move_x * e.speed * dt, e.move_y * e.speed * dt);
         if (e.move_x > 0.0)
             e.facing = 0;
diff --git a/src/game.h b/src/game.h
index bde62f0..386c098 100644
--- a/src/game.h
+++ b/src/game.h
@@ -105,11 +105,13 @@ void GameDrawSprites(const Game &game, Framebuffer &fb,
    through the mover (MoveEntity), one axis at a time, so it stops at a
    solid tile and slides along a wall. Lesson 087: a projectile's
    behavior flies it (CombatFly — its own sub-stepped flight, retiring at
-   walls, at its range, at what it hits) instead of the request. The hero
-   is handed along for the combat's rules to know the game's actor by.
-   Returns the visit count. */
+   walls, at its range, at what it hits) instead of the request. Lesson
+   089-090: the enemy behaviors and the boss's pattern write the request
+   the way the player's input writes the hero's, and an armed entity
+   attacks at its row's rate. The hero is handed along for the combat's
+   rules to know the game's actor by. Returns the visit count. */
 int GameWalk(EntityStore &store, const TileMap &map, const Entity &hero,
-             double dt);
+             const EntityTable &shots, double dt);
 
 } /* namespace engine */
 
diff --git a/src/main.cpp b/src/main.cpp
index 4c5a16f..3271085 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -336,7 +336,6 @@ int Run(void)
        same table, one entity per row. A new row is a new entity; the
        run has no per-kind code to grow. */
     int created = 1;
-    Entity *foe = 0; /* the world's one enemy row (the slime) */
     for (int i = 0; i < table.count; ++i) {
         if (&table.rows[i] == hero_def.def)
             continue;
@@ -352,8 +351,6 @@ int Run(void)
            stand-in for the AI. Lesson 089 replaced it: the behaviors
            are real now, and the world's kinds move the ways their rows
            say (the slime's row says `none`, so it stands). */
-        if (!foe)
-            foe = made.entity;
         created += 1;
     }
     std::printf("engine: world: %d entities from the table's rows, live %d of %d\n",
@@ -379,15 +376,12 @@ int Run(void)
                 foes.count, store.live, ENTITY_CAP);
 
     /* Lesson 087: weapons are rows. The hero starts armed with the
-       weapons table's first row; the number keys arm the rest (HeroFire).
-       The demonstration stand-in arms the foe with the second row — its
-       projectile is what G spits at the hero. The enemy rows that carry
-       their own attacks arrive in lesson 088; this stand-in and its key
-       die when those attacks land (lesson 090). */
+       weapons table's first row; the number keys arm the rest
+       (HeroFire). Lesson 090: the enemy-fire stand-in and its key are
+       gone — the enemy rows carry their own weapons and the walk's
+       attack fires them. */
     if (weapons.count > 0)
         CombatArm(hero, weapons.rows[0]);
-    if (weapons.count > 1 && foe)
-        CombatArm(*foe, weapons.rows[1]);
 
     /* The lookup's typed failure, checked on purpose: a definition the
        table does not hold is a value — never an entity with assumed
@@ -454,7 +448,7 @@ int Run(void)
     std::printf("engine: sound %d-frame music looping on channel %d, %d-frame effect on the pool; one mixer of %d channels\n",
                 music.frame_count, AUDIO_MUSIC_CHANNEL, effect.frame_count,
                 AUDIO_MIXER_CHANNELS);
-    std::printf("engine: arrows move the hero, 1 and 2 arm the weapons, space fires, G is the enemy spit; close the window to stop\n");
+    std::printf("engine: arrows move the hero, 1 and 2 arm the weapons, space fires; close the window to stop\n");
     std::printf("engine: hero at %.0f,%.0f\n", hero.x, hero.y);
 
     /* Lesson 059: the run's sound is a run of amplitude at the engine's
@@ -565,17 +559,6 @@ int Run(void)
             HeroMove(hero, opened.window, dt);
             HeroFire(hero, opened.window, weapons, shots, store, dt);
 
-            /* Lesson 087: the enemy-fire stand-in — G makes the slime
-               spit at the hero. What it demonstrates is real: the shot
-               is an entity, its hit reduces the hero's health by the
-               row's damage, and the hero's zero health is the game's
-               defeat. Only the shooter and its aim are scripted — the
-               enemies' own attacks (lesson 090) replace this key. */
-            if (foe && platform::KeyPressed(opened.window, platform::KEY_G)) {
-                double dir_x = 0.0, dir_y = 0.0;
-                CombatAim(hero.x - foe->x, hero.y - foe->y, dir_x, dir_y);
-                CombatFire(store, shots, *foe, dir_x, dir_y);
-            }
         }
 
         /* Lesson 084: the game resolves its movement against its map —
@@ -584,7 +567,7 @@ int Run(void)
            every projectile into its flight). The loop times it as the
            frame record's entity sub-phase. */
         double t_entities = platform::Now();
-        int visited = GameWalk(store, map, hero, dt);
+        int visited = GameWalk(store, map, hero, shots, dt);
         frame.entities = platform::Now() - t_entities;
         walk_visits += visited;
 
diff --git a/src/platform.h b/src/platform.h
index cd860ae..f2408c0 100644
--- a/src/platform.h
+++ b/src/platform.h
@@ -45,7 +45,6 @@ enum Key {
     KEY_ESCAPE,
     KEY_1,
     KEY_2,
-    KEY_G,
     KEY_COUNT
 };
 
diff --git a/src/platform_x11.cpp b/src/platform_x11.cpp
index bd91201..2991183 100644
--- a/src/platform_x11.cpp
+++ b/src/platform_x11.cpp
@@ -166,7 +166,6 @@ static int KeyIndex(KeySym sym)
     case XK_Escape: return KEY_ESCAPE;
     case XK_1:      return KEY_1;
     case XK_2:      return KEY_2;
-    case XK_g:      return KEY_G;
     default:        return -1;
     }
 }
```

## Exercices

Deux défis plus grands. Chacun se termine par sa solution — un diff contre
l'état final de cette leçon plus une visite guidée — après l'énoncé.

### Exercice 1 — Le schéma possède ses attaques *(extend-the-code)*

Pour l'instant, le boss tire quand la cadence de sa ligne le dit, quelle que
soit la phase — la ligne d'attaque est celle de tout le monde. Donnez plutôt
au *schéma* du boss ses attaques : tant que le schéma est dans sa phase
`keep`, il tire l'arme de sa ligne sur le héros à la cadence de la ligne (son
temps de maintien et de tir) ; en `chase` et `flee`, il retient son tir. La
règle d'attaque générique reste pour tout autre type armé. Puis relancez le
boss seul : les lignes `fire` tombent-elles à l'intérieur des phases `keep` et
seulement là ?

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-090/ex1.md)

### Exercice 2 — La chronologie du planning *(predict-the-output)*

Le schéma fait 3 s de poursuite, 2 s de garde de distance, 1 s de fuite, en
répétant — à partir du démarrage de la partie. Prédisez les rapports de phases de
l'exécution : dans les quinze premières secondes de jeu, quelles phases
s'annoncent et quand (approximativement — les frames de l'exécution sont les
siennes), et quelle est la forme de la courbe de distance au héros du boss sur
un cycle complet ? Une mise en garde à intégrer dans votre prédiction : la
première frame de l'exécution sans écran peut être longue. Puis lancez-le et
comparez aux rapports du schéma.

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-090/ex2.md)

---

**Partie :** [Partie 5 — le jeu](../../index.md) ·
**Précédente :** [Leçon 089 — l'IA ennemie](lesson-089-enemy-ai.md) ·
**Suivante :** [Leçon 091 — vagues](lesson-091-waves.md) ·
**Étiquette de code :** [`lesson-090`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-090)

*Page traduite de la version anglaise `book/lessons/part-5/lesson-090-boss.md`,
révision `7ce2cfe`.*

<!-- translation-source: book/lessons/part-5/lesson-090-boss.md @ 7ce2cfe -->
