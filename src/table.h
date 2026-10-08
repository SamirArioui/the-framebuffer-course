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
// header declares — so the columns may come in any order, and every
// column the format knows comes at most once. `name`, `sprite`, and
// `fires` are text (a run of non-space bytes); the rest are whole
// numbers, and facing is one of the four the format defines: 0 right,
// 1 down, 2 left, 3 up.
//
// Lesson 087: the format grows by named columns, additively. A file's
// header names the columns it uses — and may omit any column the format
// knows; the unnamed fields take the defaults the format defines below.
// This is a deliberate fix-forward of lesson 071/072's refusal edge ("a
// column not named at all"): a file may now omit columns, because the
// game's facts (a weapon's damage, a projectile's life) must not be
// crammed into columns that would lie about them, and every file the
// course has shipped keeps loading byte-for-byte. A column the format
// does *not* know is still a malformed file — and so is a name the
// table already holds, or a row with one value too few or too many.
#ifndef TABLE_H
#define TABLE_H

#include "arena.h"
#include "sprite.h"

namespace engine {

/* The parse's destination is the arena: a table's rows are the file's
   fact — how many there are is what the file says, and the arena gives
   exactly that many. The name and path widths are the fields' own
   bounds: a value longer than its field is refused, never truncated into
   one. */
constexpr int TABLE_NAME_MAX = 16;
constexpr int TABLE_PATH_MAX = 64;

/* Lesson 087: the behavior column's values — what a kind does each
   frame, the fact its row carries. The format defines the spellings,
   like it defines facing's four numbers: `none` stands still, `fly` is a
   projectile in flight, and chase / keep / flee / boss are the enemy
   behaviors (lessons 089-090) — the boss's value names its pattern.
   Lesson 093: `settle` is a burst's particle — eased travel out from
   its burst point, retiring where it settles. */
enum BehaviorKind {
    BEHAVIOR_NONE = 0,
    BEHAVIOR_FLY,
    BEHAVIOR_CHASE,
    BEHAVIOR_KEEP,
    BEHAVIOR_FLEE,
    BEHAVIOR_BOSS,
    BEHAVIOR_SETTLE,
    BEHAVIOR_COUNT
};

/* One definition: a row of the table, carrying every value its row
   states — the identity and the attributes an entity is created from.
   The image is the one fact the file states as a name: the run loads the
   art the sprite column names and hands the definition its image, so an
   entity created from the definition is answered from the definition
   alone. A weapon is one of these rows (lesson 087): it names the
   projectile kind it fires and carries its rate and damage — and is
   never itself an entity. */
struct EntityDef {
    char name[TABLE_NAME_MAX];   /* the definition's identity */
    int x, y;                    /* where it starts, in world pixels */
    int facing;                  /* 0 right, 1 down, 2 left, 3 up */
    int speed;                   /* world pixels per second */
    int health;                  /* points */
    char sprite[TABLE_PATH_MAX]; /* the art file it draws */
    const Sprite *image;         /* that art, loaded at startup */

    /* Lesson 087: the format's named columns. Every field below takes
       its default when a file's header does not name it — the defaults
       are the format's contract, not a gap in it. */
    int accel;                   /* ms: the eased-move time constant —
                                    the feel (the hero's weight; a
                                    particle's settle takes this long,
                                    lesson 093) */
    int damage;                  /* points a hit removes — a weapon
                                    row's damage, carried by its shots */
    int rate;                    /* rounds per minute; 0 = never fires */
    char fires[TABLE_NAME_MAX];  /* the projectile kind this row fires */
    int range;                   /* world pixels: a projectile's flight
                                    budget — its life (a particle's
                                    settle distance, lesson 093) */
    int behavior;                /* BehaviorKind, its row's */
    int wave;                    /* the wave this kind joins — it spawns
                                    in that wave and every wave after
                                    (lesson 091); 0 = never by wave */
    int count;                   /* how many of this kind join the wave */
};

/* Lesson 087: the defaults the format defines. A file may omit any
   column; the field takes the value named here. The accel default is
   the feel lesson 085 shipped as a constant — the hero's row predates
   the column and keeps loading byte-for-byte, and its weight is exactly
   this. */
constexpr int TABLE_ACCEL_DEFAULT = 120; /* ms */

/* A behavior value's spelling (and back), for the format's own reports. */
const char *BehaviorName(int behavior);

/* A loaded table: one definition per row, in the arena — as many rows as
   the file has, and not one more. */
struct EntityTable {
    EntityDef *rows;
    int count;
};

/* A load either hands over a complete table or names what went wrong —
   never a partial table presented as success. These are the loaders'
   failures, the same three every asset in this engine answers with. */
enum TableError {
    TABLE_OK = 0,
    TABLE_MISSING,   /* the file is not there or cannot be read */
    TABLE_MALFORMED, /* the bytes are not a complete table in the format */
    TABLE_NO_ROOM,   /* the arena had no room for the rows */
};

struct TableResult {
    EntityTable table;
    TableError error; /* TABLE_OK exactly when the table is complete */
};

/* Loads an entity table from a file read whole. The header and the rows
   are parsed byte by byte — no library reads it — and anything the format
   does not describe is refused typed: a column it does not know, a column
   named twice, a row with the wrong number of values (exactly as many as
   the header names), a value where a number is required, a value where
   text is, a behavior the format does not define, a name the table
   already holds. The rows are copied into the arena behind a mark, and
   every refusal path rolls back to it: a load that refuses leaves
   nothing behind. (Lesson 087: a column the header does *not* name is
   no longer a refusal — its field takes the format's default.) */
TableResult LoadTable(Arena &arena, const char *path);

/* Lesson 073: a definition lookup — the request the game makes when it
   wants an entity of a kind. The table either answers with the
   definition or names what is missing: a definition the table does not
   hold is a typed failure, never a row with assumed attributes. */
enum DefError {
    DEF_OK = 0,
    DEF_UNKNOWN, /* the table holds no definition by that name */
};

struct DefResult {
    const EntityDef *def; /* the definition, or 0 */
    DefError error;       /* DEF_OK exactly when def is non-0 */
};

/* The definition named `name`, or the failure that says the table does
   not hold one. */
DefResult TableFind(const EntityTable &table, const char *name);

} /* namespace engine */

#endif
