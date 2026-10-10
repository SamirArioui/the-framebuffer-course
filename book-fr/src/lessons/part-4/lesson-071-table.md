# Leçon 071 — la table d'archétypes

{{#include ../../stability-horizon.md}}

## Prose

La partie 3 s'est refermée sur un moteur qui ne tient qu'un objet à la fois — le
sprite de la démo, une carte, une police, deux sons — et la partie 5 s'ouvre sur
un jeu qui doit tenir beaucoup de choses de plusieurs sortes : trois types
d'ennemis, un boss, des projectiles, des rafales de particules. Chacune de ces
choses est un ensemble de *faits* avant d'être quoi que ce soit d'autre : où elle
commence, dans quelle direction elle regarde, à quelle vitesse elle se déplace,
combien de points de vie elle a. Si les faits vivent dans le code du moteur,
chaque changement d'équilibrage est une recompilation et chaque nouveau type est
une copie des nombres du type précédent. L'idée de cette leçon est donc une
seule, et les trois leçons suivantes reposent sur elle : **les faits d'une
entité sont écrits comme de la donnée — une table de définitions, une ligne
chacune, dans un format que ce cours définit à la main.**

### Le fichier, et le format

`assets/entities.txt` est la première table du cours, et toute sa grammaire tient
à l'écran :

```
name x y facing speed health sprite
hero 312 232 0 240 3 assets/sprite.ppm
slime 400 320 2 96 1 assets/sprite.ppm
```

La première ligne nomme les colonnes. Chaque ligne après elle est une
définition : ses valeurs, séparées par des espaces, dans l'ordre où l'en-tête
les nomme. C'est toute la grammaire — pas de blocs clé/valeur, pas d'encadrement
binaire, pas de règles de guillemets. C'est l'habitude que le fichier de carte
de la leçon 052 a commencée : un format texte assez petit pour que chaque règle
s'en vérifie contre le fichier à l'œil nu, et s'analyse octet par octet, parce
qu'aucune bibliothèque ne lit un fichier dans du code moteur visible par
l'élève.

Les deux lignes sont les deux définitions que le jeu a pour l'instant : `hero`,
dont la ligne est celle que la partie 4 déplacera, et `slime`, un type d'ennemi
pour le jeu que la partie 5 assemblera. Toutes deux nomment
`assets/sprite.ppm` comme art parce que le cours ne livre qu'un sprite — la
table est l'endroit où cela changerait, et cela changerait sans recompilation.

### Les colonnes nomment les champs

Les valeurs d'une ligne ne sont jamais « le troisième nombre est la vitesse ».
L'en-tête dit quelle valeur est laquelle, et le chargeur remplit les champs que
l'en-tête déclare. Ce qu'il remplit est une structure de champs nommés que le
jeu lit directement :

```cpp
struct EntityDef {
    char name[TABLE_NAME_MAX];   /* the definition's identity */
    int x, y;                    /* where it starts, in world pixels */
    int facing;                  /* 0 right, 1 down, 2 left, 3 up */
    int speed;                   /* world pixels per second */
    int health;                  /* points */
    char sprite[TABLE_PATH_MAX]; /* the art file it draws */
};
```

Pas un sac de clés lues par nom à l'exécution — la vitesse d'une définition est
`def.speed`, un champ d'une structure, et le chargeur a décidé quelle plage
d'octets le remplit en lisant l'en-tête. Les colonnes peuvent venir dans
n'importe quel ordre : un fichier dont l'en-tête se lit
`speed name sprite health y facing x` est la même table avec ses faits listés
autrement, et l'exercice ci-dessous vous fait prédire ce que cela fait avant de
le lancer.

Les colonnes ont des types, et le type est celui de la colonne, pas la politesse
du fichier :

- `name` et `sprite` sont du **texte** — une suite d'octets sans espace. Le nom
  est l'identité de la définition ; le sprite est le fichier d'art qu'elle
  dessine.
- `x`, `y`, `facing`, `speed` et `health` sont des **nombres entiers** — des
  chiffres, séparés de leurs voisins par des espaces. `facing` est l'un des
  quatre que le format définit : 0 droite, 1 bas, 2 gauche, 3 haut.

Et les champs ont des bornes, parce qu'un champ est un champ et non une
suggestion : un nom plus long que `TABLE_NAME_MAX` ou un chemin plus long que
`TABLE_PATH_MAX` est refusé, jamais tronqué en un autre — un nom tronqué est un
*nom différent*, et un chemin tronqué est un fichier différent. Un nombre est
refusé avant de pouvoir dépasser ce que contient l'arithmétique du lecteur, la
même borne que porte le lecteur de nombres de `map.txt`.

### Ce que l'analyse refuse

« Un chargement donne une table complète ou un échec typé — jamais des données
partielles présentées comme un succès » est la règle que la leçon 044 a donnée
aux sprites et que la leçon 061 a donnée aux échantillons, gardée à l'identique.
Les échecs sont des valeurs nommées, comme partout dans ce moteur :

| Échec | Le fichier qui l'obtient |
| ----- | ------------------------ |
| `TABLE_MISSING` | le fichier n'est pas là, ou l'OS refuse de le lire |
| `TABLE_MALFORMED` | les octets ne sont pas une table complète dans le format du moteur |
| `TABLE_FULL` | plus de lignes que n'en tient le tableau de l'analyse |

et `TABLE_MALFORMED` couvre chaque ligne en désaccord avec le format : une
colonne que le format ne connaît pas, une colonne nommée deux fois ou pas nommée
du tout, une ligne à laquelle il manque une valeur ou qui en a une de trop, une
valeur là où un nombre est requis ou un nombre là où du texte l'est, un `facing`
qui n'est pas l'un des quatre, un nom que la table contient déjà (un nom, une
définition — la table des types de la carte a la même règle), une ligne là où
l'en-tête est attendu, ou un en-tête sans aucune ligne après lui. Tout ce qui
n'est pas décrit est refusé ; rien n'est lu « du mieux possible ».

Pourquoi tant de rigueur ? Parce que les défaillances qu'elle refuse sont pires
que celles qu'elle provoque. Une ligne dont les valeurs sont décalées d'une
colonne par rapport à l'en-tête est un héros avec les points de vie d'un ennemi
et un ennemi avec la vitesse du héros — *et ça tourne très bien* : le mauvais
jeu qui marche est la défaillance qu'aucun rapport n'attrape jamais. Un nom
tronqué à seize octets peut entrer en collision avec une autre définition et le
chargeur répondrait la mauvaise. Un échec typé est la réponse bon marché et
honnête : l'exécution nomme le fichier, nomme l'échec, et aucune définition
n'est jamais utilisée dont le chargeur ne pourrait se porter garant.

### Pourquoi la donnée vaut ce prix

Le but du format est l'exigence qu'il porte : **changer une valeur d'une table
change le jeu sans recompiler le moteur**, et le code du moteur lui-même ne
contient aucune copie par type de ces valeurs. Cherchez la vitesse du héros dans
`src/` : elle n'y est pas — elle est dans un fichier, à côté de la vitesse de
l'autre ligne, modifiable dans le même éditeur. C'est ce qui fait de
l'équilibrage un changement de donnée et d'un nouveau type d'ennemi une nouvelle
ligne plutôt qu'une nouvelle branche.

L'en-tête fait partie de la même promesse. Parce que le fichier nomme ses
propres colonnes, une table reste lisible des mois après l'écriture du code qui
l'analyse, et une valeur qui atterrit dans le mauvais champ y atterrit *par la
déclaration du fichier* — vérifiable contre le fichier — plutôt que par une
supposition du chargeur, qui ne se vérifie contre rien.

### Ce que cette exécution a vérifié, et ce qu'elle n'a pas vérifié

Tous les nombres ci-dessous viennent de vraies exécutions de l'état final de
cette leçon sur cette machine :

- **La table se charge complètement.** L'exécution rapporte
  `engine: table: 2 definitions` puis une ligne par définition —
  `engine: def hero: x 312 y 232 facing 0 speed 240 health 3 sprite
  assets/sprite.ppm` et `engine: def slime: x 400 y 320 facing 2 speed
  96 health 1 sprite assets/sprite.ppm`. Chaque valeur de ces lignes est la
  valeur qu'énonce sa ligne ; la ligne est imprimée pour que la comparaison avec
  le fichier soit la vérification, et elle est exacte.
- **Un fichier manquant échoue de façon typée.** Pointez l'exécution vers un
  répertoire où `assets/entities.txt` est absent et elle dit
  `engine: assets/entities.txt: could not load (missing)` et se termine, le
  fichier nommé et l'échec nommé.
- **Un fichier de forme incorrecte échoue de façon typée.** Quatre corruptions
  ont été taillées dans le vrai fichier et chacune refuse par son nom : une
  ligne avec une valeur là où un nombre est requis (`hero 312 232 0 fast 3 …`)
  et une ligne à qui il manque une valeur rapportent toutes deux `(malformed)` ;
  deux lignes partageant le nom `hero` rapportent `(malformed)` ; dix-sept
  lignes rapportent `(too many rows)` — le tableau de l'analyse en tient seize.
- **L'ordre de l'en-tête est celui du fichier.** Les deux mêmes définitions avec
  les colonnes réordonnées et les valeurs écrites pour correspondre se chargent
  dans exactement les mêmes champs : le rapport est identique, caractère pour
  caractère.

Ce que cette leçon ne fait **pas**, c'est créer quoi que ce soit. La table est de
la donnée ; rien dans l'exécution ne bouge, ne dessine ou ne vit encore. La leçon
072 termine la moitié « chargement » de l'histoire — où les lignes sont gardées
et ce qu'un chargement refusé laisse derrière lui — et la leçon 073 transforme
une ligne en entité sur laquelle le jeu agit.

## Étape de code

Un seul changement pour cette leçon, du fichier aux définitions : `src/table.h` /
`src/table.cpp` accueillent `EntityDef`, `EntityTable`, les échecs typés et
`LoadTable` — l'en-tête et les lignes parcourus octet par octet, chaque valeur
vérifiée contre la colonne qui la revendique, les octets du fichier eux-mêmes
repartant vers l'OS quand la marche est finie. `src/main.cpp` accueille le
démarrage de l'exécution : `assets/entities.txt` est chargé à côté des autres
assets, un chargement réussi rapporte les valeurs de chaque définition comme
vérification au niveau de l'octet, et un échec termine l'exécution par son nom
comme tout autre asset. Le nouvel asset est l'autre moitié de l'étape de code :
`assets/entities.txt`, la ligne du héros et celle d'un ennemi, dans le format
que cette leçon définit. Le flux, la boucle et la couture ne sont pas touchés.
Son état final est étiqueté `lesson-071`.

```diff
diff --git a/assets/entities.txt b/assets/entities.txt
new file mode 100644
index 0000000..0683f17
--- /dev/null
+++ b/assets/entities.txt
@@ -0,0 +1,3 @@
+name x y facing speed health sprite
+hero 312 232 0 240 3 assets/sprite.ppm
+slime 400 320 2 96 1 assets/sprite.ppm
diff --git a/src/main.cpp b/src/main.cpp
index 455b331..7ddd340 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -20,6 +20,7 @@
 #include "frame.h"
 #include "platform.h"
 #include "sprite.h"
+#include "table.h"
 #include "text.h"
 #include "tilemap.h"
 #include "tiles.h"
@@ -162,6 +163,43 @@ int Run(void)
     }
     TileSheet &sheet = tiles_loaded.sheet;
 
+    /* Lesson 071: the run's entities are data. The table file holds one
+       row per definition — its columns named by its header — and the load
+       either hands over every definition or names what went wrong, like
+       every asset above. */
+    TableResult table_loaded = LoadTable("assets/entities.txt");
+    if (table_loaded.error != TABLE_OK) {
+        switch (table_loaded.error) {
+        case TABLE_MISSING:
+            std::fprintf(stderr,
+                         "engine: assets/entities.txt: could not load (missing)\n");
+            break;
+        case TABLE_MALFORMED:
+            std::fprintf(stderr,
+                         "engine: assets/entities.txt: could not load (malformed)\n");
+            break;
+        default:
+            std::fprintf(stderr,
+                         "engine: assets/entities.txt: could not load (too many rows)\n");
+            break;
+        }
+        platform::CloseWindow(opened.window);
+        ArenaRelease(arena);
+        return 1;
+    }
+    EntityTable &table = table_loaded.table;
+
+    /* The byte-level check, before anything uses the table: every
+       definition, carrying the values its row states. */
+    std::printf("engine: table: %d definition%s\n", table.count,
+                table.count == 1 ? "" : "s");
+    for (int i = 0; i < table.count; ++i) {
+        const EntityDef &def = table.rows[i];
+        std::printf("engine: def %s: x %d y %d facing %d speed %d health %d sprite %s\n",
+                    def.name, def.x, def.y, def.facing, def.speed, def.health,
+                    def.sprite);
+    }
+
     /* Lesson 066: the run's two sounds as files' bytes — the music that
        loops and the effect that plays once. Lesson 061's tone leaves the
        run here (it stays on disk: the file lessons 059-065 were built
diff --git a/src/table.cpp b/src/table.cpp
new file mode 100644
index 0000000..76e6d98
--- /dev/null
+++ b/src/table.cpp
@@ -0,0 +1,263 @@
+// table.cpp — the table file's reader: rows of definitions, complete or
+// nothing.
+//
+// Lesson 071: the format's grammar is a paragraph — a header naming the
+// columns, then one row per definition — and this file walks it byte by
+// byte like map.txt's reader does. Every value is checked against the
+// column that claims it: a whole number where a number belongs, a run of
+// non-space bytes where text belongs, and exactly as many values as the
+// header names.
+
+#include "table.h"
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
+/* One whitespace-separated value, exactly as long as the file has it. */
+bool NextToken(const unsigned char *line, int len, int &at,
+               const unsigned char *&token, int &token_len)
+{
+    while (at < len && (line[at] == ' ' || line[at] == '\t'))
+        ++at;
+    if (at >= len)
+        return false;
+    token = line + at;
+    while (at < len && line[at] != ' ' && line[at] != '\t')
+        ++at;
+    token_len = (int)(line + at - token);
+    return true;
+}
+
+/* One whole number: digits, separated from its neighbors by spaces. The
+   bound is the number reader's, not the format's — a number is refused
+   before it can grow past what this arithmetic holds. */
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
+/* One text value: a run of non-space bytes, copied into a field no wider
+   than out_max. A value that does not fit is refused, never truncated —
+   a truncated name is a different name, and a truncated path is a
+   different file. */
+bool ReadText(const unsigned char *line, int len, int &at, char *out,
+              int out_max)
+{
+    while (at < len && (line[at] == ' ' || line[at] == '\t'))
+        ++at;
+    size_t start = (size_t)at;
+    while (at < len && line[at] != ' ' && line[at] != '\t')
+        ++at;
+    int width = (int)((size_t)at - start);
+    if (width == 0 || width >= out_max)
+        return false;
+    for (int i = 0; i < width; ++i)
+        out[i] = (char)line[start + i];
+    out[width] = 0;
+    return true;
+}
+
+/* The columns the format knows. */
+enum Column {
+    COL_NAME,
+    COL_X,
+    COL_Y,
+    COL_FACING,
+    COL_SPEED,
+    COL_HEALTH,
+    COL_SPRITE,
+    COL_COUNT
+};
+
+const char *const COLUMN_NAMES[COL_COUNT] = {
+    "name", "x", "y", "facing", "speed", "health", "sprite"
+};
+
+bool TokenIs(const unsigned char *token, int token_len, const char *name)
+{
+    int n = 0;
+    while (name[n])
+        ++n;
+    if (n != token_len)
+        return false;
+    for (int i = 0; i < n; ++i)
+        if (name[i] != (char)token[i])
+            return false;
+    return true;
+}
+
+/* A text field against a text field: the same bytes, or not. */
+bool SameText(const char *a, const char *b)
+{
+    int i = 0;
+    while (a[i] && a[i] == b[i])
+        ++i;
+    return a[i] == b[i];
+}
+
+int FindColumn(const unsigned char *token, int token_len)
+{
+    for (int c = 0; c < COL_COUNT; ++c)
+        if (TokenIs(token, token_len, COLUMN_NAMES[c]))
+            return c;
+    return -1;
+}
+
+} /* namespace */
+
+TableResult LoadTable(const char *path)
+{
+    TableResult result = {};
+
+    platform::FileData file = platform::ReadFile(path);
+    if (file.error != platform::FILE_OK) {
+        result.error = TABLE_MISSING;
+        return result;
+    }
+
+    Lines lines = { file.data, file.size, 0 };
+    const unsigned char *line = 0;
+    int len = 0;
+    bool ok = true;
+    TableError failure = TABLE_MALFORMED;
+
+    /* The header: the columns this file's rows carry, named one after
+       another. The order is the file's — the loader fills the fields the
+       header declares — but every column the format knows is named, and
+       named once. A name the format does not know is refused here rather
+       than read as something else later. */
+    int order[COL_COUNT];
+    for (int c = 0; c < COL_COUNT; ++c)
+        order[c] = -1;
+    ok = ok && NextLine(lines, line, len);
+    int at = 0;
+    for (int i = 0; ok && i < COL_COUNT; ++i) {
+        const unsigned char *token = 0;
+        int token_len = 0;
+        ok = ok && NextToken(line, len, at, token, token_len);
+        if (ok) {
+            int column = FindColumn(token, token_len);
+            ok = ok && column >= 0;
+            for (int prev = 0; ok && prev < i; ++prev)
+                ok = ok && order[prev] != column; /* one name, one column */
+            order[i] = column;
+        }
+    }
+    const unsigned char *extra = 0;
+    int extra_len = 0;
+    ok = ok && !NextToken(line, len, at, extra, extra_len);
+
+    /* The rows: one definition each, every value landing in the field
+       its column names. The row is refused — the whole file is — when a
+       value is missing or one too many, when a value is not what its
+       column requires, when the facing is not one of the four the format
+       defines, or when the name is one the table already holds (one
+       name, one definition — the map's kind table has the same rule). */
+    while (ok) {
+        if (!NextLine(lines, line, len))
+            break;
+        if (len == 0)
+            break; /* the rows end here; the tail is checked below */
+        if (result.table.count >= TABLE_MAX_ROWS) {
+            failure = TABLE_FULL;
+            ok = false;
+            break;
+        }
+
+        EntityDef &def = result.table.rows[result.table.count];
+        at = 0;
+        for (int i = 0; ok && i < COL_COUNT; ++i) {
+            switch (order[i]) {
+            case COL_NAME:
+                ok = ReadText(line, len, at, def.name, TABLE_NAME_MAX);
+                break;
+            case COL_X:
+                ok = ReadInt(line, len, at, def.x);
+                break;
+            case COL_Y:
+                ok = ReadInt(line, len, at, def.y);
+                break;
+            case COL_FACING:
+                ok = ReadInt(line, len, at, def.facing);
+                ok = ok && def.facing <= 3;
+                break;
+            case COL_SPEED:
+                ok = ReadInt(line, len, at, def.speed);
+                break;
+            case COL_HEALTH:
+                ok = ReadInt(line, len, at, def.health);
+                break;
+            case COL_SPRITE:
+                ok = ReadText(line, len, at, def.sprite, TABLE_PATH_MAX);
+                break;
+            default:
+                ok = false;
+                break;
+            }
+        }
+        while (ok && at < len && (line[at] == ' ' || line[at] == '\t'))
+            ++at;
+        ok = ok && at == len; /* nothing else on the line */
+        for (int prev = 0; ok && prev < result.table.count; ++prev)
+            ok = ok && !SameText(result.table.rows[prev].name, def.name);
+        if (ok)
+            result.table.count += 1;
+    }
+
+    /* After the last row: trailing blank lines, and nothing else. A file
+       that holds a header and no rows is not a table — the load is a
+       complete table or a typed failure. */
+    while (ok && lines.at < lines.size) {
+        ok = ok && NextLine(lines, line, len);
+        ok = ok && len == 0;
+    }
+    ok = ok && result.table.count > 0;
+
+    platform::ReleaseFile(file);
+    if (!ok) {
+        result.table.count = 0;
+        result.error = failure;
+        return result;
+    }
+
+    result.error = TABLE_OK;
+    return result;
+}
+
+} /* namespace engine */
diff --git a/src/table.h b/src/table.h
new file mode 100644
index 0000000..06e0a89
--- /dev/null
+++ b/src/table.h
@@ -0,0 +1,72 @@
+// table.h — the archetype table as loadable data.
+//
+// Lesson 071: an entity's facts are authored as data. A table file names
+// its columns in a header and holds one row per definition, every value
+// whitespace-separated — the format is ours, defined by hand like
+// map.txt's and the WAV loader's:
+//
+//   name x y facing speed health sprite
+//   hero 312 232 0 240 3 assets/sprite.ppm
+//   slime 400 320 2 96 1 assets/sprite.ppm
+//
+// The header names the columns, and the loader fills the fields the
+// header declares — so the columns may come in any order, but every
+// column the format knows comes exactly once. `name` and `sprite` are
+// text (a run of non-space bytes); x, y, facing, speed, and health are
+// whole numbers, and facing is one of the four the format defines:
+// 0 right, 1 down, 2 left, 3 up.
+#ifndef TABLE_H
+#define TABLE_H
+
+namespace engine {
+
+/* The parse's destination is a fixed array of rows, like the map's kinds
+   are. How many rows a table holds is the file's fact and not this
+   constant's — lesson 072 moves the rows where the file's count is what
+   lands. The name and path widths are the fields' own bounds: a value
+   longer than its field is refused, never truncated into one. */
+constexpr int TABLE_MAX_ROWS = 16;
+constexpr int TABLE_NAME_MAX = 16;
+constexpr int TABLE_PATH_MAX = 64;
+
+/* One definition: a row of the table, carrying every value its row
+   states — the identity and the attributes an entity is created from. */
+struct EntityDef {
+    char name[TABLE_NAME_MAX];   /* the definition's identity */
+    int x, y;                    /* where it starts, in world pixels */
+    int facing;                  /* 0 right, 1 down, 2 left, 3 up */
+    int speed;                   /* world pixels per second */
+    int health;                  /* points */
+    char sprite[TABLE_PATH_MAX]; /* the art file it draws */
+};
+
+/* A loaded table: one definition per row. */
+struct EntityTable {
+    EntityDef rows[TABLE_MAX_ROWS];
+    int count;
+};
+
+/* A load either hands over a complete table or names what went wrong —
+   never a partial table presented as success. */
+enum TableError {
+    TABLE_OK = 0,
+    TABLE_MISSING,   /* the file is not there or cannot be read */
+    TABLE_MALFORMED, /* the bytes are not a complete table in the format */
+    TABLE_FULL,      /* more rows than the parse's array holds */
+};
+
+struct TableResult {
+    EntityTable table;
+    TableError error; /* TABLE_OK exactly when the table is complete */
+};
+
+/* Loads an entity table from a file read whole. The header and the rows
+   are parsed byte by byte — no library reads it — and anything the format
+   does not describe is refused typed: a column it does not know, a row
+   with the wrong number of values, a value where a number is required, a
+   value where text is, a name the table already holds. */
+TableResult LoadTable(const char *path);
+
+} /* namespace engine */
+
+#endif
```

## Exercices

Deux exercices mixtes. Chacun se termine par sa solution — un diff contre l'état
final de cette leçon, plus une visite guidée — après l'énoncé.

### Exercice 1 — L'en-tête, réordonné *(predict-the-output)*

Le chargeur remplit les champs que l'en-tête déclare, donc un fichier de table
peut nommer ses colonnes dans n'importe quel ordre — les lignes suivent leur
propre en-tête. Voici les deux mêmes définitions que `assets/entities.txt`, les
colonnes dans un ordre différent et les valeurs écrites pour correspondre :

```
speed name sprite health y facing x
240 hero assets/sprite.ppm 3 232 0 312
96 slime assets/sprite.ppm 1 320 2 400
```

Avant de lancer quoi que ce soit, écrivez ce que le rapport de l'exécution dira
au sujet de `hero` — chaque champ, dans l'ordre propre du rapport — et une
phrase sur pourquoi chaque valeur atterrit où elle atterrit. Puis faites un seul
changement à la ligne du héros et rien d'autre : échangez son `3` et son `240`,
pour que la ligne se lise `3 hero assets/sprite.ppm 240 232 0 312`. Prédisez
encore, avant de lancer : que dit le rapport maintenant, et que dit le chargeur
au sujet de la ligne — s'il dit quoi que ce soit ? Lancez les deux fichiers et
réconciliez vos deux prédictions avec les deux exécutions.

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-071/ex1.md)

### Exercice 2 — La table réécrite *(extend-the-code)*

Le chargeur lit le format de cette leçon ; apprenez au moteur à en écrire un.
Ajoutez un écrivain à côté du chargeur — l'en-tête de cette leçon, puis une
ligne par définition, chaque valeur assemblée à la main en octets et écrite à
travers l'écriture de fichier entier de la couture — dans le format du moteur et
aucun autre. Puis faites de l'exécution un aller-retour : chargez la table,
écrivez-la dans un fichier à vous, rechargez ce fichier avec `LoadTable`, et
comparez chaque champ de chaque définition contre la table qui est sortie —
rapportez le premier champ en désaccord, ou que tous les champs s'accordent.
Comparez le fichier que vous avez écrit avec `assets/entities.txt` hors de
l'exécution. Que prouve l'aller-retour au sujet du format que lire le fichier
une fois ne peut pas prouver ?

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-071/ex2.md)

---

**Partie :** [Partie 4 — les services](../../index.md) ·
**Précédente :** [Leçon 070 — le coût du mixage dans le budget de frames](../part-3/lesson-070-audio-row.md) ·
**Suivante :** [Leçon 072 — le chargement, complet ou nommé](lesson-072-load.md) ·
**Étiquette de code :** [`lesson-071`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-071)

*Page traduite de la version anglaise `book/lessons/part-4/lesson-071-table.md`,
révision `84af294`.*

<!-- translation-source: book/lessons/part-4/lesson-071-table.md @ 84af294 -->
