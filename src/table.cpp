// table.cpp — the table file's reader: rows of definitions, complete or
// nothing.
//
// Lesson 071: the format's grammar is a paragraph — a header naming the
// columns, then one row per definition — and this file walks it byte by
// byte like map.txt's reader does. Every value is checked against the
// column that claims it: a whole number where a number belongs, a run of
// non-space bytes where text belongs, and exactly as many values as the
// header names.
//
// Lesson 072: the rows live in the arena. How many there are is the
// file's fact, so the file is walked once to count them and once to fill
// them, and the whole load is bracketed by a mark — a refused load rolls
// the arena back and keeps nothing.
//
// Lesson 087: the format grows by named columns, additively. The header
// names the columns a file uses — any subset of the ones below — and the
// fill starts every row at the format's defaults (DefaultRow), writing
// only the named fields. A file that omits a column is no longer
// refused; its fields sit at their defaults. A column the format does
// not know is still malformed.

#include "table.h"

#include "platform.h"

namespace engine {
namespace {

/* Line-oriented parsing over the file's bytes: the format is lines, so
   the reader is lines. */
struct Lines {
    const unsigned char *data;
    size_t size;
    size_t at; /* the start of the current line */
};

bool NextLine(Lines &lines, const unsigned char *&line, int &len)
{
    if (lines.at >= lines.size)
        return false;
    size_t start = lines.at;
    while (lines.at < lines.size && lines.data[lines.at] != '\n')
        ++lines.at;
    len = (int)(lines.at - start);
    line = lines.data + start;
    if (lines.at < lines.size)
        ++lines.at; /* consume the newline */
    return true;
}

/* One whitespace-separated value, exactly as long as the file has it. */
bool NextToken(const unsigned char *line, int len, int &at,
               const unsigned char *&token, int &token_len)
{
    while (at < len && (line[at] == ' ' || line[at] == '\t'))
        ++at;
    if (at >= len)
        return false;
    token = line + at;
    while (at < len && line[at] != ' ' && line[at] != '\t')
        ++at;
    token_len = (int)(line + at - token);
    return true;
}

/* One whole number: digits, separated from its neighbors by spaces. The
   bound is the number reader's, not the format's — a number is refused
   before it can grow past what this arithmetic holds. */
bool ReadInt(const unsigned char *line, int len, int &at, int &out)
{
    while (at < len && (line[at] == ' ' || line[at] == '\t'))
        ++at;
    if (at >= len || line[at] < '0' || line[at] > '9')
        return false;
    int value = 0;
    while (at < len && line[at] >= '0' && line[at] <= '9') {
        value = value * 10 + (line[at] - '0');
        if (value > 1000000)
            return false;
        ++at;
    }
    out = value;
    return true;
}

/* One text value: a run of non-space bytes, copied into a field no wider
   than out_max. A value that does not fit is refused, never truncated —
   a truncated name is a different name, and a truncated path is a
   different file. */
bool ReadText(const unsigned char *line, int len, int &at, char *out,
              int out_max)
{
    while (at < len && (line[at] == ' ' || line[at] == '\t'))
        ++at;
    size_t start = (size_t)at;
    while (at < len && line[at] != ' ' && line[at] != '\t')
        ++at;
    int width = (int)((size_t)at - start);
    if (width == 0 || width >= out_max)
        return false;
    for (int i = 0; i < width; ++i)
        out[i] = (char)line[start + i];
    out[width] = 0;
    return true;
}

/* The columns the format knows (lesson 087: grown by named columns). */
enum Column {
    COL_NAME,
    COL_X,
    COL_Y,
    COL_FACING,
    COL_SPEED,
    COL_HEALTH,
    COL_SPRITE,
    COL_ACCEL,
    COL_DAMAGE,
    COL_RATE,
    COL_FIRES,
    COL_RANGE,
    COL_BEHAVIOR,
    COL_WAVE,
    COL_SPAWN_COUNT,
    COL_COUNT /* how many columns the format knows, not a column */
};

const char *const COLUMN_NAMES[COL_COUNT] = {
    "name", "x", "y", "facing", "speed", "health", "sprite",
    "accel", "damage", "rate", "fires", "range", "behavior", "wave",
    "count"
};

/* Lesson 087: the behavior column's spellings, the format's own. */
const char *const BEHAVIOR_NAMES[BEHAVIOR_COUNT] = {
    "none", "fly", "chase", "keep", "flee", "boss", "settle"
};

/* Lesson 087: one row at the format's defaults, before the named fields
   are written. A file may omit any column; whatever it omits keeps the
   value set here — the defaults are part of the format's contract. */
void DefaultRow(EntityDef &def)
{
    for (int i = 0; i < TABLE_NAME_MAX; ++i) {
        def.name[i] = 0;
        def.fires[i] = 0;
    }
    for (int i = 0; i < TABLE_PATH_MAX; ++i)
        def.sprite[i] = 0;
    def.x = 0;
    def.y = 0;
    def.facing = 0;
    def.speed = 0;
    def.health = 0;
    def.image = 0;
    def.accel = TABLE_ACCEL_DEFAULT;
    def.damage = 0;
    def.rate = 0;
    def.range = 0;
    def.behavior = BEHAVIOR_NONE;
    def.wave = 0;
    def.count = 1;
}

bool TokenIs(const unsigned char *token, int token_len, const char *name)
{
    int n = 0;
    while (name[n])
        ++n;
    if (n != token_len)
        return false;
    for (int i = 0; i < n; ++i)
        if (name[i] != (char)token[i])
            return false;
    return true;
}

/* A text field against a text field: the same bytes, or not. */
bool SameText(const char *a, const char *b)
{
    int i = 0;
    while (a[i] && a[i] == b[i])
        ++i;
    return a[i] == b[i];
}

int FindColumn(const unsigned char *token, int token_len)
{
    for (int c = 0; c < COL_COUNT; ++c)
        if (TokenIs(token, token_len, COLUMN_NAMES[c]))
            return c;
    return -1;
}

/* Lesson 087: a behavior value — one of the format's spellings, mapped
   to its number. A spelling the format does not define is refused, like
   a facing that is not one of the four. */
bool ReadBehavior(const unsigned char *line, int len, int &at, int &out)
{
    char text[TABLE_NAME_MAX];
    if (!ReadText(line, len, at, text, TABLE_NAME_MAX))
        return false;
    for (int b = 0; b < BEHAVIOR_COUNT; ++b)
        if (SameText(text, BEHAVIOR_NAMES[b])) {
            out = b;
            return true;
        }
    return false;
}

} /* namespace */

const char *BehaviorName(int behavior)
{
    if (behavior < 0 || behavior >= BEHAVIOR_COUNT)
        return "?";
    return BEHAVIOR_NAMES[behavior];
}

TableResult LoadTable(Arena &arena, const char *path)
{
    TableResult result = {};

    platform::FileData file = platform::ReadFile(path);
    if (file.error != platform::FILE_OK) {
        result.error = TABLE_MISSING;
        return result;
    }

    /* The first walk: the header's line, then the rows, counted. How many
       rows a table holds is the file's fact — never a capacity the engine
       picked — so the count comes first and the arena is asked for
       exactly that many rows. The rows end at the first blank line; what
       follows them is checked against the format in the second walk. */
    Lines lines = { file.data, file.size, 0 };
    const unsigned char *line = 0;
    int len = 0;
    bool ok = NextLine(lines, line, len); /* the header's line */
    int rows = 0;
    while (ok && NextLine(lines, line, len)) {
        if (len == 0)
            break;
        rows += 1;
    }

    /* The rows, into the arena. The mark is the load's transaction: from
       here on a refusal rolls the arena back, and a refused load leaves
       no partial rows behind — the used count does not move. */
    size_t mark = ArenaMark(arena);
    EntityDef *defs = 0;
    if (ok && rows > 0) {
        defs = (EntityDef *)ArenaAlloc(arena, (size_t)rows * sizeof(EntityDef),
                                       4);
        if (!defs) {
            ArenaRollback(arena, mark);
            platform::ReleaseFile(file);
            result.error = TABLE_NO_ROOM;
            return result;
        }
    }

    /* The second walk: the header and the rows, byte by byte — every
       value landing in the field its column names. A row is refused — the
       whole file is — when a value is missing or one too many, when a
       value is not what its column requires, when the facing is not one
       of the four the format defines, or when the name is one the table
       already holds. The fill never writes past the count the first walk
       promised. */
    lines.at = 0;
    ok = ok && NextLine(lines, line, len);
    int at = 0;
    int order[COL_COUNT];
    int named = 0; /* lesson 087: how many columns this file names */
    for (int c = 0; c < COL_COUNT; ++c)
        order[c] = -1;
    /* Lesson 087: the header names the columns this file's rows carry —
       any subset of the format's, each at most once, at least one. A
       column the header does not name is not a refusal any more: the
       field sits at the format's default. A name the format does not
       know is still refused here, and so is a name said twice. */
    while (ok) {
        const unsigned char *token = 0;
        int token_len = 0;
        if (!NextToken(line, len, at, token, token_len))
            break; /* the header's line ends */
        int column = FindColumn(token, token_len);
        ok = ok && column >= 0;
        for (int prev = 0; ok && prev < named; ++prev)
            ok = ok && order[prev] != column; /* one name, one column */
        if (ok) {
            order[named] = column;
            named += 1;
        }
    }
    ok = ok && named > 0;

    while (ok) {
        if (!NextLine(lines, line, len))
            break;
        if (len == 0)
            break; /* the rows end here; the tail is checked below */
        if (result.table.count >= rows) {
            ok = false; /* the fill never writes past the count's promise */
            break;
        }

        EntityDef &def = defs[result.table.count];
        /* Lesson 087: the row begins at the format's defaults. The named
           fields below overwrite theirs; every field the header does not
           name keeps its default. */
        DefaultRow(def);
        at = 0;
        for (int i = 0; ok && i < named; ++i) {
            switch (order[i]) {
            case COL_NAME:
                ok = ReadText(line, len, at, def.name, TABLE_NAME_MAX);
                break;
            case COL_X:
                ok = ReadInt(line, len, at, def.x);
                break;
            case COL_Y:
                ok = ReadInt(line, len, at, def.y);
                break;
            case COL_FACING:
                ok = ReadInt(line, len, at, def.facing);
                ok = ok && def.facing <= 3;
                break;
            case COL_SPEED:
                ok = ReadInt(line, len, at, def.speed);
                break;
            case COL_HEALTH:
                ok = ReadInt(line, len, at, def.health);
                break;
            case COL_SPRITE:
                ok = ReadText(line, len, at, def.sprite, TABLE_PATH_MAX);
                break;
            case COL_ACCEL:
                ok = ReadInt(line, len, at, def.accel);
                break;
            case COL_DAMAGE:
                ok = ReadInt(line, len, at, def.damage);
                break;
            case COL_RATE:
                ok = ReadInt(line, len, at, def.rate);
                break;
            case COL_FIRES:
                ok = ReadText(line, len, at, def.fires, TABLE_NAME_MAX);
                break;
            case COL_RANGE:
                ok = ReadInt(line, len, at, def.range);
                break;
            case COL_BEHAVIOR:
                ok = ReadBehavior(line, len, at, def.behavior);
                break;
            case COL_WAVE:
                ok = ReadInt(line, len, at, def.wave);
                break;
            case COL_SPAWN_COUNT:
                ok = ReadInt(line, len, at, def.count);
                break;
            default:
                ok = false;
                break;
            }
        }
        while (ok && at < len && (line[at] == ' ' || line[at] == '\t'))
            ++at;
        ok = ok && at == len; /* nothing else on the line */
        for (int prev = 0; ok && prev < result.table.count; ++prev)
            ok = ok && !SameText(defs[prev].name, def.name);
        if (ok)
            result.table.count += 1;
    }

    /* After the last row: trailing blank lines, and nothing else. A file
       that holds a header and no rows is not a table — the load is a
       complete table or a typed failure. */
    while (ok && lines.at < lines.size) {
        ok = ok && NextLine(lines, line, len);
        ok = ok && len == 0;
    }
    ok = ok && result.table.count > 0;

    platform::ReleaseFile(file);
    if (!ok) {
        ArenaRollback(arena, mark);
        result.table.count = 0;
        result.error = TABLE_MALFORMED;
        return result;
    }

    result.table.rows = defs;
    result.error = TABLE_OK;
    return result;
}

DefResult TableFind(const EntityTable &table, const char *name)
{
    DefResult result = { 0, DEF_OK };
    for (int i = 0; i < table.count; ++i)
        if (SameText(table.rows[i].name, name)) {
            result.def = &table.rows[i];
            return result;
        }
    result.error = DEF_UNKNOWN;
    return result;
}

} /* namespace engine */
