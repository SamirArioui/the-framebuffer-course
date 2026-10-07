// table.cpp — the table file's reader: rows of definitions, complete or
// nothing.
//
// Lesson 071: the format's grammar is a paragraph — a header naming the
// columns, then one row per definition — and this file walks it byte by
// byte like map.txt's reader does. Every value is checked against the
// column that claims it: a whole number where a number belongs, a run of
// non-space bytes where text belongs, and exactly as many values as the
// header names.

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

/* The columns the format knows. */
enum Column {
    COL_NAME,
    COL_X,
    COL_Y,
    COL_FACING,
    COL_SPEED,
    COL_HEALTH,
    COL_SPRITE,
    COL_COUNT
};

const char *const COLUMN_NAMES[COL_COUNT] = {
    "name", "x", "y", "facing", "speed", "health", "sprite"
};

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

} /* namespace */

TableResult LoadTable(const char *path)
{
    TableResult result = {};

    platform::FileData file = platform::ReadFile(path);
    if (file.error != platform::FILE_OK) {
        result.error = TABLE_MISSING;
        return result;
    }

    Lines lines = { file.data, file.size, 0 };
    const unsigned char *line = 0;
    int len = 0;
    bool ok = true;
    TableError failure = TABLE_MALFORMED;

    /* The header: the columns this file's rows carry, named one after
       another. The order is the file's — the loader fills the fields the
       header declares — but every column the format knows is named, and
       named once. A name the format does not know is refused here rather
       than read as something else later. */
    int order[COL_COUNT];
    for (int c = 0; c < COL_COUNT; ++c)
        order[c] = -1;
    ok = ok && NextLine(lines, line, len);
    int at = 0;
    for (int i = 0; ok && i < COL_COUNT; ++i) {
        const unsigned char *token = 0;
        int token_len = 0;
        ok = ok && NextToken(line, len, at, token, token_len);
        if (ok) {
            int column = FindColumn(token, token_len);
            ok = ok && column >= 0;
            for (int prev = 0; ok && prev < i; ++prev)
                ok = ok && order[prev] != column; /* one name, one column */
            order[i] = column;
        }
    }
    const unsigned char *extra = 0;
    int extra_len = 0;
    ok = ok && !NextToken(line, len, at, extra, extra_len);

    /* The rows: one definition each, every value landing in the field
       its column names. The row is refused — the whole file is — when a
       value is missing or one too many, when a value is not what its
       column requires, when the facing is not one of the four the format
       defines, or when the name is one the table already holds (one
       name, one definition — the map's kind table has the same rule). */
    while (ok) {
        if (!NextLine(lines, line, len))
            break;
        if (len == 0)
            break; /* the rows end here; the tail is checked below */
        if (result.table.count >= TABLE_MAX_ROWS) {
            failure = TABLE_FULL;
            ok = false;
            break;
        }

        EntityDef &def = result.table.rows[result.table.count];
        at = 0;
        for (int i = 0; ok && i < COL_COUNT; ++i) {
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
            default:
                ok = false;
                break;
            }
        }
        while (ok && at < len && (line[at] == ' ' || line[at] == '\t'))
            ++at;
        ok = ok && at == len; /* nothing else on the line */
        for (int prev = 0; ok && prev < result.table.count; ++prev)
            ok = ok && !SameText(result.table.rows[prev].name, def.name);
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
        result.table.count = 0;
        result.error = failure;
        return result;
    }

    result.error = TABLE_OK;
    return result;
}

} /* namespace engine */
