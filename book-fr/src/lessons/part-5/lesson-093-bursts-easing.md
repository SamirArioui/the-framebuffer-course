# Leçon 093 — rafales de particules et easing

{{#include ../../stability-horizon.md}}

## Prose

La boîte à outils est à deux effets d'être complète. Le hitstop et le
screenshake se déclenchent depuis les événements du jeu depuis la dernière
leçon ; aujourd'hui arrivent les deux autres que le contrat autorise — **les
rafales de particules** et **l'easing** — et avec eux la boîte à outils est
finie : quatre effets, et pas un cinquième. Ce qui arrive ici n'est pas un
système de particules boulonné à côté du moteur. Une particule est **une
entité** — une ligne d'une table, un emplacement du magasin, une visite de la
marche — et tout ce que le magasin a toujours promis (le refus de la leçon 074
qui ne vole jamais) s'y applique sans changement. Ce que la boîte à outils
ajoute, c'est la question de politique que le magasin a laissée ouverte : que se
passe-t-il quand le travail cosmétique et le jeu lui-même veulent les mêmes
emplacements.

### Les particules sont des lignes et des emplacements

L'étincelle est de la donnée — `assets/particles.txt`, une ligne dans le format
agrandi, ses colonnes nommées par son en-tête :

```
name sprite accel range behavior
spark assets/spark.ppm 400 64 settle
```

L'art (`assets/spark.ppm`, une lueur 16×16 à couleur clé magenta) est son
`sprite` ; la pose prend `accel` millisecondes et couvre `range` pixels — les
deux colonnes que le format avait déjà, dans leurs mêmes significations.
`settle` est une nouvelle orthographe de la colonne `behavior` : ce qu'un type
fait à chaque frame, exactement comme `fly` ou `chase`. L'exécution imprime la
ligne à côté de celle de chaque autre type et rien chez elle n'est traité à
part :

```
engine: def spark: x 0 y 0 facing 0 speed 0 health 0 sprite assets/spark.ppm accel 400 damage 0 rate 0 fires none range 64 behavior settle wave 0 count 1
```

Une rafale se déclenche depuis la frame propre de l'événement, comme chaque
effet de ressenti — un coup disperse quelques étincelles depuis l'impact, une
mort en disperse davantage depuis le centre de ce qui est tombé. L'effectif
jetable du coup fatal (un type fragile et immobile, les nombres sont les siens) :

```
engine: hit: bolt hits bag — damage 1, health 1 -> 0
engine: feel: hitstop fired (0.25x, 0.15s)
engine: feel: shake fired (5 px, 0.25s)
engine: burst: spark x4 at 320,232 — 4 made, 0 dropped
engine: shot bolt retired — hit bag
engine: bag retired — zero health
engine: feel: hitstop fired (0.25x, 0.30s)
engine: feel: shake fired (10 px, 0.50s)
engine: burst: spark x8 at 344,240 — 8 made, 0 dropped
```

Une frame encore : la rafale du coup et celle de la mort se tiennent dans le
même compte rendu que la rétroaction qui les a fait atterrir.

### La politique du magasin borne la rafale — et garde les emplacements du jeu

Voici la question de politique, et sa réponse : les particules prennent les
emplacements du magasin, mais jamais tous. La **part cosmétique** de la boîte à
outils (`FEEL_COSMETIC_SLOTS`) est la portion du magasin que les particules
peuvent occuper ; le reste est gardé pour les apparitions du jeu lui-même. Une
rafale qui ne trouve aucun emplacement cosmétique **abandonne sa particule** —
le travail cosmétique peut être abandonné (une étincelle abandonnée est
invisible), et c'est compté et rapporté. Le travail de gameplay ne peut pas être
abandonné : le magasin refuse toujours bruyamment et de façon typée (le
contraste qu'a enseigné la leçon 074 — un ennemi abandonné est un bug que le
joueur subit), et un déluge d'étincelles ne peut jamais en causer un.

Une exécution de stress montre toute la politique d'un coup — un déluge de 24
étincelles par frame (le déluge d'une sonde, dit comme tel) pendant que le héros
continue de se battre :

```
engine: burst: spark x24 at 320,232 — 24 made, 0 dropped
engine: burst: spark x24 at 320,232 — 8 made, 16 dropped
engine: fire: hero -> bolt (damage 1, range 160)
engine: burst: spark x24 at 320,232 — 0 made, 24 dropped
engine: burst: spark x24 at 320,232 — 0 made, 24 dropped
engine: burst: spark x4 at 320,232 — 0 made, 4 dropped
engine: burst: spark x8 at 344,240 — 0 made, 8 dropped
engine: wave 1 cleared — the next begins
engine: wave 2: spawns bag at 336,232 — speed 0, health 1, chase
engine: wave 2 begins — 1 enemies
engine: burst: spark x24 at 320,232 — 0 made, 24 dropped
```

Les deux premiers déluges remplissent la part (24 + 8 = 32 des 64 emplacements
du magasin) et tout ce qui suit est `0 made, N dropped` — et regardez ce qui
continue d'arriver à travers les abandons : `fire: hero -> bolt` (le tir du
héros prend un emplacement) et `wave 2: spawns bag` (l'ennemi de la vague en
prend un). **Une part cosmétique pleine abandonne des particules pendant que les
apparitions du jeu sont gardées.** Pas une entité de gameplay n'a été refusée,
déplacée ou volée ; les abandons sont toutes des étincelles, et le rapport les
compte tous.

### Bouger, se poser, se retirer

La vie d'une particule est une seule valeur avec easing. À chaque frame de temps
de jeu, elle se tient un peu plus loin le long de sa voie — la distance suit la
courbe de l'easing — et quand sa vie se termine, elle est exactement à la
`range` de sa ligne, et s'y retire. *Bouger, se poser, se retirer.* L'exécution
rapporte chaque arrivée :

```
engine: spark settled at 384,232 — 64 px out, its row's range 64 (exact)
engine: spark settled at 320,296 — 64 px out, its row's range 64 (exact)
engine: spark settled at 256,232 — 64 px out, its row's range 64 (exact)
engine: spark settled at 320,168 — 64 px out, its row's range 64 (exact)
engine: spark settled at 344,304 — 64 px out, its row's range 64 (exact)
engine: spark settled at 298,285 — 64 px out, its row's range 64 (exact)
engine: spark settled at 280,240 — 64 px out, its row's range 64 (exact)
engine: spark settled at 298,194 — 64 px out, its row's range 64 (exact)
engine: spark settled at 344,176 — 64 px out, its row's range 64 (exact)
engine: spark settled at 389,194 — 64 px out, its row's range 64 (exact)
engine: spark settled at 408,240 — 64 px out, its row's range 64 (exact)
engine: spark settled at 389,285 — 64 px out, its row's range 64 (exact)
```

Douze arrivées, et chacune dit `exact`. Les quatre premières sont la rafale du
coup à `320,232` — les quatre voies cardinales, à 64 px chacune : `384`,
`320,296`, `256`, `320,168`. Les huit suivantes sont la rafale de la mort à
`344,240` — les huit voies : `408,240` plein est, `280,240` plein ouest
(`344 − 64`), `298,285` et `389,285` les diagonales à `64/√2 ≈ 45.25` px, et
ainsi de suite autour de la boussole. Les étincelles volent au-dessus de la
scène — aucune collision ne les résout ; elles sont un mouvement cosmétique à
travers le temps de jeu, et la pause les fige en plein vol comme tout le reste
de ce que fait la simulation.

Un bord honnête que le design a attrapé avant l'exécution : une étincelle se pose
en temps de **jeu**, si bien que la rafale déclenchée par le coup qui *termine*
la partie resterait figée pour toujours au-dessus de l'écran de fin. Le nettoyage
du nouveau combat balaie les débris avec les combattants et les tirs du combat
précédent — le magasin démarre propre, à chaque partie.

### L'easing : des valeurs qui arrivent

Le quatrième effet est le plus petit et celui sur lequel tout le reste
s'appuiera : **l'easing** — un petit ensemble de courbes pour les valeurs qui
s'animent de là où elles sont vers là où elles doivent être. Chacune prend `t`
dans `[0, 1]` et répond la fraction parcourue — et voici tout le contrat :

> une valeur avec easing **arrive** à sa cible — exactement 0 à 0, exactement 1
> à 1 — elle ne s'en approche pas indéfiniment.

Trois courbes sont livrées : `EaseInQuad` (départ lent, qui arrive avec du
poids), `EaseOutQuad` (départ rapide, qui se pose en place — la courbe de la
pose), et `EaseInOutQuad`. Une sonde jetable des trois aux bornes et au milieu
(dite comme telle : une sonde, pas l'exécution livrée) :

```
engine: probe: ease at 0: in 0, out 0, inout 0
engine: probe: ease at 0.5: in 0.25, out 0.75, inout 0.5
engine: probe: ease at 1: in 1, out 1, inout 1
```

`out(0.5) = 0.75` est la signature de la courbe de pose — trois quarts du chemin
à la moitié du temps — et chaque courbe lit `1` à `1`, jusqu'au dernier chiffre.
La valeur propre de la pose, sondée à pleine précision pendant la vie d'une
étincelle :

```
engine: probe: spark traveled 48.825914620711856 at t 0.513
engine: probe: spark traveled 63.302683514143858 at t 0.896
engine: probe: spark traveled 63.695704497446457 at t 0.931
engine: probe: spark traveled 64 at t 1.000
engine: spark settled at 408,240 — 64 px out, its row's range 64 (exact)
```

Soixante-trois et quelque chose à `t 0.931`, `63.99…` une frame plus tard — puis
`64` à `t 1.000`, imprimé à 17 chiffres de précision comme exactement `64` : la
`range` de la ligne, pas un de ses voisins. Les frames intermédiaires suivent
l'easing (`64 × (1 − (1 − t)²)`, l'arithmétique propre de la courbe) et la
dernière frame **est** la cible. C'est ce qui sépare l'easing de l'approche
géométrique qu'utilise l'`accel` du héros : les deux semblent fluides, mais une
seule peut être crue pour mettre une valeur là où elle doit être.

### Ce que cette exécution a vérifié, et ce qu'elle n'a pas vérifié

- **Une rafale est bornée par la politique du magasin et ses particules se
  retirent** — chaque rafale rapporte `N made, M dropped` contre la part
  cosmétique (`24 + 8` l'ont remplie dans l'exécution de stress), et chaque
  particule se termine par sa propre ligne `settled` — bouger, se poser, se
  retirer.
- **Un magasin plein abandonne des particules pendant que les apparitions de
  gameplay sont gardées** — à travers un déluge de `0 made, 24 dropped`, le
  `fire` du héros et le `spawns bag` de la vague continuent de prendre leurs
  emplacements ; les abandons ne sont que des étincelles, comptées, et rien de
  déjà apparu n'est touché.
- **Les valeurs avec easing arrivent exactement à leurs cibles** — l'ensemble
  d'easing lit `1` à `1` ; la valeur de la pose est exactement `64` — la `range`
  de sa ligne — à `t 1.000`, à 17 chiffres, et l'exécution imprime `exact`
  depuis la comparaison propre de l'arrivée.

Ce que cette exécution n'a **pas** vérifié, c'est à quoi tout cela *ressemble* —
la dispersion de la rafale et la lueur de l'étincelle sont dessinées vers un
écran que cette machine n'a pas. Les nombres disent que les étincelles vont où
les voies le disent et se posent où la ligne le dit ; que cela se lise comme un
impact relève encore du jugement du joueur. Et la taille de la part cosmétique —
32 emplacements — est un choix de politique, pas une mesure : la moitié du
magasin semblait un budget sain, et le jour où le vrai jeu montrera de la
pression dessus sera le jour où la mesure (la passe de mesure) la déplacera.

## Étape de code

Un changement : les deux derniers effets de la boîte à outils. `src/feel.h/.cpp`
font grandir l'ensemble d'easing (`EaseInQuad`/`EaseOutQuad`/`EaseInOutQuad`),
la rafale (`FeelBurst` — la part cosmétique du magasin, les huit voies, la
politique d'abandon-et-comptage), et la pose de la particule (`FeelParticle` —
le trajet avec easing qui arrive exactement et se retire là).
`assets/particles.txt` et `assets/spark.ppm` sont le type étincelle — donnée et
art, comme chaque type. `src/table.h/.cpp` font grandir le vocabulaire de
`behavior` par `settle` ; `src/entity.h` donne à une particule son horloge et
son point de rafale (sa position est mesurée depuis là à chaque frame, jamais
accumulée — l'arrivée doit être la cible, pas un arrondi d'elle) ;
`src/combat.cpp` déclenche les deux rafales au coup et à la mort et laisse les
tirs traverser les étincelles ; la marche de `src/game.cpp` pose les particules
et son nettoyage de nouveau combat balaie les débris. Son état final est étiqueté
`lesson-093`.

```diff
diff --git a/assets/particles.txt b/assets/particles.txt
new file mode 100644
index 0000000..df42f9f
--- /dev/null
+++ b/assets/particles.txt
@@ -0,0 +1,2 @@
+name sprite accel range behavior
+spark assets/spark.ppm 400 64 settle
diff --git a/src/combat.cpp b/src/combat.cpp
index a10ffb7..e040bb5 100644
--- a/src/combat.cpp
+++ b/src/combat.cpp
@@ -8,6 +8,9 @@
 //
 // Lesson 092: the hit and the death are the juice toolkit's events, and
 // the feedback hooks fire at the lines where they happen.
+//
+// Lesson 093: the same events burst particles — cosmetic entities of
+// the kind the game names, bounded by the store's policy.
 
 #include "combat.h"
 
@@ -116,7 +119,8 @@ bool CombatFire(EntityStore &store, const EntityTable &shots,
 }
 
 void CombatFly(const TileMap &map, EntityStore &store, const Entity &hero,
-               Entity &shot, Feedback &feel, double dt)
+               Entity &shot, Feedback &feel, const EntityDef &spark,
+               double dt)
 {
     /* The flight, in game time: the shot's speed over dt, sub-stepped
        through the mover so each sub-step is small. What is checked at
@@ -137,6 +141,9 @@ void CombatFly(const TileMap &map, EntityStore &store, const Entity &hero,
                 continue;
             if (e.behavior == BEHAVIOR_FLY)
                 continue;
+            if (e.behavior == BEHAVIOR_SETTLE)
+                continue; /* a spark is cosmetic — a shot flies through
+                            it, the way it flies through other shots */
             if (shot.owner && SameName(e.name, shot.owner->name))
                 continue;
             if (Overlaps(shot, e))
@@ -161,6 +168,10 @@ void CombatFly(const TileMap &map, EntityStore &store, const Entity &hero,
             FeelHitstop(feel, 0.25, 0.15);
             FeelShake(feel, 5.0, 0.25);
 
+            /* Lesson 093: and the impact scatters sparks — a burst of
+               particles from the same event's own frame. */
+            FeelBurst(store, spark, shot.x, shot.y, 4);
+
             std::printf("engine: shot %s retired — hit %s\n", shot.name,
                         target->name);
             EntityRetire(store, shot);
@@ -180,6 +191,12 @@ void CombatFly(const TileMap &map, EntityStore &store, const Entity &hero,
                    as both, and the death's weights win. */
                 FeelHitstop(feel, 0.25, 0.30);
                 FeelShake(feel, 10.0, 0.50);
+
+                /* Lesson 093: and the death bursts harder — the thing
+                   that fell scatters its sparks from its own centre. */
+                FeelBurst(store, spark,
+                          target->x + ANIM_FRAME_W / 2.0,
+                          target->y + target->sprite->height / 2.0, 8);
             }
             return;
         }
diff --git a/src/combat.h b/src/combat.h
index fddb88a..abb52c9 100644
--- a/src/combat.h
+++ b/src/combat.h
@@ -59,9 +59,12 @@ bool CombatFire(EntityStore &store, const EntityTable &shots,
    defeat condition (the state machine reads it, the game's actor is not
    retired out from under the game). Lesson 092: the hit and the death
    are the toolkit's events — the feedback hooks fire here, in the
-   event's own frame, through `feel`. */
+   event's own frame, through `feel`. Lesson 093: they burst particles
+   of `spark` too — the cosmetic kind the game names, spawned in the
+   same frame from the same event. */
 void CombatFly(const TileMap &map, EntityStore &store, const Entity &hero,
-               Entity &shot, Feedback &feel, double dt);
+               Entity &shot, Feedback &feel, const EntityDef &spark,
+               double dt);
 
 /* Lesson 090: the enemy attack, once per frame of game time. An armed
    entity — one whose row names a projectile kind — fires it at the
diff --git a/src/entity.h b/src/entity.h
index 2797cc7..85d0d37 100644
--- a/src/entity.h
+++ b/src/entity.h
@@ -62,6 +62,11 @@ struct Entity {
                                   thing a row cannot carry */
     double phase_t;            /* and how long this phase has run */
     double cooldown;           /* seconds until it may fire again */
+    double life_t;             /* lesson 093: how long a particle has
+                                  run — its eased settle's clock */
+    double from_x, from_y;     /* and the burst point it eases out
+                                  from — its position is measured from
+                                  here, never accumulated */
 };
 
 /* An entity created from a definition: every attribute its row states,
diff --git a/src/feel.cpp b/src/feel.cpp
index 76eb89a..5d769d8 100644
--- a/src/feel.cpp
+++ b/src/feel.cpp
@@ -1,10 +1,16 @@
-// feel.cpp — the feedback hooks: fire, decay, rest.
+// feel.cpp — the juice toolkit: the feedback hooks, the bursts, and the
+// easing that shapes them.
 //
 // Lesson 086: each hook fires, runs down its own wall-time, and returns
 // exactly to rest. Lesson 092: the game's own events fire them — a hit
 // lands, a death falls — in the event's own frame, and each hook says
 // when it fires beside the event's own line. The weights are the
 // event's, passed in from where the event happens.
+//
+// Lesson 093: the toolkit's other two effects live here too — particle
+// bursts (cosmetic entities, bounded by the store's policy) and the
+// small set of ease functions that make animated values arrive at their
+// targets instead of stepping to them.
 
 #include "feel.h"
 
@@ -43,6 +49,121 @@ double FeelTimeScale(const Feedback &feel)
     return feel.hitstop > 0.0 ? feel.hitstop_k : GAMETIME_FULL;
 }
 
+/* Lesson 093: easing. Each shape takes t in [0, 1] and answers the
+   fraction travelled — exactly 0 at 0, exactly 1 at 1. The clamps at
+   both ends are the "arrives exactly" contract: past its target the
+   value sits at its target, and an eased value never overshoots. */
+double EaseInQuad(double t)
+{
+    if (t <= 0.0)
+        return 0.0;
+    if (t >= 1.0)
+        return 1.0;
+    return t * t;
+}
+
+double EaseOutQuad(double t)
+{
+    if (t <= 0.0)
+        return 0.0;
+    if (t >= 1.0)
+        return 1.0;
+    double u = 1.0 - t;
+    return 1.0 - u * u;
+}
+
+double EaseInOutQuad(double t)
+{
+    if (t <= 0.0)
+        return 0.0;
+    if (t >= 1.0)
+        return 1.0;
+    return t < 0.5 ? 2.0 * t * t : 1.0 - 2.0 * (1.0 - t) * (1.0 - t);
+}
+
+/* Lesson 093: the burst's eight lanes — the world's compass points, a
+   diagonal at 1/sqrt(2), the same lanes the aim and the movement use.
+   A burst is reproducible: lane i of a count is the same direction on
+   every machine and every run. */
+constexpr double BURST_DIAG = 0.70710678;
+const double LANE_X[8] = { 1.0, BURST_DIAG, 0.0, -BURST_DIAG, -1.0,
+                           -BURST_DIAG, 0.0, BURST_DIAG };
+const double LANE_Y[8] = { 0.0, BURST_DIAG, 1.0, BURST_DIAG, 0.0,
+                           -BURST_DIAG, -1.0, -BURST_DIAG };
+
+int FeelBurst(EntityStore &store, const EntityDef &kind, double x, double y,
+              int count)
+{
+    /* The cosmetic share, counted before the burst asks: the toolkit's
+       particles may hold FEEL_COSMETIC_SLOTS of the store's slots and
+       no more. */
+    int cosmetic = 0;
+    for (int i = 0; i < ENTITY_CAP; ++i)
+        if (store.slots[i].live && store.slots[i].behavior == BEHAVIOR_SETTLE)
+            cosmetic += 1;
+
+    int made = 0, dropped = 0;
+    for (int i = 0; i < count; ++i) {
+        /* A burst that finds no slot drops its particle — cosmetic work
+           may be dropped (counted, and invisible); gameplay work may
+           not (lesson 074's contrast). Nothing is ever stolen. */
+        if (cosmetic >= FEEL_COSMETIC_SLOTS) {
+            dropped += 1;
+            continue;
+        }
+        EntityResult result = EntityCreate(store, kind);
+        if (result.error != ENTITY_OK) {
+            dropped += 1;
+            continue;
+        }
+        Entity &e = *result.entity;
+        int lane = count < 8 ? (i * 8) / count : i % 8;
+        e.x = x;
+        e.y = y;
+        e.from_x = x;
+        e.from_y = y;
+        e.move_x = LANE_X[lane];
+        e.move_y = LANE_Y[lane];
+        e.traveled = 0.0;
+        e.life_t = 0.0;
+        cosmetic += 1;
+        made += 1;
+    }
+    std::printf("engine: burst: %s x%d at %d,%d — %d made, %d dropped\n",
+                kind.name, count, (int)x, (int)y, made, dropped);
+    return made;
+}
+
+void FeelParticle(EntityStore &store, Entity &e, double dt)
+{
+    /* The settle's clock runs on game time: a pause freezes a spark
+       mid-air, a hitstop slows it — the world's clock, like everything
+       the simulation does. */
+    e.life_t += dt;
+    double t = e.accel > 0 ? e.life_t / (e.accel / 1000.0) : 1.0;
+    if (t > 1.0)
+        t = 1.0;
+
+    /* The eased value is the distance out: it follows the ease's curve
+       frame by frame and arrives exactly at the row's range. The
+       position is that distance along the spark's lane — measured from
+       the burst point every frame, never accumulated step by step, so
+       the arrival is the target and not a rounding of it. */
+    e.traveled = (double)e.range * EaseOutQuad(t);
+    e.x = e.from_x + e.move_x * e.traveled;
+    e.y = e.from_y + e.move_y * e.traveled;
+
+    if (t >= 1.0) {
+        /* The life ends where it settles — and the arrival is exact:
+           the eased value is the target, not a neighbour of it. */
+        bool exact = e.traveled == (double)e.range;
+        std::printf("engine: %s settled at %d,%d — %g px out, its row's range %d (%s)\n",
+                    e.name, (int)e.x, (int)e.y, e.traveled, e.range,
+                    exact ? "exact" : "drifted");
+        EntityRetire(store, e);
+    }
+}
+
 void FeelUpdate(Feedback &feel, double wall_dt, Camera &camera)
 {
     /* The hitstop runs on its own wall-time and returns to full speed
diff --git a/src/feel.h b/src/feel.h
index 5827080..d4f1909 100644
--- a/src/feel.h
+++ b/src/feel.h
@@ -1,4 +1,4 @@
-// feel.h — the feedback hooks the juice toolkit drives.
+// feel.h — the juice toolkit the game's events drive.
 //
 // Lesson 086: two hooks — a screenshake and a hitstop — each a thing
 // that fires and then rests. Lesson 092: the toolkit fires them from the
@@ -7,14 +7,28 @@
 // are the mechanisms, each with its own fire-and-rest life; the events
 // decide when and how heavily. A hook at rest costs nothing and changes
 // nothing.
+//
+// Lesson 093: the toolkit's other two effects — particle bursts (the
+// store's cosmetic work) and easing (values that arrive at their
+// targets). Four effects, and no fifth: hitstop, screenshake, particle
+// bursts, easing.
 #ifndef FEEL_H
 #define FEEL_H
 
 #include "camera.h"
+#include "entity.h"
 #include "gametime.h"
 
 namespace engine {
 
+/* Lesson 093: the store's cosmetic share — the slots the toolkit's
+   particles may hold. A burst takes these slots and no others: the rest
+   of the store is kept for the game's own spawns, so a flood of sparks
+   drops cosmetic work and never a gameplay one. A dropped particle is
+   invisible; a dropped enemy is a bug the player experiences (lesson
+   074's contrast). */
+constexpr int FEEL_COSMETIC_SLOTS = ENTITY_CAP / 2;
+
 /* The feedback state: what is still firing. Every field is at rest at
    zero — a hook that has fired and finished leaves itself exactly here. */
 struct Feedback {
@@ -45,6 +59,30 @@ double FeelTimeScale(const Feedback &feel);
    offset (resting at exactly zero). */
 void FeelUpdate(Feedback &feel, double wall_dt, Camera &camera);
 
+/* Lesson 093: easing — a small set of shapes for values that animate
+   from where they are to where they belong. Each takes t in [0, 1] and
+   answers the fraction travelled: exactly 0 at 0, exactly 1 at 1. The
+   ends are clamped on purpose — an eased value arrives at its target,
+   it does not approach it forever. */
+double EaseInQuad(double t);     /* slow out, arriving with weight */
+double EaseOutQuad(double t);    /* fast out, settling into place */
+double EaseInOutQuad(double t);  /* both */
+
+/* Lesson 093: a particle burst — the toolkit's cosmetic spawn, fired in
+   the triggering event's own frame like every feel effect. Up to
+   `count` particles of `kind` take the store's cosmetic slots and ease
+   out from (x, y) along the burst's lanes; whatever finds no slot is
+   dropped and counted — cosmetic work may be dropped, gameplay work may
+   not. Answers how many were made. */
+int FeelBurst(EntityStore &store, const EntityDef &kind, double x, double y,
+              int count);
+
+/* One particle's settle, once a frame of game time: its eased travel
+   out from its burst point — following the ease's curve and arriving
+   exactly at its row's range — and its retirement there, when its life
+   ends. */
+void FeelParticle(EntityStore &store, Entity &e, double dt);
+
 } /* namespace engine */
 
 #endif
diff --git a/src/game.cpp b/src/game.cpp
index 3da68a4..d52e01d 100644
--- a/src/game.cpp
+++ b/src/game.cpp
@@ -218,7 +218,8 @@ void GameDrawSprites(const Game &game, Framebuffer &fb,
 }
 
 int GameWalk(EntityStore &store, const TileMap &map, const Entity &hero,
-             const EntityTable &shots, Feedback &feel, double dt)
+             const EntityTable &shots, Feedback &feel, const EntityDef &spark,
+             double dt)
 {
     /* Lesson 084: the walk — every live entity, once per frame, in slot
        order, its movement resolved against the tilemap. The per-entity
@@ -250,8 +251,12 @@ int GameWalk(EntityStore &store, const TileMap &map, const Entity &hero,
         Entity &e = store.slots[i];
         switch (e.behavior) {
         case BEHAVIOR_FLY:
-            CombatFly(map, store, hero, e, feel, dt);
+            CombatFly(map, store, hero, e, feel, spark, dt);
             continue; /* the flight moves itself, through the mover */
+        case BEHAVIOR_SETTLE:
+            FeelParticle(store, e, dt);
+            continue; /* the settle moves itself — eased travel, no
+                        collision: sparks fly over the scene */
         case BEHAVIOR_CHASE:
             AiChase(e, hero);
             break;
@@ -300,16 +305,19 @@ static bool IsFighter(const Entity &e)
 
 void GameWaves(Game &game, EntityStore &store, const EntityTable &foes)
 {
-    /* A fresh fight: the last game's fighters and shots leave the store
-       — the hero is the game's actor and the `none` kinds are the
-       world's scenery, and both stay. */
+    /* A fresh fight: the last game's fighters, shots, and debris leave
+       the store — the hero is the game's actor and the `none` kinds are
+       the world's scenery, and both stay. The debris goes too (lesson
+       093): a spark settles in game time, and a frozen one — from the
+       blow that ended the last game — would hang there forever. */
     if (game.wave == 0) {
         int cleared = 0;
         for (int i = 0; i < ENTITY_CAP; ++i) {
             Entity &e = store.slots[i];
             if (!e.live)
                 continue;
-            if (IsFighter(e) || e.behavior == BEHAVIOR_FLY) {
+            if (IsFighter(e) || e.behavior == BEHAVIOR_FLY ||
+                e.behavior == BEHAVIOR_SETTLE) {
                 EntityRetire(store, e);
                 cleared += 1;
             }
diff --git a/src/game.h b/src/game.h
index 30dcc3e..92f49dc 100644
--- a/src/game.h
+++ b/src/game.h
@@ -114,10 +114,13 @@ void GameDrawSprites(const Game &game, Framebuffer &fb,
    the way the player's input writes the hero's, and an armed entity
    attacks at its row's rate. Lesson 092: the combat's events (a hit, a
    death) fire the feedback hooks through `feel`, in their own frame.
-   The hero is handed along for the combat's rules to know the game's
-   actor by. Returns the visit count. */
+   Lesson 093: the toolkit's particles settle here (FeelParticle) and
+   burst of `spark` — the cosmetic kind the game names. The hero is
+   handed along for the combat's rules to know the game's actor by.
+   Returns the visit count. */
 int GameWalk(EntityStore &store, const TileMap &map, const Entity &hero,
-             const EntityTable &shots, Feedback &feel, double dt);
+             const EntityTable &shots, Feedback &feel, const EntityDef &spark,
+             double dt);
 
 /* Lesson 091: the waves, once per frame of play. A fresh fight clears
    the last one from the store; a wave spawns its composition from the
diff --git a/src/main.cpp b/src/main.cpp
index a115e30..472c017 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -245,11 +245,14 @@ int Run(void)
        entity of. Each file's header names the columns it uses — and only
        those; what it leaves unnamed sits at the format's defaults.
        Lesson 088: and the enemy roster — the three types and the boss,
-       every per-type fact its own row's value. */
-    EntityTable weapons, shots, foes;
+       every per-type fact its own row's value. Lesson 093: and the
+       toolkit's particle kinds — cosmetic entities from rows like every
+       other kind, the burst's art and settle in the table's columns. */
+    EntityTable weapons, shots, foes, particles;
     if (!LoadRunTable(arena, "assets/weapons.txt", weapons) ||
         !LoadRunTable(arena, "assets/projectiles.txt", shots) ||
-        !LoadRunTable(arena, "assets/enemies.txt", foes)) {
+        !LoadRunTable(arena, "assets/enemies.txt", foes) ||
+        !LoadRunTable(arena, "assets/particles.txt", particles)) {
         platform::CloseWindow(opened.window);
         ArenaRelease(arena);
         return 1;
@@ -265,11 +268,12 @@ int Run(void)
     PrintDefs("assets/weapons.txt", weapons);
     PrintDefs("assets/projectiles.txt", shots);
     PrintDefs("assets/enemies.txt", foes);
+    PrintDefs("assets/particles.txt", particles);
 
     /* Lesson 073: the definitions' art, loaded at startup. A row that
        names no sprite (a weapon row) has no art and needs none. */
     if (!LoadRunArt(arena, table) || !LoadRunArt(arena, shots) ||
-        !LoadRunArt(arena, foes)) {
+        !LoadRunArt(arena, foes) || !LoadRunArt(arena, particles)) {
         platform::CloseWindow(opened.window);
         ArenaRelease(arena);
         return 1;
@@ -355,6 +359,19 @@ int Run(void)
     if (weapons.count > 0)
         CombatArm(hero, weapons.rows[0]);
 
+    /* Lesson 093: the burst kind — the particles table's first row. The
+       game bursts what the table puts first, the way the hero arms with
+       the weapons table's first row; a table with no particle kind is a
+       named failure, never a burst of assumed attributes. */
+    if (particles.count == 0) {
+        std::fprintf(stderr,
+                     "engine: assets/particles.txt: no particle kind\n");
+        platform::CloseWindow(opened.window);
+        ArenaRelease(arena);
+        return 1;
+    }
+    const EntityDef &spark = particles.rows[0];
+
     /* The lookup's typed failure, checked on purpose: a definition the
        table does not hold is a value — never an entity with assumed
        attributes. */
@@ -533,7 +550,7 @@ int Run(void)
             GameWaves(game, store, foes);
 
         double t_entities = platform::Now();
-        int visited = GameWalk(store, map, hero, shots, feel, dt);
+        int visited = GameWalk(store, map, hero, shots, feel, spark, dt);
         frame.entities = platform::Now() - t_entities;
         walk_visits += visited;
 
diff --git a/src/table.cpp b/src/table.cpp
index fd179ab..0b07d3e 100644
--- a/src/table.cpp
+++ b/src/table.cpp
@@ -133,7 +133,7 @@ const char *const COLUMN_NAMES[COL_COUNT] = {
 
 /* Lesson 087: the behavior column's spellings, the format's own. */
 const char *const BEHAVIOR_NAMES[BEHAVIOR_COUNT] = {
-    "none", "fly", "chase", "keep", "flee", "boss"
+    "none", "fly", "chase", "keep", "flee", "boss", "settle"
 };
 
 /* Lesson 087: one row at the format's defaults, before the named fields
diff --git a/src/table.h b/src/table.h
index a79fe0c..9fb504e 100644
--- a/src/table.h
+++ b/src/table.h
@@ -46,7 +46,9 @@ constexpr int TABLE_PATH_MAX = 64;
    frame, the fact its row carries. The format defines the spellings,
    like it defines facing's four numbers: `none` stands still, `fly` is a
    projectile in flight, and chase / keep / flee / boss are the enemy
-   behaviors (lessons 089-090) — the boss's value names its pattern. */
+   behaviors (lessons 089-090) — the boss's value names its pattern.
+   Lesson 093: `settle` is a burst's particle — eased travel out from
+   its burst point, retiring where it settles. */
 enum BehaviorKind {
     BEHAVIOR_NONE = 0,
     BEHAVIOR_FLY,
@@ -54,6 +56,7 @@ enum BehaviorKind {
     BEHAVIOR_KEEP,
     BEHAVIOR_FLEE,
     BEHAVIOR_BOSS,
+    BEHAVIOR_SETTLE,
     BEHAVIOR_COUNT
 };
 
@@ -78,13 +81,16 @@ struct EntityDef {
        its default when a file's header does not name it — the defaults
        are the format's contract, not a gap in it. */
     int accel;                   /* ms: the eased-move time constant —
-                                    the feel (the hero's weight) */
+                                    the feel (the hero's weight; a
+                                    particle's settle takes this long,
+                                    lesson 093) */
     int damage;                  /* points a hit removes — a weapon
                                     row's damage, carried by its shots */
     int rate;                    /* rounds per minute; 0 = never fires */
     char fires[TABLE_NAME_MAX];  /* the projectile kind this row fires */
     int range;                   /* world pixels: a projectile's flight
-                                    budget — its life */
+                                    budget — its life (a particle's
+                                    settle distance, lesson 093) */
     int behavior;                /* BehaviorKind, its row's */
     int wave;                    /* the wave this kind joins — it spawns
                                     in that wave and every wave after
```

## Exercices

Deux défis plus conséquents. Chacun se termine par sa solution — un diff contre
l'état final de cette leçon, plus une visite guidée — après l'énoncé.

### Exercice 1 — Les étincelles héritent du coup *(extend-the-code)*

Les étincelles d'un coup et celles d'une mort se ressemblent aujourd'hui : les
deux rafales arrosent chaque voie de la même façon. Rendez la rafale
**directionnelle** : les étincelles d'un coup devraient arroser dans la direction
où volait le tir — le coup porte son élan dans la dispersion — tandis qu'une
mort garde sa rafale dans toutes les directions. Lancez ensuite un coup fatal où
le tir vole vers l'est et citez où les quatre étincelles du coup se posent à
côté des huit de la mort.

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-093/ex1.md)

### Exercice 2 — La courbe, prédite *(predict-the-output)*

Lisez `EaseOutQuad` — la courbe le long de laquelle voyage la pose — et prédisez
l'exécution avant de la faire : pour la ligne d'étincelle livrée (`accel 400`,
`range 64`), à quelle distance se trouve une étincelle à la moitié de sa pose, et
où se termine-t-elle ? Lancez-la ensuite avec une sonde qui imprime la valeur
avec easing à chaque frame à pleine précision, et comparez — y compris la
question sur laquelle tourne tout l'effet : la dernière frame atterrit-elle
**sur** la cible, ou seulement près d'elle ?

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-093/ex2.md)

---

**Partie :** [Partie 5 — le jeu](../../index.md) ·
**Précédente :** [Leçon 092 — hitstop et screenshake](lesson-092-hitstop-shake.md) ·
**Suivante :** [Leçon 094 — le HUD](lesson-094-hud.md) ·
**Étiquette de code :** [`lesson-093`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-093)

*Page traduite de la version anglaise `book/lessons/part-5/lesson-093-bursts-easing.md`,
révision `3c9d3c2`.*

<!-- translation-source: book/lessons/part-5/lesson-093-bursts-easing.md @ 3c9d3c2 -->
