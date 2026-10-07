// table.h — the archetype table as loadable data.
//
// Lesson 071: an entity's facts are authored as data. A table file names
// its columns in a header and holds one row per definition, every value
// whitespace-separated — the format is ours, defined by hand like
// map.txt's and the WAV loader's:
//
//   name x y facing speed health sprite
//   hero 312 232 0 240 3 assets/sprite.ppm
//   slime 400 320 2 96 1 assets/sprite.ppm
//
// The header names the columns, and the loader fills the fields the
// header declares — so the columns may come in any order, but every
// column the format knows comes exactly once. `name` and `sprite` are
// text (a run of non-space bytes); x, y, facing, speed, and health are
// whole numbers, and facing is one of the four the format defines:
// 0 right, 1 down, 2 left, 3 up.
#ifndef TABLE_H
#define TABLE_H

namespace engine {

/* The parse's destination is a fixed array of rows, like the map's kinds
   are. How many rows a table holds is the file's fact and not this
   constant's — lesson 072 moves the rows where the file's count is what
   lands. The name and path widths are the fields' own bounds: a value
   longer than its field is refused, never truncated into one. */
constexpr int TABLE_MAX_ROWS = 16;
constexpr int TABLE_NAME_MAX = 16;
constexpr int TABLE_PATH_MAX = 64;

/* One definition: a row of the table, carrying every value its row
   states — the identity and the attributes an entity is created from. */
struct EntityDef {
    char name[TABLE_NAME_MAX];   /* the definition's identity */
    int x, y;                    /* where it starts, in world pixels */
    int facing;                  /* 0 right, 1 down, 2 left, 3 up */
    int speed;                   /* world pixels per second */
    int health;                  /* points */
    char sprite[TABLE_PATH_MAX]; /* the art file it draws */
};

/* A loaded table: one definition per row. */
struct EntityTable {
    EntityDef rows[TABLE_MAX_ROWS];
    int count;
};

/* A load either hands over a complete table or names what went wrong —
   never a partial table presented as success. */
enum TableError {
    TABLE_OK = 0,
    TABLE_MISSING,   /* the file is not there or cannot be read */
    TABLE_MALFORMED, /* the bytes are not a complete table in the format */
    TABLE_FULL,      /* more rows than the parse's array holds */
};

struct TableResult {
    EntityTable table;
    TableError error; /* TABLE_OK exactly when the table is complete */
};

/* Loads an entity table from a file read whole. The header and the rows
   are parsed byte by byte — no library reads it — and anything the format
   does not describe is refused typed: a column it does not know, a row
   with the wrong number of values, a value where a number is required, a
   value where text is, a name the table already holds. */
TableResult LoadTable(const char *path);

} /* namespace engine */

#endif
