// tilemap.cpp — the map file's reader: line by line, complete or nothing.
//
// Lesson 052: the format is three counts, a kind table, and one character
// per cell — small enough that every rule here can be checked against the
// file with your own eyes.

#include "tilemap.h"

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

/* One decimal number, separated from its neighbors by spaces. */
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

} /* namespace */

TileResult LoadTileMap(Arena &arena, const char *path)
{
    TileResult result = { { 0, 0, 0, { { 0, 0 } }, 0 }, TILE_OK };

    platform::FileData file = platform::ReadFile(path);
    if (file.error != platform::FILE_OK) {
        result.error = TILE_MISSING;
        return result;
    }

    Lines lines = { file.data, file.size, 0 };
    const unsigned char *line = 0;
    int len = 0;
    bool ok = true;

    /* The first line: width height kind-count — the file's only numbers,
       and the counts every later line is checked against. */
    int width = 0, height = 0, kind_count = 0;
    ok = ok && NextLine(lines, line, len);
    int at = 0;
    ok = ok && ReadInt(line, len, at, width);
    ok = ok && ReadInt(line, len, at, height);
    ok = ok && ReadInt(line, len, at, kind_count);
    ok = ok && at == len; /* nothing else on the line */
    ok = ok && width > 0 && width <= TILE_MAX_DIM;
    ok = ok && height > 0 && height <= TILE_MAX_DIM;
    ok = ok && kind_count > 0 && kind_count <= TILE_MAX_KINDS;

    /* The kind table: one character and its solidity per kind, in order. */
    TileKind kinds[TILE_MAX_KINDS];
    for (int k = 0; ok && k < kind_count; ++k) {
        ok = ok && NextLine(lines, line, len);
        ok = ok && len >= 3;
        if (ok) {
            kinds[k].cell = (char)line[0];
            int solid = -1;
            int at2 = 1;
            ok = ok && ReadInt(line, len, at2, solid);
            ok = ok && (solid == 0 || solid == 1);
            ok = ok && at2 == len;
            kinds[k].solid = (unsigned char)solid;
        }
        for (int prev = 0; ok && prev < k; ++prev)
            if (kinds[prev].cell == kinds[k].cell)
                ok = false; /* one character, one kind */
    }

    /* The cells: exactly height rows of exactly width characters, every
       character one the kind table names. */
    size_t mark = ArenaMark(arena);
    unsigned char *cells = 0;
    if (ok) {
        cells = (unsigned char *)ArenaAlloc(arena, (size_t)width * height, 1);
        if (!cells) {
            ArenaRollback(arena, mark);
            platform::ReleaseFile(file);
            result.error = TILE_NO_ROOM;
            return result;
        }
    }
    for (int y = 0; ok && y < height; ++y) {
        ok = ok && NextLine(lines, line, len);
        ok = ok && len == width;
        for (int x = 0; ok && x < width; ++x) {
            int kind = -1;
            for (int k = 0; k < kind_count; ++k)
                if (kinds[k].cell == (char)line[x]) {
                    kind = k;
                    break;
                }
            if (kind < 0)
                ok = false; /* a character no kind claims */
            else
                cells[y * width + x] = (unsigned char)kind;
        }
    }

    /* After the last row: trailing blank lines, and nothing else. */
    while (ok && lines.at < lines.size) {
        ok = ok && NextLine(lines, line, len);
        ok = ok && len == 0;
    }

    if (!ok) {
        ArenaRollback(arena, mark);
        platform::ReleaseFile(file);
        result.error = TILE_MALFORMED;
        return result;
    }

    platform::ReleaseFile(file);
    result.map.width = width;
    result.map.height = height;
    result.map.kind_count = kind_count;
    for (int k = 0; k < kind_count; ++k)
        result.map.kinds[k] = kinds[k];
    result.map.cells = cells;
    result.error = TILE_OK;
    return result;
}

int TileAt(const TileMap &map, int x, int y)
{
    if (x < 0 || x >= map.width || y < 0 || y >= map.height)
        return -1; /* outside the map: the defined answer, not a read */
    return map.cells[y * map.width + x];
}

bool TileSolid(const TileMap &map, int x, int y)
{
    int kind = TileAt(map, x, y);
    if (kind < 0)
        return true; /* outside the map: the edge blocks like a wall */
    return map.kinds[kind].solid != 0;
}

bool TilePointSolid(const TileMap &map, int world_x, int world_y)
{
    if (world_x < 0 || world_y < 0 ||
        world_x >= map.width * TILE_SIZE ||
        world_y >= map.height * TILE_SIZE)
        return true; /* out of bounds: the policy's answer, no cell read */
    return TileSolid(map, world_x / TILE_SIZE, world_y / TILE_SIZE);
}

bool TileRectSolid(const TileMap &map, int x, int y, int w, int h)
{
    if (w <= 0 || h <= 0)
        return false; /* an empty rectangle overlaps nothing */

    /* The map's edge is solid: a rectangle that leaves the map answers
       without reading a single cell. */
    if (x < 0 || y < 0 || x + w > map.width * TILE_SIZE ||
        y + h > map.height * TILE_SIZE)
        return true;

    /* Otherwise only the cells the rectangle covers can say yes. */
    int cx0 = x / TILE_SIZE;
    int cy0 = y / TILE_SIZE;
    int cx1 = (x + w - 1) / TILE_SIZE;
    int cy1 = (y + h - 1) / TILE_SIZE;
    for (int cy = cy0; cy <= cy1; ++cy)
        for (int cx = cx0; cx <= cx1; ++cx)
            if (map.kinds[map.cells[cy * map.width + cx]].solid)
                return true;
    return false;
}

} /* namespace engine */
