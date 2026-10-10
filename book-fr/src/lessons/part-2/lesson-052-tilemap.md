# Leçon 052 — le format d'asset du tilemap

{{#include ../../stability-horizon.md}}

## Prose

Le monde d'un jeu est de la donnée avant d'être des pixels. Cette leçon définit
le **format de tilemap** du cours — le fichier qui dit de quoi le monde est
fait — et le fait charger : des dimensions, des types de tuiles avec leur
solidité, un caractère par cellule, le tout assez petit pour être défini à la
main et lu à l'œil. La quatrième obligation du MVD (*monde : une seule carte à
défilement, collision de tuiles*) commence ici : la leçon 053 dessine cette
donnée, la leçon 055 l'interroge, et la démo de clôture tourne dessus. Le
format est fixé dans cette leçon, comme chaque autre contrat du cours.

### Le format, défini à la main

`assets/map.txt` — un fichier texte en trois parties :

```
20 12 3
. 0
# 1
w 0
####################
#..................#
#..................#
#....##......##....#
#....##......##....#
#..................#
#..................#
#...w....##....w...#
#...w....##....w...#
#..................#
#..................#
####################
```

1. **La première ligne : trois nombres** — la largeur, la hauteur, et combien
   de types de tuiles le fichier définit. Chaque ligne suivante est vérifiée
   contre ces comptes.
2. **La table des types : une ligne par type** — le caractère qui nomme le
   type, puis sa solidité (`0` ou `1`). La solidité est de la donnée de
   collision (la leçon 055 la lira), et elle voyage dans le *format* dès le
   premier jour parce que la carte est la donnée du monde : quelles tuiles
   bloquent le déplacement est un fait sur le monde, pas sur le moteur de
   rendu.
3. **Les lignes : exactement `height` lignes d'exactement `width`
   caractères**, chaque caractère nommant un type de la table. Les lignes
   *sont* la carte — ce fichier se lit comme de l'art ASCII, ce qui est toute
   la raison de définir un format texte plutôt que d'empaqueter des octets.

L'exemple est une salle de 20×12 : des murs `#` (solides) sur le pourtour et en
quatre piliers, un sol `.`, de l'eau peu profonde `w` (dessinée mais praticable
— la table des types le dit, et la leçon 055 y croira).

### Complet, ou rien

`LoadTileMap` lit le fichier entier à travers la couture (leçon 037) et
l'analyse ligne par ligne. Son contrat est celui de la spéc : *un chargement
donne soit une carte complète, soit un échec typé — jamais une carte partielle
présentée comme un succès.* Les règles qu'il fait respecter, chacune avec sa
raison :

- la première ligne est trois nombres, dans les bornes (`≤ 256` par dimension,
  `≤ 8` types) et rien d'autre ;
- chaque ligne de type est un caractère suivi de `0` ou `1` — et aucun
  caractère ne nomme deux types ;
- chaque ligne est exactement `width` caractères — pas un de moins, pas un de
  plus ;
- chaque caractère de chaque ligne nomme un type de la table ;
- après la dernière ligne : les lignes vides de fin sont tolérées, **le contenu
  ne l'est pas**.

Les cellules atterrissent dans un bloc d'arena de `width × height` octets, un
index de type chacune ; à la moindre défaillance, le bloc est ramené en arrière
(la marque de la leçon 041) et la structure de carte reste vide. L'échec typé
est la réponse — l'appelant voit `TILE_MALFORMED` et aucune carte, exactement
comme le contrat du chargeur de sprites de la leçon 044.

L'exécution, sur le fichier ci-dessus :

```
engine: map assets/map.txt: 20x12, 3 kinds
engine: map kind '.' (solid 0): 164 cells
engine: map kind '#' (solid 1): 72 cells
engine: map kind 'w' (solid 0): 4 cells
engine: map check: 240 cells, 0 unknown, 2 of 2 corners solid
```

Le chargement est complet au seul sens qui compte : **164 + 72 + 4 = 240 =
20 × 12** — chaque cellule du fichier est une cellule de la carte, comptée. La
table des types est passée avec sa solidité (`#` est solide ; les coins de
cette carte sont des murs et la vérification le dit), et aucune cellule ne
nomme un type hors de la table (le chargeur aurait refusé le fichier entier
plutôt que de laisser cela arriver).

### Les échecs typés, nommés

Une copie corrompue du fichier échoue ainsi. L'état final a un seul message
pour chaque sorte d'erreur — chaque exécution ci-dessous est une copie avec
exactement une faute :

```
$ ./build/game    # the header claims "0 12 3", a row is 19 characters,
$ ./build/game    # a cell is '?' no kind claims, two rows are missing,
$ ./build/game    # or a stray line follows the last row — every time:
engine: assets/map.txt: not a complete map
```

Le rapport ne dit pas encore *quelle* règle le fichier a enfreinte (l'exercice à
la fin de cette leçon le scinde en cas nommés), mais le contrat fait déjà son
vrai travail : l'exécution rapporte un échec typé et ne remet **aucune carte** —
pas de cellules à moitié chargées, pas de mauvais compte de cellules, pas de
crash. Les chemins d'échec sont testés autant que le chemin de succès ; c'est
ce que « un échec typé » achète.

### Pourquoi la carte est de la donnée

La spéc derrière cette leçon dit le but tout haut : *les mondes peuvent être
écrits comme des fichiers et testés sans écran.* Tout ce que le moteur sait du
monde vient de ce fichier — sa taille, ses types, quels types bloquent le
déplacement — et chaque réponse à son sujet se calcule dans un harnais de test
sans fenêtre, sans dessin et sans joueur. Quand la leçon 055 répondra « ce
rectangle est-il dans un mur ? », la réponse viendra de ces octets et de rien
d'autre. Quand la démo de clôture (leçon 057) fera défiler un monde, elle fera
défiler *cette* donnée. Les auteurs changent le monde en éditant un fichier
texte ; le moteur le relit au démarrage ; rien d'autre ne bouge.

## Étape de code

Un seul changement pour cette leçon : `assets/map.txt` est écrit (une carte de
20×12 au format ci-dessus), `src/tilemap.h` / `src/tilemap.cpp` apportent le
lecteur (orienté lignes, complet-ou-rien, solidité portée par type), et
`main.cpp` charge la carte, rapporte ses types et ses comptes de cellules, et
vérifie le chargement contre l'arithmétique propre du fichier. La police, le
texte et le blitter restent intacts. Son état final est étiqueté `lesson-052`.

```diff
diff --git a/assets/map.txt b/assets/map.txt
new file mode 100644
index 0000000..f142cb1
--- /dev/null
+++ b/assets/map.txt
@@ -0,0 +1,16 @@
+20 12 3
+. 0
+# 1
+w 0
+####################
+#..................#
+#..................#
+#....##......##....#
+#....##......##....#
+#..................#
+#..................#
+#...w....##....w...#
+#...w....##....w...#
+#..................#
+#..................#
+####################
diff --git a/src/main.cpp b/src/main.cpp
index c60358b..b63c353 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -15,6 +15,7 @@
 #include "platform.h"
 #include "sprite.h"
 #include "text.h"
+#include "tilemap.h"
 
 namespace engine {
 
@@ -332,6 +333,55 @@ int Run(void)
                     slot_state[0], slot_state[1], slot_state[2], slot_state[3]);
     }
 
+    /* Lesson 052: the world as data — the map file, loaded whole, its
+       cells counted against the file's own rows. */
+    const char *map_path = "assets/map.txt";
+    TileResult map_loaded = LoadTileMap(arena, map_path);
+    if (map_loaded.error != TILE_OK) {
+        switch (map_loaded.error) {
+        case TILE_MISSING:
+            std::fprintf(stderr, "engine: %s: missing or unreadable\n",
+                         map_path);
+            break;
+        case TILE_MALFORMED:
+            std::fprintf(stderr,
+                         "engine: %s: not a complete map\n", map_path);
+            break;
+        default:
+            std::fprintf(stderr, "engine: %s: no room in the arena\n",
+                         map_path);
+            break;
+        }
+        platform::CloseWindow(opened.window);
+        ArenaRelease(arena);
+        return 1;
+    }
+    TileMap &map = map_loaded.map;
+    std::printf("engine: map %s: %dx%d, %d kinds\n", map_path, map.width,
+                map.height, map.kind_count);
+    for (int k = 0; k < map.kind_count; ++k) {
+        int count = 0;
+        for (int y = 0; y < map.height; ++y)
+            for (int x = 0; x < map.width; ++x)
+                if (TileAt(map, x, y) == k)
+                    ++count;
+        std::printf("engine: map kind '%c' (solid %d): %d cells\n",
+                    map.kinds[k].cell, map.kinds[k].solid, count);
+    }
+    int unknown = 0, solid_corners = 0;
+    for (int y = 0; y < map.height; ++y)
+        for (int x = 0; x < map.width; ++x) {
+            int kind = TileAt(map, x, y);
+            if (kind < 0 || kind >= map.kind_count)
+                ++unknown;
+        }
+    if (map.kinds[TileAt(map, 0, 0)].solid)
+        ++solid_corners;
+    if (map.kinds[TileAt(map, map.width - 1, map.height - 1)].solid)
+        ++solid_corners;
+    std::printf("engine: map check: %d cells, %d unknown, %d of 2 corners solid\n",
+                map.width * map.height, unknown, solid_corners);
+
     double sprite_x = (FRAME_WIDTH - sprite.width) / 2.0;
     double sprite_y = (FRAME_HEIGHT - sprite.height) / 2.0;
     double started = platform::Now();
diff --git a/src/tilemap.cpp b/src/tilemap.cpp
new file mode 100644
index 0000000..fadf9fe
--- /dev/null
+++ b/src/tilemap.cpp
@@ -0,0 +1,164 @@
+// tilemap.cpp — the map file's reader: line by line, complete or nothing.
+//
+// Lesson 052: the format is three counts, a kind table, and one character
+// per cell — small enough that every rule here can be checked against the
+// file with your own eyes.
+
+#include "tilemap.h"
+
+#include "platform.h"
+
+namespace engine {
+namespace {
+
+/* Line-oriented parsing over the file's bytes: the format is lines, so
+   the reader is lines. */
+struct Lines {
+    const unsigned char *data;
+    size_t size;
+    size_t at; /* the start of the current line */
+};
+
+bool NextLine(Lines &lines, const unsigned char *&line, int &len)
+{
+    if (lines.at >= lines.size)
+        return false;
+    size_t start = lines.at;
+    while (lines.at < lines.size && lines.data[lines.at] != '\n')
+        ++lines.at;
+    len = (int)(lines.at - start);
+    line = lines.data + start;
+    if (lines.at < lines.size)
+        ++lines.at; /* consume the newline */
+    return true;
+}
+
+/* One decimal number, separated from its neighbors by spaces. */
+bool ReadInt(const unsigned char *line, int len, int &at, int &out)
+{
+    while (at < len && (line[at] == ' ' || line[at] == '\t'))
+        ++at;
+    if (at >= len || line[at] < '0' || line[at] > '9')
+        return false;
+    int value = 0;
+    while (at < len && line[at] >= '0' && line[at] <= '9') {
+        value = value * 10 + (line[at] - '0');
+        if (value > 1000000)
+            return false;
+        ++at;
+    }
+    out = value;
+    return true;
+}
+
+} /* namespace */
+
+TileResult LoadTileMap(Arena &arena, const char *path)
+{
+    TileResult result = { { 0, 0, 0, { { 0, 0 } }, 0 }, TILE_OK };
+
+    platform::FileData file = platform::ReadFile(path);
+    if (file.error != platform::FILE_OK) {
+        result.error = TILE_MISSING;
+        return result;
+    }
+
+    Lines lines = { file.data, file.size, 0 };
+    const unsigned char *line = 0;
+    int len = 0;
+    bool ok = true;
+
+    /* The first line: width height kind-count — the file's only numbers,
+       and the counts every later line is checked against. */
+    int width = 0, height = 0, kind_count = 0;
+    ok = ok && NextLine(lines, line, len);
+    int at = 0;
+    ok = ok && ReadInt(line, len, at, width);
+    ok = ok && ReadInt(line, len, at, height);
+    ok = ok && ReadInt(line, len, at, kind_count);
+    ok = ok && at == len; /* nothing else on the line */
+    ok = ok && width > 0 && width <= TILE_MAX_DIM;
+    ok = ok && height > 0 && height <= TILE_MAX_DIM;
+    ok = ok && kind_count > 0 && kind_count <= TILE_MAX_KINDS;
+
+    /* The kind table: one character and its solidity per kind, in order. */
+    TileKind kinds[TILE_MAX_KINDS];
+    for (int k = 0; ok && k < kind_count; ++k) {
+        ok = ok && NextLine(lines, line, len);
+        ok = ok && len >= 3;
+        if (ok) {
+            kinds[k].cell = (char)line[0];
+            int solid = -1;
+            int at2 = 1;
+            ok = ok && ReadInt(line, len, at2, solid);
+            ok = ok && (solid == 0 || solid == 1);
+            ok = ok && at2 == len;
+            kinds[k].solid = (unsigned char)solid;
+        }
+        for (int prev = 0; ok && prev < k; ++prev)
+            if (kinds[prev].cell == kinds[k].cell)
+                ok = false; /* one character, one kind */
+    }
+
+    /* The cells: exactly height rows of exactly width characters, every
+       character one the kind table names. */
+    size_t mark = ArenaMark(arena);
+    unsigned char *cells = 0;
+    if (ok) {
+        cells = (unsigned char *)ArenaAlloc(arena, (size_t)width * height, 1);
+        if (!cells) {
+            ArenaRollback(arena, mark);
+            platform::ReleaseFile(file);
+            result.error = TILE_NO_ROOM;
+            return result;
+        }
+    }
+    for (int y = 0; ok && y < height; ++y) {
+        ok = ok && NextLine(lines, line, len);
+        ok = ok && len == width;
+        for (int x = 0; ok && x < width; ++x) {
+            int kind = -1;
+            for (int k = 0; k < kind_count; ++k)
+                if (kinds[k].cell == (char)line[x]) {
+                    kind = k;
+                    break;
+                }
+            if (kind < 0)
+                ok = false; /* a character no kind claims */
+            else
+                cells[y * width + x] = (unsigned char)kind;
+        }
+    }
+
+    /* After the last row: trailing blank lines, and nothing else. */
+    while (ok && lines.at < lines.size) {
+        ok = ok && NextLine(lines, line, len);
+        ok = ok && len == 0;
+    }
+
+    if (!ok) {
+        ArenaRollback(arena, mark);
+        platform::ReleaseFile(file);
+        result.error = TILE_MALFORMED;
+        return result;
+    }
+
+    platform::ReleaseFile(file);
+    result.map.width = width;
+    result.map.height = height;
+    result.map.kind_count = kind_count;
+    for (int k = 0; k < kind_count; ++k)
+        result.map.kinds[k] = kinds[k];
+    result.map.cells = cells;
+    result.error = TILE_OK;
+    return result;
+}
+
+int TileAt(const TileMap &map, int x, int y)
+{
+    if (x < 0 || x >= map.width || y < 0 || y >= map.height)
+        return -1; /* outside the map: the defined answer, not a read */
+    return map.cells[y * map.width + x];
+}
+
+} /* namespace engine */
diff --git a/src/tilemap.h b/src/tilemap.h
new file mode 100644
index 0000000..1c076cf
--- /dev/null
+++ b/src/tilemap.h
@@ -0,0 +1,67 @@
+// tilemap.h — the tilemap as loadable world data.
+//
+// Lesson 052: the map is a file, the format is ours, and the load is
+// complete or nothing. The format, defined by hand:
+//
+//   <width> <height> <kind-count>       one line, three numbers
+//   <cell-char> <solid: 0|1>            one line per tile kind
+//   <width> characters                  one line per map row
+//
+// Every row is exactly <width> characters; every character names a kind
+// from the table; the table carries each kind's solidity, so collision
+// queries (lesson 055) are answered from the map data alone.
+#ifndef TILEMAP_H
+#define TILEMAP_H
+
+#include "arena.h"
+
+namespace engine {
+
+/* The bounds the format fixes — a map bigger than this is a different
+   format's file. */
+constexpr int TILE_MAX_DIM = 256;
+constexpr int TILE_MAX_KINDS = 8;
+
+/* One tile kind: the character that names it in the file, and whether it
+   is solid for collision (lesson 055 reads this; the format carries it
+   from the first day). */
+struct TileKind {
+    char cell;
+    unsigned char solid;
+};
+
+/* A loaded map: its dimensions, its kinds, and one kind index per cell. */
+struct TileMap {
+    int width;
+    int height;
+    int kind_count;
+    TileKind kinds[TILE_MAX_KINDS];
+    unsigned char *cells; /* width * height kind indices, in the arena */
+};
+
+/* A load either hands over a complete map or names what went wrong —
+   never a partial map presented as success. */
+enum TileError {
+    TILE_OK = 0,
+    TILE_MISSING,   /* the file is not there or cannot be read */
+    TILE_MALFORMED, /* the bytes do not form a complete map */
+    TILE_NO_ROOM,   /* the arena had no room for the cells */
+};
+
+struct TileResult {
+    TileMap map;
+    TileError error;
+};
+
+/* Loads a map from a file read whole: the first line's three numbers, the
+   kind table, then exactly height rows of exactly width characters. Any
+   deviation — a short row, an extra line, a character no kind claims —
+   is a typed failure, and nothing is handed over. */
+TileResult LoadTileMap(Arena &arena, const char *path);
+
+/* The kind of a cell, or -1 outside the map. */
+int TileAt(const TileMap &map, int x, int y);
+
+} /* namespace engine */
+
+#endif
```

## Exercices

Deux extensions « faites-le vôtre ». Chacune se termine par sa solution — un
diff contre l'état final de cette leçon, plus une visite guidée — après
l'énoncé.

### Exercice 1 — L'échec qui se nomme lui-même *(extend-the-code)*

L'unique `TILE_MALFORMED` du chargeur couvre cinq fautes différentes, et un
rapport qui dit « malformed » vous envoie lire le fichier à la main. Scindez
l'échec en cas nommés — l'en-tête, la table des types, les lignes, le contenu
final — faites-les passer par l'enum et par le rapport de l'exécution, et
relancez chaque copie corrompue de la leçon pour vérifier que chaque rapport
nomme sa propre faute. Où se trouve la frontière entre « les lignes sont
fausses » et « la table des types est fausse » si une ligne utilise un
caractère que la table n'a jamais défini ?

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-052/ex1.md)

### Exercice 2 — La carte en caractères *(extend-the-code)*

Vous avez une police et une carte ; dessinez la carte comme vue de débogage —
chaque cellule comme le caractère de son type à travers `DrawText`, une ligne
de glyphes par ligne de cellules. Relisez ensuite deux lignes et comparez leur
encre à l'arithmétique propre du fichier (comptez les caractères par ligne et
l'encre que porte chaque glyphe). Quelle vérification est la plus précise —
les emplacements portant de l'encre, ou les pixels d'encre — et que dit la
réponse sur la vérification de dessins contre de la donnée ?

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-052/ex2.md)

---

**Partie :** [Partie 2 — le rendu logiciel](../../index.md) ·
**Précédente :** [Leçon 051 — du texte à l'écran](lesson-051-text.md) ·
**Suivante :** [Leçon 053 — le dessin de tilemap](lesson-053-tiles.md) ·
**Étiquette de code :** [`lesson-052`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-052)

*Page traduite de la version anglaise `book/lessons/part-2/lesson-052-tilemap.md`,
révision `cc9b3fa`.*

<!-- translation-source: book/lessons/part-2/lesson-052-tilemap.md @ cc9b3fa -->
