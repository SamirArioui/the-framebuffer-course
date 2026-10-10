# Leçon 075 — la marche et l'emplacement libre

{{#include ../../stability-horizon.md}}

## Prose

La leçon 074 s'est terminée sur un magasin (store) plein d'entités et rien qui
les regarde : la création, le refus et un compte. Un magasin que personne ne
parcourt est une liste, pas un jeu — le travail par entité d'une frame (se
déplacer, penser, dessiner, mourir) a besoin d'un moyen d'atteindre chaque
entité vivante exactement une fois. Et une fois que les entités peuvent finir,
deux questions suivent, auxquelles le magasin doit répondre sans ambiguïté :
quand un emplacement redevient-il libre, et quel emplacement la prochaine entité
prend-elle. L'idée de cette leçon est donc l'autre moitié du stockage : **la
marche visite chaque entité vivante exactement une fois par frame, le retrait
libère un emplacement, et l'emplacement libéré est réutilisé avant tout
emplacement jamais utilisé.**

### La marche est une boucle sur des emplacements fixes

La marche a une seule forme, et c'est toute l'API :

```cpp
for (int i = 0; i < ENTITY_CAP; ++i) {
    if (!store.slots[i].live)
        continue;
    ... one entity's work ...
}
```

Chaque entité vivante exactement une fois, dans l'ordre des emplacements ; aucun
emplacement retiré ou vide visité. Le travail est exprimé *ici*, une fois — pas
une fois par type d'entité. Que l'entité de l'emplacement 12 soit le héros ou un
projectile, le travail qui s'applique à toutes est ce seul corps.

La moitié intéressante du contrat, c'est ce qui arrive quand ce corps retire
quelque chose. **Les emplacements ne bougent jamais.** Contrairement à une liste
qui décale ses éléments quand on en retire un, les emplacements du magasin sont
fixes — donc une marche qui retire l'entité qu'elle regarde (ou une plus loin,
ou une derrière) ne peut perdre ni répéter personne en le faisant. Qui n'est pas
vivante quand la marche arrive n'est pas visitée ; qui l'est l'est. La première
marche de l'exécution fait exactement cela : elle retire trois entités au
passage et visite les huit.

Une sonde jetable (pas du code du moteur) a vérifié l'ordre de visite jusqu'à
l'emplacement, avec des retraits à trois positions :

```
walk visited 8: 0 1 2 3 4 5 6 7  (retired at -1 -> slot -1), live 5
walk visited 5: 0 1 2 4 5  (retired at 1 -> slot 3), live 5
walk visited 6: 0 1 2 3 4 5  (retired at 2 -> slot 0), live 5
```

La première ligne est la marche de la démo : huit entités vivantes aux
emplacements 0-7, trois d'entre elles tuées d'avance, les huit visitées une fois
chacune — les trois retirées au passage *après* avoir été visitées, parce que le
retrait est ce que le travail leur a fait. La deuxième ligne retire l'emplacement
3 alors que la marche se tient à l'emplacement 1 — devant elle — et
l'emplacement 3 n'est simplement plus là quand la marche arrive : cinq visités,
personne d'autre sauté. La troisième ligne retire l'emplacement 0 alors qu'elle
est à l'emplacement 2 — derrière, déjà visité — et les six sont visités une fois
chacun : la retirée a été vue avant de partir, et n'est plus revue.

### Le retrait, et ce que le compte signifie

```cpp
void EntityRetire(EntityStore &store, Entity &entity);
```

Une entité partie : le drapeau `live` de son emplacement retombe et le compte
baisse. C'est tout le modèle de durée de vie — pas de destruction, pas de copie,
pas de compactage. L'emplacement garde les octets de l'entité qui était ; ce
n'est pas l'affaire du magasin de les nettoyer, et la prochaine création les
écrase.

Le compte, c'est `store.live`, et il n'est délibérément *pas* le nombre
d'emplacements jamais remplis. Le jeu lit « combien de choses sont vivantes
maintenant » — pour le HUD, pour la logique des vagues, pour le budget de frames
— et un compte qui ne ferait que croître répondrait à une question que personne
n'a posée. Le rapport de l'exécution montre le compte baisser et remonter en
l'espace d'une seconde :

```
engine: walk (frame 1): visited 8 live entities, once each, in slot order; live 5 of 64
engine: store: live 8 of 64
```

### L'emplacement libéré revient en premier

La création prend le premier emplacement libre — l'indice le plus bas qui ne
contient rien (la règle de la leçon 074). Les emplacements étant fixes, cette
règle répond gratuitement à la question de la réutilisation : l'indice d'un
emplacement libéré est plus bas que celui de tout emplacement jamais utilisé,
donc **l'emplacement libéré est pris avant tout emplacement jamais utilisé**. Il
n'y a pas de liste libre à maintenir ni d'ordre à se tromper ; le parcours est
la politique.

La démo le rend visible. Le magasin contient huit entités aux emplacements 0-7 ;
la marche retire 2, 4 et 6 ; les trois requêtes suivantes atterrissent dans les
emplacements qu'elles ont quittés :

```
engine: store: created in slot 2 (freed before never-used)
engine: store: created in slot 4 (freed before never-used)
engine: store: created in slot 6 (freed before never-used)
engine: store: live 8 of 64
```

Les emplacements 2, 4, 6 — les libérés — tandis que les emplacements 8-63 n'ont
jamais été utilisés et restent inutilisés. Si le magasin avait pris un
emplacement jamais utilisé à la place, le rapport lirait `created in slot 8`, et
la mémoire du jeu grimperait pendant que des emplacements parfaitement bons
resteraient vides. (Une note honnête : une fois que le magasin a été rempli à
capacité, tous les emplacements ont servi et la distinction disparaît — c'est
pourquoi cette démo garde son monde à huit entités plutôt que le magasin plein
de la leçon 074.)

### Ce que cette exécution a vérifié, et ce qu'elle n'a pas vérifié

D'une vraie exécution de l'état final de cette leçon sur cette machine :

- **Chaque entité vivante est parcourue exactement une fois par frame.** Le
  rapport de la première marche : `visited 8 live entities, once each, in slot
  order` — et la sonde ci-dessus nomme les emplacements. Le bilan de l'exécution
  ferme l'arithmétique : `engine: walk: 29 visits over 4 frames — one per live
  entity per frame`, soit 8 + 5 + 8 + 8 — le compte des vivantes de chaque
  frame, sommé, et rien d'autre. La marche est le travail par entité de
  l'update ; chaque frame l'exécute une fois.
- **Une entité retirée pendant la marche se comporte bien.** La marche a retiré
  les emplacements 2, 4 et 6 au passage et a quand même visité les huit ; les
  marches « retrait devant » et « retrait derrière » de la sonde confirment les
  deux directions.
- **Le retrait libère un emplacement et le compte baisse.** `live 8` → `live 5`
  sur une marche.
- **L'emplacement libéré est réutilisé avant tout emplacement jamais utilisé.**
  Les trois créations ont atterri dans les emplacements 2, 4 et 6, pas dans 8,
  9, 10.

Ce que cette leçon ne fait **pas**, c'est donner à la marche du vrai travail. Le
corps par entité est la vérification de mise à mort de la démo — le corps d'un
jeu sera le mouvement, le comportement, le dessin. Cela arrive à la leçon 076,
où le héros devient la première entité à qui une frame fait réellement *quelque
chose*.

## Étape de code

Un seul changement pour cette leçon, du stockage à la durée de vie :
`src/entity.h` / `src/entity.cpp` accueillent `EntityRetire` et le contrat de la
marche — la forme que prend le travail par entité du jeu, et pourquoi retirer
pendant celle-ci est sûr. `src/main.cpp` accueille la frame de l'exécution : la
marche dans la phase d'update, chaque entité vivante visitée une fois et les
entités tuées de la démo retirées au passage, et le script de réutilisation à la
frame suivante — trois requêtes atterrissant dans les emplacements qui viennent
d'être libérés. Le monde de la démo rétrécit du magasin plein de la leçon 074 à
huit entités, pour que le rapport puisse distinguer un emplacement libéré d'un
emplacement jamais utilisé. Le sprite de la démo, la carte et la boucle sont
intacts. Son état final est étiqueté `lesson-075`.

```diff
diff --git a/src/entity.cpp b/src/entity.cpp
index 36bb098..04ff1f3 100644
--- a/src/entity.cpp
+++ b/src/entity.cpp
@@ -48,4 +48,12 @@ EntityResult EntityCreate(EntityStore &store, const EntityDef &def)
     return result;
 }
 
+void EntityRetire(EntityStore &store, Entity &entity)
+{
+    if (!entity.live)
+        return; /* retiring nothing is nothing */
+    entity.live = false;
+    store.live -= 1;
+}
+
 } /* namespace engine */
diff --git a/src/entity.h b/src/entity.h
index d31886b..785f868 100644
--- a/src/entity.h
+++ b/src/entity.h
@@ -65,6 +65,26 @@ struct EntityResult {
    inaudible; a stolen enemy is a bug the player experiences. */
 EntityResult EntityCreate(EntityStore &store, const EntityDef &def);
 
+/* Lesson 075: retirement — the entity is gone and its slot is free
+   again. The live count falls; the slot is reusable, and creation's
+   first-free-slot rule hands it back before any slot that has never
+   been used. Retiring an entity that is already gone is nothing. */
+void EntityRetire(EntityStore &store, Entity &entity);
+
+/* Lesson 075: the walk — the shape the game's per-entity work takes:
+
+     for (int i = 0; i < ENTITY_CAP; ++i) {
+         if (!store.slots[i].live)
+             continue;
+         ... one entity's work ...
+     }
+
+   Every live entity exactly once, in slot order; no retired or empty
+   slot visited. The slots never move, so the work may retire the entity
+   it is looking at — or one further along — without the walk repeating
+   or skipping anyone: whoever is not live when the walk arrives is not
+   visited, and everyone who is, is. */
+
 } /* namespace engine */
 
 #endif
diff --git a/src/main.cpp b/src/main.cpp
index d84675f..51ae3de 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -252,22 +252,24 @@ int Run(void)
                 hero.name, hero.x, hero.y, hero.facing, hero.speed, hero.health,
                 hero.sprite->width, hero.sprite->height);
 
-    /* Lesson 074: the creation script — request after request, each an
-       entity from the table's rows in turn, until the store answers with
-       its typed failure. This is the policy under load: the first free
-       slot, and never a live entity's. */
-    size_t before_script = arena.used;
+    /* Lesson 074: the creation policy — the first free slot, and never
+       a live entity's. Lesson 075: the demo keeps a small world — eight
+       entities, three of them killed before the first walk — so the
+       store has free slots and the report can tell a freed slot from one
+       that has never been used. */
     int created = 0;
-    for (;;) {
-        EntityResult made = EntityCreate(store, table.rows[created % table.count]);
+    while (store.live < 8) {
+        EntityResult made =
+            EntityCreate(store, table.rows[store.live % table.count]);
         if (made.error != ENTITY_OK)
             break;
         created += 1;
     }
-    std::printf("engine: store: live %d of %d — the hero and %d from the script\n",
+    store.slots[2].health = 0;
+    store.slots[4].health = 0;
+    store.slots[6].health = 0;
+    std::printf("engine: store: live %d of %d — the hero and %d from the script; slots 2, 4, 6 killed\n",
                 store.live, ENTITY_CAP, created);
-    std::printf("engine: store: creation refused (full), arena %zu -> %zu — creation allocates nothing\n",
-                before_script, arena.used);
 
     /* The lookup's typed failure, checked on purpose: a definition the
        table does not hold is a value — never an entity with assumed
@@ -379,6 +381,7 @@ int Run(void)
        account of what it carried is the frame record's audio phase now,
        measured like every other phase of the frame. */
     int feeds = 0;         /* buffers handed to the device */
+    long walk_visits = 0;  /* lesson 075: entities visited by the walk */
     while (!platform::CloseRequested(opened.window)) {
         platform::PumpEvents(opened.window);
         if (platform::CloseRequested(opened.window))
@@ -393,6 +396,42 @@ int Run(void)
         double dt = now - last;
         last = now;
 
+        /* Lesson 075: the walk — every live entity, once per frame, in
+           slot order. The per-entity work is expressed here, once, and
+           not per type; today it is the demo's kill check. An entity
+           retired in passing is not visited again and no other is
+           skipped — the slots do not move under the walk. */
+        int visited = 0;
+        for (int i = 0; i < ENTITY_CAP; ++i) {
+            if (!store.slots[i].live)
+                continue;
+            visited += 1;
+            if (store.slots[i].health <= 0) {
+                std::printf("engine: walk (frame %ld): retiring slot %d in passing\n",
+                            frame.number, i);
+                EntityRetire(store, store.slots[i]);
+            }
+        }
+        walk_visits += visited;
+        if (frame_number == 1)
+            std::printf("engine: walk (frame 1): visited %d live entities, once each, in slot order; live %d of %d\n",
+                        visited, store.live, ENTITY_CAP);
+
+        /* Lesson 075: the reuse — three requests once the walk has
+           freed three slots. Each lands in a freed slot, before any
+           slot that has never been used. */
+        if (frame_number == 2) {
+            for (int k = 0; k < 3; ++k) {
+                EntityResult made =
+                    EntityCreate(store, table.rows[k % table.count]);
+                if (made.error != ENTITY_OK)
+                    break;
+                std::printf("engine: store: created in slot %d (freed before never-used)\n",
+                            (int)(made.entity - store.slots));
+            }
+            std::printf("engine: store: live %d of %d\n", store.live, ENTITY_CAP);
+        }
+
         double was_x = sprite_x, was_y = sprite_y;
         double move_x = 0.0, move_y = 0.0;
         if (platform::KeyDown(opened.window, platform::KEY_LEFT))
@@ -606,6 +645,11 @@ int Run(void)
     std::printf("engine: demo: %ld frames measured, %d buffers fed, %d effects fired, %d music wraps\n",
                 frame_number, feeds, effect_count, music_wraps);
 
+    /* Lesson 075: the walk's account — one visit per live entity per
+       frame, and nothing else. */
+    std::printf("engine: walk: %ld visits over %ld frames — one per live entity per frame\n",
+                walk_visits, frame_number);
+
     /* The account as the frame-budget table (lesson 058): the frame
        count, the average, the worst frame — and the render attributed to
        its subsystems, the report Part 5's finale grows. */
```

## Exercices

Deux exercices mixtes. Chacun se termine par sa solution — un diff contre l'état
final de cette leçon, plus une visite guidée — après l'énoncé.

### Exercice 1 — La marche qui retire devant *(predict-the-output)*

Le corps de la marche peut retirer *n'importe quelle* entité, pas seulement
celle qu'elle regarde. Prenez un magasin de six entités vivantes aux emplacements
0-5 et faites-la marcher avec un corps qui, en atteignant l'emplacement 1, retire
l'entité de l'emplacement 3. Avant de lancer quoi que ce soit, écrivez la liste
des emplacements que la marche visite, dans l'ordre, et le compte des vivantes
ensuite. Écrivez ensuite les deux mêmes réponses pour le cas miroir : un corps
qui, en atteignant l'emplacement 3, retire l'entité de l'emplacement 1. Lancez
les deux et réconciliez — et dites, en une phrase, ce qui rend les deux réponses
sûres.

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-075/ex1.md)

### Exercice 2 — La carte du magasin *(extend-the-code)*

Les chiffres disent combien ; une carte dit où. Apprenez à l'exécution à
imprimer l'occupation du magasin comme un caractère par emplacement — une entité
vivante et un emplacement libre sont deux caractères — à chacun des trois
moments de la démo : après la création du monde, après les retraits de la
marche, et après la réutilisation. À quoi ressemble la carte à chaque moment ?
Répondez ensuite à la question que la carte ne peut pas trancher : un
emplacement libéré et un emplacement jamais utilisé sont tous deux libres — d'où
le rapport de cette leçon tient-il la distinction entre eux, et que devrait
retenir le magasin pour la montrer dans la carte ?

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-075/ex2.md)

---

**Partie :** [Partie 4 — les services](../../index.md) ·
**Précédente :** [Leçon 074 — un magasin fixe](lesson-074-store.md) ·
**Suivante :** [Leçon 076 — le héros comme entité](lesson-076-hero.md) ·
**Étiquette de code :** [`lesson-075`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-075)

*Page traduite de la version anglaise `book/lessons/part-4/lesson-075-lifetime.md`,
révision `25e6c7f`.*

<!-- translation-source: book/lessons/part-4/lesson-075-lifetime.md @ 25e6c7f -->
