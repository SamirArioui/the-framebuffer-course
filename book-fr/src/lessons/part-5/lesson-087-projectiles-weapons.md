# Leçon 087 — les projectiles et les deux armes

{{#include ../../stability-horizon.md}}

## Prose

Le héros marche avec du poids et s'anime en le faisant — et rien dans le monde
ne riposte. Cette leçon, c'est le combat : **deux armes, et les projectiles
qu'elles tirent.** C'est aussi, d'abord, une leçon sur la donnée — parce que le
format de table des leçons 071-072 ne peut pas porter ce que savent une arme et
un projectile, et la façon dont il grandit est la décision de conception sur
laquelle toute cette série de leçons repose.

### Le format grandit par colonnes nommées

Que doit savoir le combat ? Combien de dégâts fait un coup. À quelle fréquence
une arme tire. Quel type de projectile une arme envoie. Quelle distance un
projectile parcourt avant que sa vie ne s'achève. Les sept colonnes de
`assets/entities.txt` — `name x y facing speed health sprite` — ne portent rien
de tout cela. Il y avait des tricheries tentantes : la vie pouvait se lire comme
des dégâts « pour les types où ça colle », la colonne facing pouvait porter un
identifiant d'arme. C'est l'échec que la leçon 071 a nommé : *le mauvais jeu qui
marche* — les valeurs mentent sur ce qu'elles sont, et aucun rapport ne l'attrape
jamais.

Le format grandit donc **par colonnes nommées, de façon additive**. `EntityDef`
et le chargeur gagnent des champs ; l'en-tête d'un fichier nomme les colonnes
qu'il utilise ; le chargeur remplit les champs nommés et **laisse le reste à ses
valeurs par défaut**. Le format grandi connaît huit colonnes de plus :

| Colonne | Type | Défaut | Ce qu'elle porte |
| ------- | ---- | ------ | ---------------- |
| `accel` | ms | 120 | la constante de temps du déplacement lissé — le ressenti |
| `damage` | points | 0 | ce qu'un coup retire (les dégâts d'une ligne d'arme) |
| `rate` | tirs/min | 0 | la cadence de tir d'une arme ; 0 = jamais |
| `fires` | texte | none | le type de projectile qu'une arme tire |
| `range` | pixels monde | 0 | le budget de vol d'un projectile — sa vie |
| `behavior` | texte | `none` | ce qu'un type fait à chaque frame : `none`, `fly`, `chase`, `keep`, `flee`, `boss` |
| `wave` | nombre | 0 | quelle vague fait apparaître ce type ; 0 = jamais par vague |
| `count` | nombre | 1 | combien de ce type rejoignent cette vague |

Le ressenti est de la donnée désormais, exactement comme la leçon 085 l'avait
annoncé : l'easing du héros lit son propre `accel`, pas une constante dans son
code. (La ligne du héros précède la colonne et continue de charger à l'octet
près — son poids est le défaut, les 120 ms livrés par la leçon 085.)

C'est un choix délibéré de **corriger en avançant** (fix-forward) une limite de
refus publiée. Le chargeur des leçons 071/072 refusait « une colonne pas nommée
du tout » : chaque fichier devait nommer chaque colonne. Cette règle avait un
sens quand il y avait sept colonnes et une seule sorte de ligne ; elle ne peut
pas survivre à un format où une ligne d'arme n'a pas de sprite et une ligne de
projectile pas de dégâts. La limite est desserrée d'exactement un cran : **un
fichier peut omettre n'importe quelle colonne que le format connaît, et les
champs non nommés prennent leurs valeurs par défaut.** Une colonne que le format
ne connaît *pas* reste un fichier malformé — et un nom dit deux fois aussi, ainsi
qu'une ligne avec une valeur de trop ou de moins, un behavior que le format ne
définit pas. D'une exécution jetable, un de chaque :

```
engine: assets/entities.txt: could not load (malformed)      <- the header named "sprit"
engine: def hero: x 312 y 232 facing 0 speed 240 health 3 sprite assets/hero.ppm accel 120 damage 0 rate 0 fires none range 0 behavior none wave 0 count 1
engine: assets/entities.txt: could not load (malformed)      <- a row one value short
```

La ligne du milieu est l'autre moitié de la preuve : un fichier nommant
seulement `name x y speed health sprite` a chargé, et les champs qu'il n'a jamais
nommés se lisent comme les défauts du format — `facing 0`, `accel 120`,
`behavior none`.

Et la règle de compatibilité tient sans négociation : **chaque fichier que le
cours a livré continue de charger à l'octet près.** `assets/entities.txt` n'est
pas touché du tout par cette leçon — `git diff lesson-086 lesson-087 --
assets/entities.txt` est vide — et la vérification au niveau de l'octet de
l'exécution imprime toujours ses deux lignes exactement comme la leçon 071 les a
écrites, désormais à côté des défauts que leur fichier ne nomme jamais :

```
engine: table: unnamed fields at their defaults — accel 120, damage 0, rate 0, fires none, range 0, behavior none, wave 0, count 1
engine: table assets/entities.txt: 2 definitions
engine: def hero: x 312 y 232 facing 0 speed 240 health 3 sprite assets/hero.ppm accel 120 damage 0 rate 0 fires none range 0 behavior none wave 0 count 1
engine: def slime: x 400 y 320 facing 2 speed 96 health 1 sprite assets/sprite.ppm accel 120 damage 0 rate 0 fires none range 0 behavior none wave 0 count 1
```

### Les armes sont des lignes

Les données de combat du jeu sont deux nouveaux fichiers au format grandi —
l'en-tête de chaque fichier nommant exactement les colonnes que ses lignes
utilisent. `assets/weapons.txt` dit ce qu'est une arme : une ligne qui nomme le
type de projectile qu'elle tire et porte sa cadence et ses dégâts.
`assets/projectiles.txt` dit ce qu'est un type de projectile : sa vitesse, son
art, sa vie, et `behavior fly` — le fait que sa ligne porte, comme tous les
autres.

```
name damage rate fires
blaster 1 300 bolt
cannon 2 60 shell
```

Une ligne d'arme n'est jamais une entité. Elle n'a ni position, ni vie, ni
travail par frame — ses faits sont seulement *portés*. Armer un tireur avec une
ligne d'arme copie ces faits sur lui, et à partir de là le tireur les porte
comme n'importe quelle entité porte les valeurs de sa ligne :

```
engine: arm: hero arms blaster (damage 1, rate 300, fires bolt)
engine: arm: slime arms cannon (damage 2, rate 60, fires shell)
```

Les touches numériques arment les lignes de la table des armes (`HeroFire`), et
la touche de tir envoie un tir à la cadence de la ligne — la cadence est un
plafond : la gâchette ne répond de nouveau que lorsque son temps de recharge est
écoulé. Les deux armes se répartissent exactement comme leurs lignes l'énoncent :
le blaster est rapide et faible (un tir toutes les 0,2 seconde, un point), le
cannon lent et fort (un tir par seconde, deux points) et — parce que les lignes
de projectiles le disent — l'obus du cannon vole plus loin que le trait du
blaster. D'une exécution, chaque arme tirant le type de projectile que sa ligne
énonce :

```
engine: arm: hero arms blaster (damage 1, rate 300, fires bolt)
engine: fire: hero -> bolt (damage 1, range 160)
engine: shot bolt retired — range
...
engine: arm: hero arms cannon (damage 2, rate 60, fires shell)
engine: fire: hero -> shell (damage 2, range 400)
engine: shot shell retired — wall
```

Le héros vise comme il court : le tir vole vers le point cardinal du mouvement
du héros — les mêmes huit directions, la même diagonale à 1/√2 — ou vers son
facing au repos.

### Les projectiles sont des entités

Le projectile n'est pas un système de particules ni un cas particulier : c'est
une entité, créée depuis sa définition à travers le magasin (store), exactement
comme le héros et le slime. Sa ligne lui donne sa vitesse, son art, sa portée ;
son tireur lui donne les dégâts qu'il infligera et le fait qu'il a été tiré
(`owner` — un tir ne touche jamais son tireur). Ce que la durée de vie du
magasin, la règle d'une visite par entité de la marche et le mover font déjà
pour les entités, ils le font désormais pour les projectiles gratuitement.

Son vol est la seule branche de comportement du travail par entité — `fly` —
sous-pas à travers le mover quelques pixels à la fois, pour qu'un tir rapide ne
puisse pas dépasser ce qu'il touche à l'intérieur d'une longue frame. À chaque
position — y compris celle d'où il est tiré, pour qu'un tir à bout portant
atterrisse — il regarde ce qu'il doit remarquer, et il se retire exactement à
l'une de trois fins : **un mur** (le mover a refusé le pas), **la fin de sa
portée** (le budget de vol de sa ligne est épuisé), ou **l'entité qu'il a
touchée**. De la même exécution, les trois :

```
engine: shot bolt retired — wall
engine: shot bolt retired — range
engine: hit: bolt hits slime — damage 1, health 1 -> 0
engine: shot bolt retired — hit slime
engine: slime retired — zero health
```

Le coup, ce sont les dégâts de la ligne, et les dégâts de la ligne sont tout ce
qu'il est : `damage 1` fait passer le slime de `1` à `0` exactement. Une entité
à zéro vie est retirée — avec une exception qui mérite d'être nommée : **le
héros n'est jamais retiré.** Sa vie à zéro est la condition de défaite propre au
jeu, que la machine à états lit ; l'acteur du jeu n'est pas retiré sous les
pieds du jeu. Une exécution qui mène la vie du héros à zéro deux fois — une fois
en mourant, une fois après qu'une nouvelle partie l'a restaurée — montre à la
fois la règle et l'exception :

```
engine: fire: slime -> shell (damage 2, range 400)
engine: hit: shell hits hero — damage 2, health 3 -> 1
engine: shot shell retired — hit hero
engine: fire: slime -> shell (damage 2, range 400)
engine: hit: shell hits hero — damage 2, health 1 -> 0
engine: shot shell retired — hit hero
engine: state play -> death (the hero's health reached zero)
engine: state death -> title (the player returned to the title)
engine: state title -> play (the player started)
engine: fire: slime -> shell (damage 2, range 400)
engine: hit: shell hits hero — damage 2, health 3 -> 1
```

`damage 2` deux fois : `3 -> 1`, `1 -> 0` — les nombres sont ceux de la ligne,
et rien d'autre. La dernière ligne est l'exception à l'œuvre : après que la
nouvelle partie a restauré la vie du héros à 3, le héros est touché de nouveau —
toujours vivant, toujours dans le magasin. Et les tirs de cette exécution
étaient le **substitut de tir ennemi** : `G` fait cracher le slime sur le héros,
une démonstration à touche comme celle de la leçon 082, parce que ce qui tient
lieu de substitut aujourd'hui, c'est seulement *qui tire sur le héros et quand*.
Le coup lui-même est du vrai combat — c'est ce que cette leçon a livré.

### Ce que cette exécution a vérifié, et ce qu'elle n'a pas vérifié

- **Chaque fichier de table livré charge inchangé** — `assets/entities.txt` à
  l'octet près (cette leçon ne le touche pas ; son diff est vide), ses lignes
  imprimant les mêmes valeurs qu'elles ont toujours eues, les champs non nommés
  aux défauts du format.
- **Chaque arme tire le type de projectile que sa ligne énonce** — les rapports
  d'armement et de tir : `blaster … fires bolt`, `cannon … fires shell`, et les
  tirs qui apparaissent sont `bolt` et `shell`.
- **Les projectiles se retirent aux murs, à la fin de leur portée, et aux
  entités qu'ils touchent** — les trois lignes de retrait, dans de vraies
  exécutions.
- **Un coup réduit la vie des dégâts de la ligne** — `damage 1` a mené le slime
  de `1 -> 0`, `damage 2` a mené le héros de `3 -> 1` et `1 -> 0` — et une
  entité à zéro vie est retirée, le héros excepté.

Ce que cette exécution n'a **pas** vérifié, c'est le *ressenti* des armes à
leurs cadences — la boucle de la machine de l'auteur est rythmée par une entrée
scriptée (environ 25 frames par seconde ici, et la gâchette tire au plus un coup
par frame), donc le plafond de cinq tirs par seconde du blaster se lit comme un
tir par frame sous le script. La cadence est un fait de la ligne dans tous les
cas ; la régler est un jugement du jeu, pris plus tard. La leçon ne dit pas non
plus encore *qui* sont les ennemis ni *quand* ils attaquent : les lignes
d'ennemis qui portent leurs propres armes arrivent à la leçon 088, les
comportements de déplacement à la 089, et les attaques qui remplacent le
substitut `G` à la 090.

## Étape de code

Un changement : le format grandit, et le combat repose dessus. `src/table.h` et
`src/table.cpp` gagnent les colonnes nommées (les défauts, les orthographes de
behavior, un en-tête qui nomme n'importe quel sous-ensemble — le fix-forward de
la limite de refus de la leçon 072) ; `src/entity.h/.cpp` portent les nouveaux
faits depuis la ligne ; `src/combat.h` et `src/combat.cpp` sont nouveaux —
l'armement, le tir et le vol du projectile (`CombatArm`, `CombatFire`,
`CombatFly`) ; `src/hero.h/.cpp` lisent le ressenti depuis l'`accel` de la ligne
et tirent l'arme du héros ; la marche de `src/game.cpp` branche sur le behavior
que la ligne porte (un projectile vole) ; `src/main.cpp` charge les deux
nouvelles tables du jeu et arme les tireurs ; `src/platform.h` et
`src/platform_x11.cpp` gagnent les trois touches dont le combat a besoin.
`assets/weapons.txt` et `assets/projectiles.txt` sont les données de combat du
jeu (nouvelles), et `assets/bolt.ppm` et `assets/shell.ppm` sont les deux
sprites de tir — deux images 16×16 à couleur clé magenta, le bolt un cœur
brillant avec une traînée, le shell un disque chaud avec un centre brûlant. Son
état final est étiqueté `lesson-087`.

```diff
diff --git a/assets/projectiles.txt b/assets/projectiles.txt
new file mode 100644
index 0000000..beb5ef6
--- /dev/null
+++ b/assets/projectiles.txt
@@ -0,0 +1,3 @@
+name speed sprite range behavior
+bolt 480 assets/bolt.ppm 160 fly
+shell 240 assets/shell.ppm 400 fly
diff --git a/assets/weapons.txt b/assets/weapons.txt
new file mode 100644
index 0000000..ee42dd4
--- /dev/null
+++ b/assets/weapons.txt
@@ -0,0 +1,3 @@
+name damage rate fires
+blaster 1 300 bolt
+cannon 2 60 shell
diff --git a/src/combat.cpp b/src/combat.cpp
new file mode 100644
index 0000000..905cfc7
--- /dev/null
+++ b/src/combat.cpp
@@ -0,0 +1,183 @@
+// combat.cpp — the combat's mechanics: arm, fire, fly, hit, retire.
+//
+// Lesson 087: every mechanic here is the data's. The damage a hit does is
+// the row's damage the shooter carries; the projectile is the kind its
+// row names; the flight is the projectile's speed over game time; the
+// life is its row's range. Nothing here knows which weapon or which
+// projectile exists — the tables know that.
+
+#include "combat.h"
+
+#include <cstdio>
+
+namespace engine {
+namespace {
+
+/* The two boxes of a shot and its target: what each draws is what each
+   is hit as — one ANIM_FRAME_W-wide frame of its art (lesson 086). */
+bool Overlaps(const Entity &a, const Entity &b)
+{
+    return a.x < b.x + ANIM_FRAME_W && b.x < a.x + ANIM_FRAME_W &&
+           a.y < b.y + b.sprite->height && b.y < a.y + a.sprite->height;
+}
+
+} /* namespace */
+
+void CombatAim(double vx, double vy, double &dir_x, double &dir_y)
+{
+    /* The aim is the compass point of the motion — one of the eight
+       directions the movement already knows, a diagonal at 1/sqrt(2) —
+       never a direction of some other length. At rest this is (0, 0). */
+    dir_x = 0.0;
+    dir_y = 0.0;
+    if (vx > 0.0)
+        dir_x = 1.0;
+    else if (vx < 0.0)
+        dir_x = -1.0;
+    if (vy > 0.0)
+        dir_y = 1.0;
+    else if (vy < 0.0)
+        dir_y = -1.0;
+    if (dir_x != 0.0 && dir_y != 0.0) {
+        dir_x *= AIM_DIAG;
+        dir_y *= AIM_DIAG;
+    }
+}
+
+void CombatArm(Entity &shooter, const EntityDef &weapon)
+{
+    /* A weapon is a row; carrying it means carrying its values. The
+       shooter is not tied to the row afterwards — it carries the facts
+       and may be armed with another row's. */
+    shooter.damage = weapon.damage;
+    shooter.rate = weapon.rate;
+    for (int i = 0; i < TABLE_NAME_MAX; ++i)
+        shooter.fires[i] = weapon.fires[i];
+    shooter.cooldown = 0.0;
+    std::printf("engine: arm: %s arms %s (damage %d, rate %d, fires %s)\n",
+                shooter.name, weapon.name, weapon.damage, weapon.rate,
+                weapon.fires[0] ? weapon.fires : "none");
+}
+
+bool CombatFire(EntityStore &store, const EntityTable &shots,
+                Entity &shooter, double dir_x, double dir_y)
+{
+    if (!shooter.fires[0])
+        return false; /* an unarmed shooter fires nothing */
+
+    /* The projectile kind is the row's fact, looked up where the kinds
+       live. A kind the table does not hold is a typed failure — never a
+       shot with assumed attributes. */
+    DefResult kind = TableFind(shots, shooter.fires);
+    if (kind.error != DEF_OK) {
+        std::printf("engine: fire refused — %s is no projectile kind\n",
+                    shooter.fires);
+        return false;
+    }
+
+    /* The projectile is an entity: created from its definition, through
+       the store, in a slot like every entity. */
+    EntityResult made = EntityCreate(store, *kind.def);
+    if (made.error != ENTITY_OK) {
+        std::printf("engine: fire refused — the store is full\n");
+        return false;
+    }
+
+    Entity &shot = *made.entity;
+    shot.x = shooter.x; /* a shot leaves its shooter's box */
+    shot.y = shooter.y;
+    shot.owner = &shooter; /* a shot never hits its owner */
+    shot.damage = shooter.damage; /* the row's damage, carried into the hit */
+    shot.move_x = dir_x;
+    shot.move_y = dir_y;
+    if (dir_x > 0.0)
+        shot.facing = 0;
+    else if (dir_y > 0.0)
+        shot.facing = 1;
+    else if (dir_x < 0.0)
+        shot.facing = 2;
+    else
+        shot.facing = 3;
+    std::printf("engine: fire: %s -> %s (damage %d, range %d)\n", shooter.name,
+                shot.name, shot.damage, shot.range);
+    return true;
+}
+
+void CombatFly(const TileMap &map, EntityStore &store, const Entity &hero,
+               Entity &shot, double dt)
+{
+    /* The flight, in game time: the shot's speed over dt, sub-stepped
+       through the mover so each sub-step is small. What is checked at
+       every position — including the one it is fired at, so a shot fired
+       point-blank lands — is what a projectile must notice: the entity
+       it hit, a wall (the step refused), and its range running out. */
+    double left = (double)shot.speed * dt;
+    for (;;) {
+        /* The first actor the shot overlaps, other than itself and its
+           owner. Slots are checked in order; the first hit is the hit.
+           A shot flies through other shots — a projectile hits the
+           living, and crossing fire does not cancel in mid-air. */
+        Entity *target = 0;
+        for (int i = 0; i < ENTITY_CAP && !target; ++i) {
+            Entity &e = store.slots[i];
+            if (!e.live || &e == &shot || &e == shot.owner)
+                continue;
+            if (e.behavior == BEHAVIOR_FLY)
+                continue;
+            if (Overlaps(shot, e))
+                target = &e;
+        }
+        if (target) {
+            /* The hit: the target's health falls by the row's damage,
+               and the shot is spent on it. */
+            int was = target->health;
+            target->health -= shot.damage;
+            if (target->health < 0)
+                target->health = 0;
+            std::printf("engine: hit: %s hits %s — damage %d, health %d -> %d\n",
+                        shot.name, target->name, shot.damage, was,
+                        target->health);
+            std::printf("engine: shot %s retired — hit %s\n", shot.name,
+                        target->name);
+            EntityRetire(store, shot);
+            if (target->health == 0 && target != &hero) {
+                /* A zero-health entity is retired — the hero excepted:
+                   its zero health is the game's defeat condition, which
+                   the state machine reads; the game's actor is not
+                   retired out from under the game. */
+                std::printf("engine: %s retired — zero health\n",
+                            target->name);
+                EntityRetire(store, *target);
+            }
+            return;
+        }
+
+        if (left <= 0.0)
+            return; /* this frame's flight is spent */
+        if (shot.traveled >= shot.range) {
+            /* The range is the shot's life: a projectile that has flown
+               its row's range retires in the air — after its last look
+               at what it might have hit. */
+            std::printf("engine: shot %s retired — range\n", shot.name);
+            EntityRetire(store, shot);
+            return;
+        }
+
+        double step = left < COMBAT_STEP ? left : COMBAT_STEP;
+        double was_x = shot.x, was_y = shot.y;
+        MoveEntity(map, shot, shot.move_x * step, shot.move_y * step);
+        bool moved_x = shot.move_x == 0.0 || shot.x != was_x;
+        bool moved_y = shot.move_y == 0.0 || shot.y != was_y;
+        if (!moved_x || !moved_y) {
+            /* The mover refused the step where the shot meant to go: a
+               projectile that meets a wall retires at it. */
+            std::printf("engine: shot %s retired — wall\n", shot.name);
+            EntityRetire(store, shot);
+            return;
+        }
+        shot.traveled += step;
+        left -= step;
+    }
+}
+
+} /* namespace engine */
diff --git a/src/combat.h b/src/combat.h
new file mode 100644
index 0000000..b052eaf
--- /dev/null
+++ b/src/combat.h
@@ -0,0 +1,65 @@
+// combat.h — the combat: weapons armed, shots fired, shots in flight.
+//
+// Lesson 087: weapons are rows, projectiles are entities (design D5). A
+// weapon is a table row that names the projectile definition it fires and
+// carries its rate and damage; arming a shooter carries those values onto
+// it. Firing creates a projectile entity from that definition — through
+// the store, like every entity — and the projectile moves through the
+// mover in game time. It retires at a wall, at its range's end, and at
+// the entity it hit; a hit reduces the target's health by the row's
+// damage and retires the projectile. A zero-health entity is retired —
+// the hero excepted, whose zero health is the game's defeat condition.
+//
+// This is the game layer's combat file pair, beside the services (D2):
+// the store, the mover, and the table stay exactly what they are.
+#ifndef COMBAT_H
+#define COMBAT_H
+
+#include "entity.h"
+#include "table.h"
+
+namespace engine {
+
+/* Lesson 087: the flight's sub-step, in world pixels. A projectile moves
+   through the mover a few pixels at a time, checking what it hit along
+   the way — a fast shot cannot pass what it hits, or cross a thin wall
+   inside one long frame. */
+constexpr double COMBAT_STEP = 4.0;
+
+/* 1 / sqrt(2): a diagonal aim is scaled by this, exactly as the hero's
+   diagonal intent is (lesson 085), so a shot covers ground at its speed
+   whichever of the eight directions it flies. */
+constexpr double AIM_DIAG = 0.70710678;
+
+/* The eight compass points: the aim of the motion (vx, vy), normalized.
+   At rest the result is (0, 0) and the caller picks its own fallback —
+   the hero fires along its facing. */
+void CombatAim(double vx, double vy, double &dir_x, double &dir_y);
+
+/* Arm a shooter from a weapon row: the row's damage, rate, and the
+   projectile kind it fires become the shooter's own values — the weapon
+   is a row, and carrying a weapon is carrying its row's facts. */
+void CombatArm(Entity &shooter, const EntityDef &weapon);
+
+/* Fire: a projectile entity created from the definition the shooter's
+   `fires` names, sent along (dir_x, dir_y) — a direction of unit length.
+   Answers false — the report names why — when the shooter names no
+   projectile kind, when the table holds no such definition, or when the
+   store has no slot: never a stolen entity, never a shot with assumed
+   attributes. */
+bool CombatFire(EntityStore &store, const EntityTable &shots,
+                Entity &shooter, double dir_x, double dir_y);
+
+/* One projectile's flight, once a frame of game time: sub-stepped
+   through the mover, retiring at a wall (the step refused), at its
+   range's end, and at the entity it hit. A hit reduces the target's
+   health by the shot's damage and retires the shot; a zero-health target
+   is retired too — the hero excepted, whose zero health is the game's
+   defeat condition (the state machine reads it, the game's actor is not
+   retired out from under the game). */
+void CombatFly(const TileMap &map, EntityStore &store, const Entity &hero,
+               Entity &shot, double dt);
+
+} /* namespace engine */
+
+#endif
diff --git a/src/entity.cpp b/src/entity.cpp
index b45cc1f..77cdd42 100644
--- a/src/entity.cpp
+++ b/src/entity.cpp
@@ -12,14 +12,21 @@ namespace engine {
 Entity EntityFromDef(const EntityDef &def)
 {
     Entity entity = {};
-    for (int i = 0; i < TABLE_NAME_MAX; ++i)
+    for (int i = 0; i < TABLE_NAME_MAX; ++i) {
         entity.name[i] = def.name[i];
+        entity.fires[i] = def.fires[i];
+    }
     entity.x = def.x;
     entity.y = def.y;
     entity.facing = def.facing;
     entity.speed = def.speed;
     entity.health = def.health;
     entity.sprite = def.image;
+    entity.accel = def.accel;
+    entity.damage = def.damage;
+    entity.rate = def.rate;
+    entity.range = def.range;
+    entity.behavior = def.behavior;
     return entity;
 }
 
diff --git a/src/entity.h b/src/entity.h
index 560296f..c214bb8 100644
--- a/src/entity.h
+++ b/src/entity.h
@@ -38,6 +38,23 @@ struct Entity {
                                   into motion */
     bool live;                 /* lesson 074: this entity exists — the
                                   slot's state, set by the store */
+
+    /* Lesson 087: the combat facts, carried from the row the entity was
+       created from (or, for a shooter armed with a weapon row, from that
+       row — a weapon's values are data like any other's). */
+    int accel;                 /* ms: the eased-move time constant — the
+                                  feel; the format's default if its row
+                                  named no accel column */
+    int damage;                /* points a hit from this entity removes */
+    int rate;                  /* rounds per minute; 0 = never fires */
+    char fires[TABLE_NAME_MAX]; /* the projectile kind it fires */
+    int range;                 /* a projectile's flight budget, pixels */
+    double traveled;           /* how far a projectile has flown */
+    const Entity *owner;       /* the shooter of a projectile — a shot
+                                  never hits its owner */
+    int behavior;              /* BehaviorKind, its row's: what this
+                                  entity does each frame */
+    double cooldown;           /* seconds until it may fire again */
 };
 
 /* An entity created from a definition: every attribute its row states,
diff --git a/src/game.cpp b/src/game.cpp
index 967f072..33101e3 100644
--- a/src/game.cpp
+++ b/src/game.cpp
@@ -12,23 +12,24 @@
 #include <cstdio>
 
 #include "blit.h"
+#include "combat.h"
 #include "text.h"
 #include "tilemap.h"
 #include "tiles.h"
 
 namespace engine {
 
-/* The demonstration stand-in, named so it cannot be mistaken for the
-   game. Lesson 082 has the state machine but not yet the gameplay that
-   drives two of its named conditions: combat reduces the hero's health
-   (lesson 087) and the waves spend the game's completion (lesson 091).
-   Until those land, keys stand in for them: SPACE is a hit on the hero,
-   and ENTER in play says the game is complete — the same kind of keyed
-   demonstration, and like lesson 078's script before the juice toolkit
-   drove it. Both are removed when the real triggers arrive; the
-   transitions they fire are the game's own (defeat on zero health,
-   completion on no waves). Keyed, they never fire on their own during a
-   gameplay test. */
+/* The demonstration stand-ins, named so they cannot be mistaken for the
+   game. Lesson 087 made the hit real: a projectile reduces its target's
+   health by its row's damage, and a zero-health entity is retired. What
+   still stands in is the *enemy's fire* — who shoots at the hero, and
+   when. Until the enemies' attacks land (lesson 090), the run's G key
+   makes the slime spit at the hero (a keyed stand-in in the run, like
+   the feel demonstration beside it), and ENTER in play below says the
+   game is complete — lesson 091's waves spend it for real. Both die
+   when the real triggers arrive; the transitions they fire are the
+   game's own (defeat on zero health, completion on no waves). Keyed,
+   they never fire on their own during a gameplay test. */
 
 /* A transition, named once here and printed the moment it happens, so a
    run shows the machine moving between states and why. */
@@ -86,19 +87,10 @@ void GameInput(Game &game, platform::Window *window, Entity &hero,
         break;
 
     case GAME_PLAY: {
-        /* Play's movement is the hero's own (HeroMove, lesson 085) — the
-           held direction read and eased into motion there. What is left
-           here is the play state's other input and the named conditions. */
-
-        /* The stand-in for combat: SPACE is a hit on the hero (lesson 087
-           makes real hits land). The named condition below reads the
-           health this lowers. */
-        if (platform::KeyPressed(window, platform::KEY_SPACE) &&
-            hero.health > 0) {
-            hero.health -= 1;
-            std::printf("engine: hero takes a hit — health %d (t=%.3f)\n",
-                        hero.health, game.play_clock);
-        }
+        /* Play's movement is the hero's own (HeroMove, lesson 085) and
+           its fire is the hero's weapon's (HeroFire, lesson 087) — both
+           the run's, in play. What is left here is the play state's
+           other input and the named conditions. */
 
         game.play_clock += wall_dt;
 
@@ -234,7 +226,8 @@ void GameDrawSprites(const Game &game, Framebuffer &fb,
     }
 }
 
-int GameWalk(EntityStore &store, const TileMap &map, double dt)
+int GameWalk(EntityStore &store, const TileMap &map, const Entity &hero,
+             double dt)
 {
     /* Lesson 084: the walk — every live entity, once per frame, in slot
        order, its movement resolved against the tilemap. The per-entity
@@ -242,13 +235,22 @@ int GameWalk(EntityStore &store, const TileMap &map, double dt)
        turns the request into motion one axis at a time, so an entity
        that meets a solid tile stops on that axis and slides along the
        wall on the other — and the facing follows where it is going.
-       The hero and every other entity resolve the same way. */
+
+       Lesson 087: the per-entity work branches on the entity's behavior
+       — the fact its row carries. A projectile flies: its own
+       sub-stepped flight through the same mover, retiring at walls, at
+       its range's end, and at the entity it hit. Every other behavior
+       leaves the request for the mover below. */
     int visited = 0;
     for (int i = 0; i < ENTITY_CAP; ++i) {
         if (!store.slots[i].live)
             continue;
         visited += 1;
         Entity &e = store.slots[i];
+        if (e.behavior == BEHAVIOR_FLY) {
+            CombatFly(map, store, hero, e, dt);
+            continue;
+        }
         MoveEntity(map, e, e.move_x * e.speed * dt, e.move_y * e.speed * dt);
         if (e.move_x > 0.0)
             e.facing = 0;
diff --git a/src/game.h b/src/game.h
index a031ef7..bde62f0 100644
--- a/src/game.h
+++ b/src/game.h
@@ -103,8 +103,13 @@ void GameDrawSprites(const Game &game, Framebuffer &fb,
 /* Lesson 084: the walk — the game resolves every live entity's movement
    against the tilemap. Each entity's movement request becomes motion
    through the mover (MoveEntity), one axis at a time, so it stops at a
-   solid tile and slides along a wall. Returns the visit count. */
-int GameWalk(EntityStore &store, const TileMap &map, double dt);
+   solid tile and slides along a wall. Lesson 087: a projectile's
+   behavior flies it (CombatFly — its own sub-stepped flight, retiring at
+   walls, at its range, at what it hits) instead of the request. The hero
+   is handed along for the combat's rules to know the game's actor by.
+   Returns the visit count. */
+int GameWalk(EntityStore &store, const TileMap &map, const Entity &hero,
+             double dt);
 
 } /* namespace engine */
 
diff --git a/src/hero.cpp b/src/hero.cpp
index a2798ab..8945263 100644
--- a/src/hero.cpp
+++ b/src/hero.cpp
@@ -6,6 +6,8 @@
 
 #include "hero.h"
 
+#include "combat.h"
+
 namespace engine {
 
 void HeroMove(Entity &hero, platform::Window *window, double dt)
@@ -31,13 +33,15 @@ void HeroMove(Entity &hero, platform::Window *window, double dt)
         intent_y *= HERO_DIAG;
     }
 
-    /* The ease: the velocity closes on the intent by dt/HERO_TIME each
-       frame — toward the intent when the player steers (acceleration),
-       toward rest when they let go (deceleration). A turn passes through
-       the ease instead of snapping to full speed the other way. The
-       hero's movement request carries the eased velocity; the walk turns
-       it into motion (move x speed = the velocity). */
-    double k = dt / HERO_TIME;
+    /* The ease: the velocity closes on the intent by dt/accel each frame
+       — toward the intent when the player steers (acceleration), toward
+       rest when they let go (deceleration). A turn passes through the
+       ease instead of snapping to full speed the other way. The feel is
+       the hero's own `accel` — its row's fact, milliseconds (lesson
+       087); an accel of 0 is instant weightless motion. The hero's
+       movement request carries the eased velocity; the walk turns it
+       into motion (move x speed = the velocity). */
+    double k = hero.accel > 0 ? dt / (hero.accel / 1000.0) : 1.0;
     if (k > 1.0)
         k = 1.0;
     hero.move_x += (intent_x - hero.move_x) * k;
@@ -61,4 +65,47 @@ void HeroMove(Entity &hero, platform::Window *window, double dt)
     }
 }
 
+void HeroFire(Entity &hero, platform::Window *window,
+              const EntityTable &weapons, const EntityTable &shots,
+              EntityStore &store, double dt)
+{
+    /* Lesson 087: the number keys arm the weapons table's rows. A weapon
+       is a row — arming carries its values — so the weapons grow as
+       rows: one row more is one key more, and no weapon code. */
+    if (platform::KeyPressed(window, platform::KEY_1) && weapons.count > 0)
+        CombatArm(hero, weapons.rows[0]);
+    if (platform::KeyPressed(window, platform::KEY_2) && weapons.count > 1)
+        CombatArm(hero, weapons.rows[1]);
+
+    /* The rate is the row's, in rounds per minute: the trigger answers
+       again only when the cooldown it earns has run out. At most one
+       shot per frame — a long frame is caught up by the next shot, never
+       by a burst of them. */
+    if (hero.cooldown > 0.0) {
+        hero.cooldown -= dt;
+        return;
+    }
+    if (!platform::KeyDown(window, platform::KEY_SPACE))
+        return;
+    if (hero.rate <= 0 || !hero.fires[0])
+        return; /* unarmed, or a row that never fires */
+
+    /* The aim: the compass point of the hero's motion — the eight
+       directions, a diagonal at 1/sqrt(2) — or its facing at rest. */
+    double dir_x = 0.0, dir_y = 0.0;
+    CombatAim(hero.move_x, hero.move_y, dir_x, dir_y);
+    if (dir_x == 0.0 && dir_y == 0.0) {
+        if (hero.facing == 0)
+            dir_x = 1.0;
+        else if (hero.facing == 1)
+            dir_y = 1.0;
+        else if (hero.facing == 2)
+            dir_x = -1.0;
+        else
+            dir_y = -1.0;
+    }
+    if (CombatFire(store, shots, hero, dir_x, dir_y))
+        hero.cooldown = 60.0 / (double)hero.rate;
+}
+
 } /* namespace engine */
diff --git a/src/hero.h b/src/hero.h
index c602349..6804096 100644
--- a/src/hero.h
+++ b/src/hero.h
@@ -17,13 +17,6 @@
 
 namespace engine {
 
-/* The hero's accel/decel time constant — the feel: roughly how long it
-   takes to ease from rest to full speed (or back). Lesson 085 keeps it
-   here as the hero's own fact; when the table format grows named
-   columns (lesson 087) the feel becomes data, like the hero's speed
-   already is. */
-constexpr double HERO_TIME = 0.12; /* seconds to close on the intent */
-
 /* 1 / sqrt(2): a diagonal intent is scaled by this so the hero covers
    ground at the straight-line speed, not sqrt(2) times it. */
 constexpr double HERO_DIAG = 0.70710678;
@@ -36,9 +29,22 @@ constexpr double ANIM_STEP = 0.12;
    hero's velocity eases toward that intent (accel) and toward rest
    (decel) — a turn passes through the ease rather than snapping. The
    result is left in the hero's own movement request, which the walk
-   turns into motion against the map. */
+   turns into motion against the map. The ease's time constant is the
+   hero's own `accel` — its row's fact since lesson 087 grew the format
+   by named columns (the hero's row predates the column and keeps loading
+   byte-for-byte, its weight the format's default). */
 void HeroMove(Entity &hero, platform::Window *window, double dt);
 
+/* Lesson 087: the hero's weapon, once per frame of play. The number keys
+   arm the weapons table's rows — a weapon is a row, and carrying it is
+   carrying its values — and the fire key sends a shot along the hero's
+   motion (the eight compass points of its velocity) or, at rest, along
+   its facing. The shot is an entity like any other; the rate its row
+   states is the ceiling on how often the trigger answers. */
+void HeroFire(Entity &hero, platform::Window *window,
+              const EntityTable &weapons, const EntityTable &shots,
+              EntityStore &store, double dt);
+
 } /* namespace engine */
 
 #endif
diff --git a/src/main.cpp b/src/main.cpp
index e1993f9..58ff0bd 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -14,6 +14,7 @@
 #include "arena.h"
 #include "audio.h"
 #include "blit.h"
+#include "combat.h"
 #include "entity.h"
 #include "feel.h"
 #include "font.h"
@@ -93,6 +94,75 @@ static bool LoadRunSample(Arena &arena, const char *path, Sample &into)
     return false;
 }
 
+/* Lesson 087: one table load's whole failure path, the same shape — the
+   load either hands over every definition or names what went wrong typed
+   and the run ends by name. Used for every table file the game loads. */
+static bool LoadRunTable(Arena &arena, const char *path, EntityTable &into)
+{
+    TableResult loaded = LoadTable(arena, path);
+    if (loaded.error == TABLE_OK) {
+        into = loaded.table;
+        return true;
+    }
+    switch (loaded.error) {
+    case TABLE_MISSING:
+        std::fprintf(stderr, "engine: %s: could not load (missing)\n", path);
+        break;
+    case TABLE_MALFORMED:
+        std::fprintf(stderr, "engine: %s: could not load (malformed)\n", path);
+        break;
+    default:
+        std::fprintf(stderr, "engine: %s: could not load (no room)\n", path);
+        break;
+    }
+    return false;
+}
+
+/* Lesson 073/087: the definitions' art, loaded at startup. The sprite
+   column names the file; the run loads each one and hands the definition
+   its image, so an entity created from the definition is answered from
+   the definition alone. A row that names no sprite (a weapon row) has no
+   art and needs none. */
+static bool LoadRunArt(Arena &arena, EntityTable &table)
+{
+    Sprite *images = (Sprite *)ArenaAlloc(
+        arena, (size_t)table.count * sizeof(Sprite), 4);
+    if (!images) {
+        std::fprintf(stderr, "engine: no room for the definitions' art\n");
+        return false;
+    }
+    for (int i = 0; i < table.count; ++i) {
+        EntityDef &def = table.rows[i];
+        if (!def.sprite[0])
+            continue;
+        SpriteResult art = LoadSprite(arena, def.sprite);
+        if (art.error != SPRITE_OK) {
+            std::fprintf(stderr, "engine: %s: could not load\n", def.sprite);
+            return false;
+        }
+        images[i] = art.sprite;
+        def.image = &images[i];
+    }
+    return true;
+}
+
+/* Lesson 087: the byte-level check on a table, before anything uses it —
+   every definition, carrying every field: the values its row states and
+   the format's defaults for the columns its file did not name. */
+static void PrintDefs(const char *path, const EntityTable &table)
+{
+    std::printf("engine: table %s: %d definition%s\n", path, table.count,
+                table.count == 1 ? "" : "s");
+    for (int i = 0; i < table.count; ++i) {
+        const EntityDef &def = table.rows[i];
+        std::printf("engine: def %s: x %d y %d facing %d speed %d health %d sprite %s accel %d damage %d rate %d fires %s range %d behavior %s wave %d count %d\n",
+                    def.name, def.x, def.y, def.facing, def.speed, def.health,
+                    def.sprite[0] ? def.sprite : "none", def.accel, def.damage,
+                    def.rate, def.fires[0] ? def.fires : "none", def.range,
+                    BehaviorName(def.behavior), def.wave, def.count);
+    }
+}
+
 int Run(void)
 {
     platform::WindowResult opened =
@@ -153,68 +223,51 @@ int Run(void)
     }
     TileSheet &sheet = tiles_loaded.sheet;
 
-    /* Lesson 071: the run's entities are data. The table file holds one
+    /* Lesson 071: the run's entities are data. A table file holds one
        row per definition — its columns named by its header — and the load
        either hands over every definition or names what went wrong, like
        every asset above. Lesson 072: the rows are the arena's, and a
-       refused load keeps none of them. */
-    TableResult table_loaded = LoadTable(arena, "assets/entities.txt");
-    if (table_loaded.error != TABLE_OK) {
-        switch (table_loaded.error) {
-        case TABLE_MISSING:
-            std::fprintf(stderr,
-                         "engine: assets/entities.txt: could not load (missing)\n");
-            break;
-        case TABLE_MALFORMED:
-            std::fprintf(stderr,
-                         "engine: assets/entities.txt: could not load (malformed)\n");
-            break;
-        default:
-            std::fprintf(stderr,
-                         "engine: assets/entities.txt: could not load (no room)\n");
-            break;
-        }
+       refused load keeps none of them. Lesson 087: the format grew by
+       named columns — and this file keeps loading byte-for-byte, its
+       seven columns exactly as lesson 071 wrote them, every field it
+       never named at the format's default. */
+    EntityTable table;
+    if (!LoadRunTable(arena, "assets/entities.txt", table)) {
         platform::CloseWindow(opened.window);
         ArenaRelease(arena);
         return 1;
     }
-    EntityTable &table = table_loaded.table;
 
-    /* The byte-level check, before anything uses the table: every
-       definition, carrying the values its row states. */
-    std::printf("engine: table: %d definition%s\n", table.count,
-                table.count == 1 ? "" : "s");
-    for (int i = 0; i < table.count; ++i) {
-        const EntityDef &def = table.rows[i];
-        std::printf("engine: def %s: x %d y %d facing %d speed %d health %d sprite %s\n",
-                    def.name, def.x, def.y, def.facing, def.speed, def.health,
-                    def.sprite);
+    /* Lesson 087: the game's own data, in the grown format. The weapons
+       are rows that name the projectile kind they fire and carry their
+       rate and damage; the projectile kinds are rows a fired shot is an
+       entity of. Each file's header names the columns it uses — and only
+       those; what it leaves unnamed sits at the format's defaults. */
+    EntityTable weapons, shots;
+    if (!LoadRunTable(arena, "assets/weapons.txt", weapons) ||
+        !LoadRunTable(arena, "assets/projectiles.txt", shots)) {
+        platform::CloseWindow(opened.window);
+        ArenaRelease(arena);
+        return 1;
     }
 
-    /* Lesson 073: the definitions' art, loaded at startup. The table's
-       sprite column names the file; the run loads each one and hands the
-       definition its image, so an entity created from a definition is
-       answered from the definition alone. */
-    Sprite *images = (Sprite *)ArenaAlloc(
-        arena, (size_t)table.count * sizeof(Sprite), 4);
-    if (!images) {
-        std::fprintf(stderr, "engine: no room for the definitions' art\n");
+    /* The byte-level check, before anything uses the tables: every
+       definition of every table, carrying every field — the values its
+       row states and the format's defaults for the columns its file did
+       not name. */
+    std::printf("engine: table: unnamed fields at their defaults — accel %d, damage 0, rate 0, fires none, range 0, behavior none, wave 0, count 1\n",
+                TABLE_ACCEL_DEFAULT);
+    PrintDefs("assets/entities.txt", table);
+    PrintDefs("assets/weapons.txt", weapons);
+    PrintDefs("assets/projectiles.txt", shots);
+
+    /* Lesson 073: the definitions' art, loaded at startup. A row that
+       names no sprite (a weapon row) has no art and needs none. */
+    if (!LoadRunArt(arena, table) || !LoadRunArt(arena, shots)) {
         platform::CloseWindow(opened.window);
         ArenaRelease(arena);
         return 1;
     }
-    for (int i = 0; i < table.count; ++i) {
-        EntityDef &def = table.rows[i];
-        SpriteResult art = LoadSprite(arena, def.sprite);
-        if (art.error != SPRITE_OK) {
-            std::fprintf(stderr, "engine: %s: could not load\n", def.sprite);
-            platform::CloseWindow(opened.window);
-            ArenaRelease(arena);
-            return 1;
-        }
-        images[i] = art.sprite;
-        def.image = &images[i];
-    }
 
     /* Lesson 073: the game's first entity — created from the hero's
        definition, carrying the values its row states in named fields the
@@ -263,6 +316,7 @@ int Run(void)
        same table, one entity per row. A new row is a new entity; the
        run has no per-kind code to grow. */
     int created = 1;
+    Entity *foe = 0; /* the world's one enemy row (the slime) */
     for (int i = 0; i < table.count; ++i) {
         if (&table.rows[i] == hero_def.def)
             continue;
@@ -280,11 +334,24 @@ int Run(void)
            walls and stops it at solid tiles. */
         made.entity->move_x = 1.0;
         made.entity->move_y = 1.0;
+        if (!foe)
+            foe = made.entity;
         created += 1;
     }
     std::printf("engine: world: %d entities from the table's rows, live %d of %d\n",
                 created, store.live, ENTITY_CAP);
 
+    /* Lesson 087: weapons are rows. The hero starts armed with the
+       weapons table's first row; the number keys arm the rest (HeroFire).
+       The demonstration stand-in arms the foe with the second row — its
+       projectile is what G spits at the hero. The enemy rows that carry
+       their own attacks arrive in lesson 088; this stand-in and its key
+       die when those attacks land (lesson 090). */
+    if (weapons.count > 0)
+        CombatArm(hero, weapons.rows[0]);
+    if (weapons.count > 1 && foe)
+        CombatArm(*foe, weapons.rows[1]);
+
     /* The lookup's typed failure, checked on purpose: a definition the
        table does not hold is a value — never an entity with assumed
        attributes. */
@@ -348,7 +415,7 @@ int Run(void)
     std::printf("engine: sound %d-frame music looping on channel %d, %d-frame effect on the pool; one mixer of %d channels\n",
                 music.frame_count, AUDIO_MUSIC_CHANNEL, effect.frame_count,
                 AUDIO_MIXER_CHANNELS);
-    std::printf("engine: arrow keys move the hero, space shakes the camera; close the window to stop\n");
+    std::printf("engine: arrows move the hero, 1 and 2 arm the weapons, space fires, G is the enemy spit; close the window to stop\n");
     std::printf("engine: hero at %.0f,%.0f\n", hero.x, hero.y);
 
     /* Lesson 059: the run's sound is a run of amplitude at the engine's
@@ -451,16 +518,34 @@ int Run(void)
 
         /* Lesson 085: the hero's movement — the held direction eased into
            motion (accel/decel, the diagonal at the straight-line speed).
-           Only in play; the walk turns the eased velocity into steps. */
-        if (game.state == GAME_PLAY)
+           Lesson 087: and its weapon — the number keys arm the weapons
+           table's rows, the fire key sends a shot. Only in play; the
+           walk turns the eased velocity into steps and the shot into its
+           flight. */
+        if (game.state == GAME_PLAY) {
             HeroMove(hero, opened.window, dt);
+            HeroFire(hero, opened.window, weapons, shots, store, dt);
+
+            /* Lesson 087: the enemy-fire stand-in — G makes the slime
+               spit at the hero. What it demonstrates is real: the shot
+               is an entity, its hit reduces the hero's health by the
+               row's damage, and the hero's zero health is the game's
+               defeat. Only the shooter and its aim are scripted — the
+               enemies' own attacks (lesson 090) replace this key. */
+            if (foe && platform::KeyPressed(opened.window, platform::KEY_G)) {
+                double dir_x = 0.0, dir_y = 0.0;
+                CombatAim(hero.x - foe->x, hero.y - foe->y, dir_x, dir_y);
+                CombatFire(store, shots, *foe, dir_x, dir_y);
+            }
+        }
 
         /* Lesson 084: the game resolves its movement against its map —
            the walk is the game's now (GameWalk, in game.cpp), turning
-           every live entity's request into motion through the mover.
-           The loop times it as the frame record's entity sub-phase. */
+           every live entity's request into motion through the mover (and
+           every projectile into its flight). The loop times it as the
+           frame record's entity sub-phase. */
         double t_entities = platform::Now();
-        int visited = GameWalk(store, map, dt);
+        int visited = GameWalk(store, map, hero, dt);
         frame.entities = platform::Now() - t_entities;
         walk_visits += visited;
 
diff --git a/src/platform.h b/src/platform.h
index 4473e68..cd860ae 100644
--- a/src/platform.h
+++ b/src/platform.h
@@ -32,7 +32,9 @@ struct WindowResult {
 WindowResult OpenWindow(int width, int height);
 
 /* The keys the engine tracks. Plain values — no OS key code ever crosses
-   the seam. */
+   the seam. Lesson 087: the game's combat needs three more (the two
+   weapon rows the number keys arm, and the enemy-fire demonstration
+   key); a second OS maps its own three. */
 enum Key {
     KEY_UP = 0,
     KEY_DOWN,
@@ -41,6 +43,9 @@ enum Key {
     KEY_SPACE,
     KEY_ENTER,
     KEY_ESCAPE,
+    KEY_1,
+    KEY_2,
+    KEY_G,
     KEY_COUNT
 };
 
diff --git a/src/platform_x11.cpp b/src/platform_x11.cpp
index a251f11..bd91201 100644
--- a/src/platform_x11.cpp
+++ b/src/platform_x11.cpp
@@ -164,6 +164,9 @@ static int KeyIndex(KeySym sym)
     case XK_space:  return KEY_SPACE;
     case XK_Return: return KEY_ENTER;
     case XK_Escape: return KEY_ESCAPE;
+    case XK_1:      return KEY_1;
+    case XK_2:      return KEY_2;
+    case XK_g:      return KEY_G;
     default:        return -1;
     }
 }
diff --git a/src/table.cpp b/src/table.cpp
index ec1b59e..fd179ab 100644
--- a/src/table.cpp
+++ b/src/table.cpp
@@ -12,6 +12,13 @@
 // file's fact, so the file is walked once to count them and once to fill
 // them, and the whole load is bracketed by a mark — a refused load rolls
 // the arena back and keeps nothing.
+//
+// Lesson 087: the format grows by named columns, additively. The header
+// names the columns a file uses — any subset of the ones below — and the
+// fill starts every row at the format's defaults (DefaultRow), writing
+// only the named fields. A file that omits a column is no longer
+// refused; its fields sit at their defaults. A column the format does
+// not know is still malformed.
 
 #include "table.h"
 
@@ -98,7 +105,7 @@ bool ReadText(const unsigned char *line, int len, int &at, char *out,
     return true;
 }
 
-/* The columns the format knows. */
+/* The columns the format knows (lesson 087: grown by named columns). */
 enum Column {
     COL_NAME,
     COL_X,
@@ -107,13 +114,54 @@ enum Column {
     COL_SPEED,
     COL_HEALTH,
     COL_SPRITE,
-    COL_COUNT
+    COL_ACCEL,
+    COL_DAMAGE,
+    COL_RATE,
+    COL_FIRES,
+    COL_RANGE,
+    COL_BEHAVIOR,
+    COL_WAVE,
+    COL_SPAWN_COUNT,
+    COL_COUNT /* how many columns the format knows, not a column */
 };
 
 const char *const COLUMN_NAMES[COL_COUNT] = {
-    "name", "x", "y", "facing", "speed", "health", "sprite"
+    "name", "x", "y", "facing", "speed", "health", "sprite",
+    "accel", "damage", "rate", "fires", "range", "behavior", "wave",
+    "count"
+};
+
+/* Lesson 087: the behavior column's spellings, the format's own. */
+const char *const BEHAVIOR_NAMES[BEHAVIOR_COUNT] = {
+    "none", "fly", "chase", "keep", "flee", "boss"
 };
 
+/* Lesson 087: one row at the format's defaults, before the named fields
+   are written. A file may omit any column; whatever it omits keeps the
+   value set here — the defaults are part of the format's contract. */
+void DefaultRow(EntityDef &def)
+{
+    for (int i = 0; i < TABLE_NAME_MAX; ++i) {
+        def.name[i] = 0;
+        def.fires[i] = 0;
+    }
+    for (int i = 0; i < TABLE_PATH_MAX; ++i)
+        def.sprite[i] = 0;
+    def.x = 0;
+    def.y = 0;
+    def.facing = 0;
+    def.speed = 0;
+    def.health = 0;
+    def.image = 0;
+    def.accel = TABLE_ACCEL_DEFAULT;
+    def.damage = 0;
+    def.rate = 0;
+    def.range = 0;
+    def.behavior = BEHAVIOR_NONE;
+    def.wave = 0;
+    def.count = 1;
+}
+
 bool TokenIs(const unsigned char *token, int token_len, const char *name)
 {
     int n = 0;
@@ -144,8 +192,31 @@ int FindColumn(const unsigned char *token, int token_len)
     return -1;
 }
 
+/* Lesson 087: a behavior value — one of the format's spellings, mapped
+   to its number. A spelling the format does not define is refused, like
+   a facing that is not one of the four. */
+bool ReadBehavior(const unsigned char *line, int len, int &at, int &out)
+{
+    char text[TABLE_NAME_MAX];
+    if (!ReadText(line, len, at, text, TABLE_NAME_MAX))
+        return false;
+    for (int b = 0; b < BEHAVIOR_COUNT; ++b)
+        if (SameText(text, BEHAVIOR_NAMES[b])) {
+            out = b;
+            return true;
+        }
+    return false;
+}
+
 } /* namespace */
 
+const char *BehaviorName(int behavior)
+{
+    if (behavior < 0 || behavior >= BEHAVIOR_COUNT)
+        return "?";
+    return BEHAVIOR_NAMES[behavior];
+}
+
 TableResult LoadTable(Arena &arena, const char *path)
 {
     TableResult result = {};
@@ -199,23 +270,29 @@ TableResult LoadTable(Arena &arena, const char *path)
     ok = ok && NextLine(lines, line, len);
     int at = 0;
     int order[COL_COUNT];
+    int named = 0; /* lesson 087: how many columns this file names */
     for (int c = 0; c < COL_COUNT; ++c)
         order[c] = -1;
-    for (int i = 0; ok && i < COL_COUNT; ++i) {
+    /* Lesson 087: the header names the columns this file's rows carry —
+       any subset of the format's, each at most once, at least one. A
+       column the header does not name is not a refusal any more: the
+       field sits at the format's default. A name the format does not
+       know is still refused here, and so is a name said twice. */
+    while (ok) {
         const unsigned char *token = 0;
         int token_len = 0;
-        ok = ok && NextToken(line, len, at, token, token_len);
+        if (!NextToken(line, len, at, token, token_len))
+            break; /* the header's line ends */
+        int column = FindColumn(token, token_len);
+        ok = ok && column >= 0;
+        for (int prev = 0; ok && prev < named; ++prev)
+            ok = ok && order[prev] != column; /* one name, one column */
         if (ok) {
-            int column = FindColumn(token, token_len);
-            ok = ok && column >= 0;
-            for (int prev = 0; ok && prev < i; ++prev)
-                ok = ok && order[prev] != column; /* one name, one column */
-            order[i] = column;
+            order[named] = column;
+            named += 1;
         }
     }
-    const unsigned char *extra = 0;
-    int extra_len = 0;
-    ok = ok && !NextToken(line, len, at, extra, extra_len);
+    ok = ok && named > 0;
 
     while (ok) {
         if (!NextLine(lines, line, len))
@@ -228,9 +305,12 @@ TableResult LoadTable(Arena &arena, const char *path)
         }
 
         EntityDef &def = defs[result.table.count];
-        def.image = 0; /* the run hands the definition its art, not the file */
+        /* Lesson 087: the row begins at the format's defaults. The named
+           fields below overwrite theirs; every field the header does not
+           name keeps its default. */
+        DefaultRow(def);
         at = 0;
-        for (int i = 0; ok && i < COL_COUNT; ++i) {
+        for (int i = 0; ok && i < named; ++i) {
             switch (order[i]) {
             case COL_NAME:
                 ok = ReadText(line, len, at, def.name, TABLE_NAME_MAX);
@@ -254,6 +334,30 @@ TableResult LoadTable(Arena &arena, const char *path)
             case COL_SPRITE:
                 ok = ReadText(line, len, at, def.sprite, TABLE_PATH_MAX);
                 break;
+            case COL_ACCEL:
+                ok = ReadInt(line, len, at, def.accel);
+                break;
+            case COL_DAMAGE:
+                ok = ReadInt(line, len, at, def.damage);
+                break;
+            case COL_RATE:
+                ok = ReadInt(line, len, at, def.rate);
+                break;
+            case COL_FIRES:
+                ok = ReadText(line, len, at, def.fires, TABLE_NAME_MAX);
+                break;
+            case COL_RANGE:
+                ok = ReadInt(line, len, at, def.range);
+                break;
+            case COL_BEHAVIOR:
+                ok = ReadBehavior(line, len, at, def.behavior);
+                break;
+            case COL_WAVE:
+                ok = ReadInt(line, len, at, def.wave);
+                break;
+            case COL_SPAWN_COUNT:
+                ok = ReadInt(line, len, at, def.count);
+                break;
             default:
                 ok = false;
                 break;
diff --git a/src/table.h b/src/table.h
index fae166d..c4d7572 100644
--- a/src/table.h
+++ b/src/table.h
@@ -10,11 +10,22 @@
 //   slime 400 320 2 96 1 assets/sprite.ppm
 //
 // The header names the columns, and the loader fills the fields the
-// header declares — so the columns may come in any order, but every
-// column the format knows comes exactly once. `name` and `sprite` are
-// text (a run of non-space bytes); x, y, facing, speed, and health are
-// whole numbers, and facing is one of the four the format defines:
-// 0 right, 1 down, 2 left, 3 up.
+// header declares — so the columns may come in any order, and every
+// column the format knows comes at most once. `name`, `sprite`, and
+// `fires` are text (a run of non-space bytes); the rest are whole
+// numbers, and facing is one of the four the format defines: 0 right,
+// 1 down, 2 left, 3 up.
+//
+// Lesson 087: the format grows by named columns, additively. A file's
+// header names the columns it uses — and may omit any column the format
+// knows; the unnamed fields take the defaults the format defines below.
+// This is a deliberate fix-forward of lesson 071/072's refusal edge ("a
+// column not named at all"): a file may now omit columns, because the
+// game's facts (a weapon's damage, a projectile's life) must not be
+// crammed into columns that would lie about them, and every file the
+// course has shipped keeps loading byte-for-byte. A column the format
+// does *not* know is still a malformed file — and so is a name the
+// table already holds, or a row with one value too few or too many.
 #ifndef TABLE_H
 #define TABLE_H
 
@@ -31,12 +42,29 @@ namespace engine {
 constexpr int TABLE_NAME_MAX = 16;
 constexpr int TABLE_PATH_MAX = 64;
 
+/* Lesson 087: the behavior column's values — what a kind does each
+   frame, the fact its row carries. The format defines the spellings,
+   like it defines facing's four numbers: `none` stands still, `fly` is a
+   projectile in flight, and chase / keep / flee / boss are the enemy
+   behaviors (lessons 089-090) — the boss's value names its pattern. */
+enum BehaviorKind {
+    BEHAVIOR_NONE = 0,
+    BEHAVIOR_FLY,
+    BEHAVIOR_CHASE,
+    BEHAVIOR_KEEP,
+    BEHAVIOR_FLEE,
+    BEHAVIOR_BOSS,
+    BEHAVIOR_COUNT
+};
+
 /* One definition: a row of the table, carrying every value its row
    states — the identity and the attributes an entity is created from.
    The image is the one fact the file states as a name: the run loads the
    art the sprite column names and hands the definition its image, so an
    entity created from the definition is answered from the definition
-   alone. */
+   alone. A weapon is one of these rows (lesson 087): it names the
+   projectile kind it fires and carries its rate and damage — and is
+   never itself an entity. */
 struct EntityDef {
     char name[TABLE_NAME_MAX];   /* the definition's identity */
     int x, y;                    /* where it starts, in world pixels */
@@ -45,8 +73,34 @@ struct EntityDef {
     int health;                  /* points */
     char sprite[TABLE_PATH_MAX]; /* the art file it draws */
     const Sprite *image;         /* that art, loaded at startup */
+
+    /* Lesson 087: the format's named columns. Every field below takes
+       its default when a file's header does not name it — the defaults
+       are the format's contract, not a gap in it. */
+    int accel;                   /* ms: the eased-move time constant —
+                                    the feel (the hero's weight) */
+    int damage;                  /* points a hit removes — a weapon
+                                    row's damage, carried by its shots */
+    int rate;                    /* rounds per minute; 0 = never fires */
+    char fires[TABLE_NAME_MAX];  /* the projectile kind this row fires */
+    int range;                   /* world pixels: a projectile's flight
+                                    budget — its life */
+    int behavior;                /* BehaviorKind, its row's */
+    int wave;                    /* which wave spawns this kind;
+                                    0 = never by wave */
+    int count;                   /* how many of this kind join the wave */
 };
 
+/* Lesson 087: the defaults the format defines. A file may omit any
+   column; the field takes the value named here. The accel default is
+   the feel lesson 085 shipped as a constant — the hero's row predates
+   the column and keeps loading byte-for-byte, and its weight is exactly
+   this. */
+constexpr int TABLE_ACCEL_DEFAULT = 120; /* ms */
+
+/* A behavior value's spelling (and back), for the format's own reports. */
+const char *BehaviorName(int behavior);
+
 /* A loaded table: one definition per row, in the arena — as many rows as
    the file has, and not one more. */
 struct EntityTable {
@@ -71,11 +125,14 @@ struct TableResult {
 
 /* Loads an entity table from a file read whole. The header and the rows
    are parsed byte by byte — no library reads it — and anything the format
-   does not describe is refused typed: a column it does not know, a row
-   with the wrong number of values, a value where a number is required, a
-   value where text is, a name the table already holds. The rows are
-   copied into the arena behind a mark, and every refusal path rolls back
-   to it: a load that refuses leaves nothing behind. */
+   does not describe is refused typed: a column it does not know, a column
+   named twice, a row with the wrong number of values (exactly as many as
+   the header names), a value where a number is required, a value where
+   text is, a behavior the format does not define, a name the table
+   already holds. The rows are copied into the arena behind a mark, and
+   every refusal path rolls back to it: a load that refuses leaves
+   nothing behind. (Lesson 087: a column the header does *not* name is
+   no longer a refusal — its field takes the format's default.) */
 TableResult LoadTable(Arena &arena, const char *path);
 
 /* Lesson 073: a definition lookup — the request the game makes when it
```

## Exercices

Deux défis plus vastes. Chacun se termine par sa solution — un diff contre
l'état final de cette leçon, plus une visite guidée — après l'énoncé.

### Exercice 1 — Le tir en éventail *(extend-the-code)*

Les deux armes couvrent chacune une ligne droite. Donnez au jeu une arme à
dispersion : une troisième ligne d'arme dont la gâchette tire **un éventail de
trois tirs à la fois** — celui du milieu le long de la visée, un tourné d'un
cran de chaque côté — et armez-la sur une troisième touche numérique. La ligne
doit *énoncer* combien de tirs sa gâchette tire d'un coup, donc le format
grandit encore : faites-le comme cette leçon l'a fait grandir (nommé, additif,
avec défaut) et vérifiez que chaque fichier livré charge toujours à l'octet
près. Une contrainte sur la dispersion : chaque tir doit voler à la vitesse
propre du projectile — une dispersion dont les tirs latéraux rampent est une
dispersion qui ment. Puis lancez-le et regardez les retraits des trois tirs :
que vous disent leurs sorts sur l'éventail que vous avez construit ?

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-087/ex1.md)

### Exercice 2 — Le projectile sans art *(fix-the-crash)*

Donnez au fichier de la table des projectiles un en-tête qui ne nomme aucune
colonne `sprite` — le format l'autorise (une ligne d'arme ne nomme pas d'art non
plus) — et tirez l'arme qui nomme un de ces types de projectiles. L'exécution
meurt. Déterminez exactement *où* l'exécution meurt et *pourquoi* elle ne peut
pas survivre à une définition sans art ; corrigez-la ensuite pour que la
défaillance soit **typée et nommée**, comme chaque autre refus auquel ce moteur
répond — et pour que les lignes d'armes, qui ne portent pas d'art non plus,
continuent de fonctionner exactement comme avant.

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-087/ex2.md)

---

**Partie :** [Partie 5 — le jeu](../../index.md) ·
**Précédente :** [Leçon 086 — la rétroaction et l'animation](lesson-086-feedback-animation.md) ·
**Suivante :** [Leçon 088 — les tables d'archétypes des ennemis](lesson-088-enemy-tables.md) ·
**Étiquette de code :** [`lesson-087`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-087)

*Page traduite de la version anglaise `book/lessons/part-5/lesson-087-projectiles-weapons.md`,
révision `3fd99b7`.*

<!-- translation-source: book/lessons/part-5/lesson-087-projectiles-weapons.md @ 3fd99b7 -->
