# Leçon 072 — le chargement, complet ou nommé

{{#include ../../stability-horizon.md}}

## Prose

La leçon 071 s'est terminée sur un chargeur qui tient sa promesse sur le
*format* : chaque ligne vérifiée contre l'en-tête, chaque valeur contre sa
colonne, une table complète ou un échec nommé. Il tenait cette promesse dans un
tableau fixe de seize lignes — et ce tableau est une décision que le fichier n'a
jamais prise. Combien de définitions tient une table est un fait du *fichier*,
comme ses colonnes et ses valeurs. L'idée de cette leçon est donc l'autre moitié
de la promesse d'un chargement : **le chargement est complet ou nommé — et un
chargement qui refuse ne garde rien.** Les lignes déménagent dans l'arena, où
c'est le compte du fichier qui atterrit, et le chargement entier devient une
seule transaction.

### Le compte est un fait du fichier

Une allocation d'arena est un pointeur qui avance (leçon 041) : ce que vous
demandez est ce que vous obtenez, et il ne peut pas grandir. Un chargeur qui
alloue des lignes pendant qu'il analyse doit donc soit deviner le compte à
l'avance, soit réallouer sans cesse — et l'arena n'a aucune réallocation à
offrir. Ce chargeur parcourt le fichier deux fois, ce qui est la forme honnête
pour un allocateur à bump pointer :

1. **la marche de comptage** — la ligne de l'en-tête, puis les lignes jusqu'à ce
   qu'elles finissent, comptées. Rien n'est encore vérifié et rien n'est gardé ;
   le seul produit de la marche est le nombre `rows` ;
2. **la marche de remplissage** — l'en-tête analysé et les lignes remplies, dans
   exactement `rows` définitions allouées depuis l'arena.

Les deux marches s'accordent sur l'endroit où les lignes finissent : la première
ligne vide. Ce qui vient après elle est la queue du fichier, et le format
autorise les lignes vides de fin et rien d'autre — une ligne après le trou est
un fichier malformé, pas une seconde table. (Le remplissage refuse aussi
d'écrire au-delà du compte que la première marche a promis, donc les deux
marches ne peuvent jamais être en désaccord sur la taille de l'allocation.)

La comptabilité de l'arena elle-même montre l'arithmétique. D'après la sonde
jetable ci-dessous, une table de deux lignes atterrit dans 200 octets —
`rows × sizeof(EntityDef)`, cent octets par définition — et une table de deux
cents lignes atterrit dans 22200 : pas une capacité choisie à l'avance, et pas
un octet gaspillé sur des lignes que le fichier n'a pas. Et le seul refus de la
leçon 071 qui a disparu : **dix-sept lignes sont une table maintenant.** Le
tableau de l'analyse en tenait seize ; l'arena tient ce que le fichier tient, et
la seule limite qui reste est la place propre de l'arena, à laquelle elle répond
`TABLE_NO_ROOM`.

### Le chargement est une seule transaction

La marque descend avant que les lignes soient prises et chaque refus fait un
retour arrière jusqu'à elle — la même transaction que le chargeur d'échantillons
de la leçon 061 encadre autour de ses trames, et pour la même raison. Un
chargement qui refuse à mi-chemin ne doit pas laisser une demi-table dans la
mémoire du moteur : le jeu verrait un compte utilisé qui ne redescend jamais, et
les octets d'un fichier que le chargeur a *dit* refuser. Avec le retour arrière,
« rien de partiel gardé » n'est pas un commentaire — c'est un nombre qu'on peut
regarder :

```
assets/entities.txt        -> ok        rows 2, arena 0 -> 200  rows kept
assets/seventeen.txt       -> ok        rows 17, arena 200 -> 1900  rows kept
assets/short-row.txt       -> malformed rows 0, arena 1900 -> 1900  nothing kept
assets/not-a-number.txt    -> malformed rows 0, arena 1900 -> 1900  nothing kept
assets/two-heroes.txt      -> malformed rows 0, arena 1900 -> 1900  nothing kept
assets/missing.txt         -> missing   rows 0, arena 1900 -> 1900  nothing kept
assets/entities.txt        -> ok        rows 2, arena 1900 -> 2100  rows kept
```

Ces sept lignes sont une sonde jetable — pas du code moteur, compilée contre
`table.cpp`, `arena.cpp` et les fichiers de la plateforme pour surveiller
`arena.used` à travers chaque chargement. Le premier chargement prend ses
200 octets et les garde, comme un chargement réussi le doit. La table de
dix-sept lignes prend 1700 de plus. Puis quatre refus d'affilée — une ligne à
qui il manque une valeur, une valeur là où un nombre est requis, deux
définitions nommées `hero`, un fichier qui n'est pas là — et le compte d'octets
utilisés de l'arena ne bouge pas d'un seul octet sur aucun d'eux. Le dernier
chargement reprend ses lignes. C'est la promesse du retour arrière, vérifiée.

Le rapport d'arena de l'exécution elle-même s'accorde depuis l'intérieur du
moteur. Avant cette leçon il lisait
`engine: arena: 1534080 of 33554432 bytes used` ; après elle, `1534280` — les
200 octets des deux définitions et rien d'autre.

### Les échecs sont ceux des chargeurs

L'ensemble d'échecs de la leçon 071 avait un nom de plus que les autres —
`TABLE_FULL` — parce que l'analyse avait une capacité dont elle pouvait manquer.
La capacité disparue, les échecs de la table se fixent sur les trois auxquels
tout chargeur d'asset de ce moteur répond :

| Échec | Le fichier qui l'obtient |
| ----- | ------------------------ |
| `TABLE_MISSING` | le fichier n'est pas là, ou l'OS refuse de le lire |
| `TABLE_MALFORMED` | les octets ne sont pas une table complète dans le format du moteur |
| `TABLE_NO_ROOM` | l'arena n'a pas de place pour les lignes |

`TABLE_MALFORMED` couvre toujours les mêmes refus au niveau des lignes qu'avant :
une colonne que le format ne connaît pas, une colonne nommée deux fois ou pas du
tout, une ligne à laquelle il manque une valeur ou qui en a une de trop, une
valeur là où un nombre est requis, un `facing` qui n'est pas l'un des quatre, un
nom que la table contient déjà, un fichier sans aucune ligne. Ce qui a changé
n'est pas quels fichiers sont refusés mais ce que coûte un refus : rien.

### Ce que cette exécution a vérifié, et ce qu'elle n'a pas vérifié

Tous les nombres ci-dessus viennent de vraies exécutions de l'état final de
cette leçon sur cette machine :

- **La table se charge complètement, dans l'arena.** L'exécution rapporte les
  deux mêmes définitions que la leçon 071 — `engine: table: 2 definitions` et
  les valeurs qu'énoncent leurs lignes — et son rapport d'arena a bougé
  d'exactement `rows × sizeof(EntityDef)` : 200 octets pour deux définitions.
- **Un chargement refusé ne garde rien.** Les quatre refus de la sonde jetable
  ci-dessus, chacun avec le compte d'octets utilisés de l'arena inchangé à
  travers lui.
- **Les fichiers manquants et malformés échouent de façon typée, par leur nom.**
  L'exécution pointée vers un répertoire sans le fichier dit
  `engine: assets/entities.txt: could not load (missing)` ; contre le fichier
  avec `fast` là où un nombre est requis elle dit
  `engine: assets/entities.txt: could not load (malformed)`. Les deux terminent
  l'exécution par son nom, comme tout chargement d'asset au-dessus d'elle.
- **Le compte des lignes est un fait du fichier.** La table de dix-sept lignes
  que la leçon précédente refusait comme `(too many rows)` se charge, dans
  1700 octets d'arena.

Ce que cette leçon ne vérifie **pas**, c'est `TABLE_NO_ROOM` : c'est la réponse
de l'arena, et l'arena de cette machine fait 32 Mo — une table aurait besoin
d'environ trois cent mille lignes pour l'entendre. Le chemin a la même forme que
le `NO_ROOM` de tout autre chargeur (marque, aucune ligne, nommer l'échec), et
la leçon 061 a exercé cette forme là où elle était atteignable.

Rien ici ne change le format ni le jeu : le même fichier, le même rapport, les
mêmes deux définitions. Ce que le jeu a maintenant, c'est un chargement auquel
il peut se fier sur parole — complet, ou nommé, sans rien laissé derrière lui.

## Étape de code

Un seul changement pour cette leçon, du tableau à l'arena : `src/table.h` /
`src/table.cpp` déménagent les lignes dans la mémoire du moteur — le fichier
parcouru une fois pour les compter et une fois pour les remplir — et `LoadTable`
accueille l'arena qu'elle prend et la marque qui fait du chargement une
transaction. Les lignes d'`EntityTable` deviennent un pointeur, `TABLE_MAX_ROWS`
prend sa retraite (le compte est un fait du fichier maintenant), et les échecs
se fixent sur les trois auxquels tout chargeur répond : `TABLE_MISSING`,
`TABLE_MALFORMED`, `TABLE_NO_ROOM`. `src/main.cpp` accueille l'appel et nomme le
troisième échec comme les deux autres. Le format n'est pas touché — même
fichier, mêmes refus — et le rapport de l'exécution est inchangé. Son état final
est étiqueté `lesson-072`.

```diff
diff --git a/src/main.cpp b/src/main.cpp
index 7ddd340..3f07b8b 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -166,8 +166,9 @@ int Run(void)
     /* Lesson 071: the run's entities are data. The table file holds one
        row per definition — its columns named by its header — and the load
        either hands over every definition or names what went wrong, like
-       every asset above. */
-    TableResult table_loaded = LoadTable("assets/entities.txt");
+       every asset above. Lesson 072: the rows are the arena's, and a
+       refused load keeps none of them. */
+    TableResult table_loaded = LoadTable(arena, "assets/entities.txt");
     if (table_loaded.error != TABLE_OK) {
         switch (table_loaded.error) {
         case TABLE_MISSING:
@@ -180,7 +181,7 @@ int Run(void)
             break;
         default:
             std::fprintf(stderr,
-                         "engine: assets/entities.txt: could not load (too many rows)\n");
+                         "engine: assets/entities.txt: could not load (no room)\n");
             break;
         }
         platform::CloseWindow(opened.window);
diff --git a/src/table.cpp b/src/table.cpp
index 76e6d98..c589b8d 100644
--- a/src/table.cpp
+++ b/src/table.cpp
@@ -7,6 +7,11 @@
 // column that claims it: a whole number where a number belongs, a run of
 // non-space bytes where text belongs, and exactly as many values as the
 // header names.
+//
+// Lesson 072: the rows live in the arena. How many there are is the
+// file's fact, so the file is walked once to count them and once to fill
+// them, and the whole load is bracketed by a mark — a refused load rolls
+// the arena back and keeps nothing.
 
 #include "table.h"
 
@@ -141,7 +146,7 @@ int FindColumn(const unsigned char *token, int token_len)
 
 } /* namespace */
 
-TableResult LoadTable(const char *path)
+TableResult LoadTable(Arena &arena, const char *path)
 {
     TableResult result = {};
 
@@ -151,22 +156,51 @@ TableResult LoadTable(const char *path)
         return result;
     }
 
+    /* The first walk: the header's line, then the rows, counted. How many
+       rows a table holds is the file's fact — never a capacity the engine
+       picked — so the count comes first and the arena is asked for
+       exactly that many rows. The rows end at the first blank line; what
+       follows them is checked against the format in the second walk. */
     Lines lines = { file.data, file.size, 0 };
     const unsigned char *line = 0;
     int len = 0;
-    bool ok = true;
-    TableError failure = TABLE_MALFORMED;
+    bool ok = NextLine(lines, line, len); /* the header's line */
+    int rows = 0;
+    while (ok && NextLine(lines, line, len)) {
+        if (len == 0)
+            break;
+        rows += 1;
+    }
 
-    /* The header: the columns this file's rows carry, named one after
-       another. The order is the file's — the loader fills the fields the
-       header declares — but every column the format knows is named, and
-       named once. A name the format does not know is refused here rather
-       than read as something else later. */
+    /* The rows, into the arena. The mark is the load's transaction: from
+       here on a refusal rolls the arena back, and a refused load leaves
+       no partial rows behind — the used count does not move. */
+    size_t mark = ArenaMark(arena);
+    EntityDef *defs = 0;
+    if (ok && rows > 0) {
+        defs = (EntityDef *)ArenaAlloc(arena, (size_t)rows * sizeof(EntityDef),
+                                       4);
+        if (!defs) {
+            ArenaRollback(arena, mark);
+            platform::ReleaseFile(file);
+            result.error = TABLE_NO_ROOM;
+            return result;
+        }
+    }
+
+    /* The second walk: the header and the rows, byte by byte — every
+       value landing in the field its column names. A row is refused — the
+       whole file is — when a value is missing or one too many, when a
+       value is not what its column requires, when the facing is not one
+       of the four the format defines, or when the name is one the table
+       already holds. The fill never writes past the count the first walk
+       promised. */
+    lines.at = 0;
+    ok = ok && NextLine(lines, line, len);
+    int at = 0;
     int order[COL_COUNT];
     for (int c = 0; c < COL_COUNT; ++c)
         order[c] = -1;
-    ok = ok && NextLine(lines, line, len);
-    int at = 0;
     for (int i = 0; ok && i < COL_COUNT; ++i) {
         const unsigned char *token = 0;
         int token_len = 0;
@@ -183,24 +217,17 @@ TableResult LoadTable(const char *path)
     int extra_len = 0;
     ok = ok && !NextToken(line, len, at, extra, extra_len);
 
-    /* The rows: one definition each, every value landing in the field
-       its column names. The row is refused — the whole file is — when a
-       value is missing or one too many, when a value is not what its
-       column requires, when the facing is not one of the four the format
-       defines, or when the name is one the table already holds (one
-       name, one definition — the map's kind table has the same rule). */
     while (ok) {
         if (!NextLine(lines, line, len))
             break;
         if (len == 0)
             break; /* the rows end here; the tail is checked below */
-        if (result.table.count >= TABLE_MAX_ROWS) {
-            failure = TABLE_FULL;
-            ok = false;
+        if (result.table.count >= rows) {
+            ok = false; /* the fill never writes past the count's promise */
             break;
         }
 
-        EntityDef &def = result.table.rows[result.table.count];
+        EntityDef &def = defs[result.table.count];
         at = 0;
         for (int i = 0; ok && i < COL_COUNT; ++i) {
             switch (order[i]) {
@@ -235,7 +262,7 @@ TableResult LoadTable(const char *path)
             ++at;
         ok = ok && at == len; /* nothing else on the line */
         for (int prev = 0; ok && prev < result.table.count; ++prev)
-            ok = ok && !SameText(result.table.rows[prev].name, def.name);
+            ok = ok && !SameText(defs[prev].name, def.name);
         if (ok)
             result.table.count += 1;
     }
@@ -251,11 +278,13 @@ TableResult LoadTable(const char *path)
 
     platform::ReleaseFile(file);
     if (!ok) {
+        ArenaRollback(arena, mark);
         result.table.count = 0;
-        result.error = failure;
+        result.error = TABLE_MALFORMED;
         return result;
     }
 
+    result.table.rows = defs;
     result.error = TABLE_OK;
     return result;
 }
diff --git a/src/table.h b/src/table.h
index 06e0a89..c570c17 100644
--- a/src/table.h
+++ b/src/table.h
@@ -18,14 +18,15 @@
 #ifndef TABLE_H
 #define TABLE_H
 
+#include "arena.h"
+
 namespace engine {
 
-/* The parse's destination is a fixed array of rows, like the map's kinds
-   are. How many rows a table holds is the file's fact and not this
-   constant's — lesson 072 moves the rows where the file's count is what
-   lands. The name and path widths are the fields' own bounds: a value
-   longer than its field is refused, never truncated into one. */
-constexpr int TABLE_MAX_ROWS = 16;
+/* The parse's destination is the arena: a table's rows are the file's
+   fact — how many there are is what the file says, and the arena gives
+   exactly that many. The name and path widths are the fields' own
+   bounds: a value longer than its field is refused, never truncated into
+   one. */
 constexpr int TABLE_NAME_MAX = 16;
 constexpr int TABLE_PATH_MAX = 64;
 
@@ -40,19 +41,21 @@ struct EntityDef {
     char sprite[TABLE_PATH_MAX]; /* the art file it draws */
 };
 
-/* A loaded table: one definition per row. */
+/* A loaded table: one definition per row, in the arena — as many rows as
+   the file has, and not one more. */
 struct EntityTable {
-    EntityDef rows[TABLE_MAX_ROWS];
+    EntityDef *rows;
     int count;
 };
 
 /* A load either hands over a complete table or names what went wrong —
-   never a partial table presented as success. */
+   never a partial table presented as success. These are the loaders'
+   failures, the same three every asset in this engine answers with. */
 enum TableError {
     TABLE_OK = 0,
     TABLE_MISSING,   /* the file is not there or cannot be read */
     TABLE_MALFORMED, /* the bytes are not a complete table in the format */
-    TABLE_FULL,      /* more rows than the parse's array holds */
+    TABLE_NO_ROOM,   /* the arena had no room for the rows */
 };
 
 struct TableResult {
@@ -64,8 +67,10 @@ struct TableResult {
    are parsed byte by byte — no library reads it — and anything the format
    does not describe is refused typed: a column it does not know, a row
    with the wrong number of values, a value where a number is required, a
-   value where text is, a name the table already holds. */
-TableResult LoadTable(const char *path);
+   value where text is, a name the table already holds. The rows are
+   copied into the arena behind a mark, and every refusal path rolls back
+   to it: a load that refuses leaves nothing behind. */
+TableResult LoadTable(Arena &arena, const char *path);
 
 } /* namespace engine */
 
```

## Exercices

Deux exercices mixtes. Chacun se termine par sa solution — un diff contre l'état
final de cette leçon, plus une visite guidée — après l'énoncé.

### Exercice 1 — La table avec un trou *(predict-the-output)*

Un fichier de table dont les lignes s'arrêtent au milieu et reprennent après une
ligne vide :

```
name x y facing speed health sprite
hero 312 232 0 240 3 assets/sprite.ppm

slime 400 320 2 96 1 assets/sprite.ppm
```

Avant de lancer quoi que ce soit, écrivez trois prédictions : ce que le chargeur
rapporte — son échec typé, ou un succès ; combien de définitions il remet ; et
le compte d'octets utilisés de l'arena avant et après le chargement, avec la
table du cours chargée d'abord. Lancez ensuite et réconciliez les trois, et
nommez quelle marche a décidé chaque réponse.

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-072/ex1.md)

### Exercice 2 — Le coût du chargement, mesuré *(measure-the-performance)*

Le rapport de l'exécution dit que le chargement a eu lieu ; rien ne dit ce qu'il
a coûté. Mettez-y un nombre : mesurez `LoadTable` sur votre machine à quatre
tailles de table — 2, 20, 200 et 2000 définitions — avec `platform::Now()` autour
de l'appel, et rapportez le temps du chargement et la croissance de l'arena à
chaque taille. Où passe le temps quand le nombre de lignes grandit par dizaines,
et qu'est-ce qui, dans le chargeur, en est responsable ? Changeriez-vous quoi
que ce soit pour les tables de ce jeu — et quelle serait votre réponse si une
table venait à tenir dix mille lignes ?

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-072/ex2.md)

---

**Partie :** [Partie 4 — les services](../../index.md) ·
**Précédente :** [Leçon 071 — la table d'archétypes](lesson-071-table.md) ·
**Suivante :** [Leçon 073 — les entités en lignes](lesson-073-rows.md) ·
**Étiquette de code :** [`lesson-072`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-072)

*Page traduite de la version anglaise `book/lessons/part-4/lesson-072-load.md`,
révision `2794ee0`.*

<!-- translation-source: book/lessons/part-4/lesson-072-load.md @ 2794ee0 -->
