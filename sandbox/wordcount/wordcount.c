// wordcount.c — count lines, words, bytes, and the longest line in every
// file named on the command line.
//
// Lesson 004: malloc and free — growing buffers on the heap.
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

struct Counts {
    unsigned long lines, words, bytes, longest;
};

// Buffer is a growable byte buffer. data points at len bytes of useful
// content with room for cap bytes in total.
struct Buffer {
    char *data;
    size_t len, cap;
};

// BufferInit prepares an empty buffer. The first push allocates.
//
// NOTE: nothing in this lesson ever frees the buffer's memory. We are
// leaking this on purpose; lesson 005 makes it visible.
static void BufferInit(struct Buffer *buf)
{
    buf->data = NULL;
    buf->len = 0;
    buf->cap = 0;
}

// BufferGrow makes room for more bytes, doubling the capacity each time so
// that pushing N bytes costs O(log N) reallocations instead of N.
static void BufferGrow(struct Buffer *buf)
{
    size_t new_cap = buf->cap == 0 ? 64 : buf->cap * 2;
    buf->data = realloc(buf->data, new_cap);
    buf->cap = new_cap;
}

// BufferPush appends one byte, growing first if the buffer is full.
static void BufferPush(struct Buffer *buf, char c)
{
    if (buf->len == buf->cap)
        BufferGrow(buf);
    buf->data[buf->len++] = c;
}

// LineLen is this program's own strlen: it walks a NUL-terminated string
// and returns its length in bytes.
static unsigned long LineLen(const char *s)
{
    unsigned long n = 0;
    while (s[n] != '\0')
        ++n;
    return n;
}

// CountStream reads f to EOF and accumulates counts into *out. The line
// buffer lives on the heap now, so lines of any length are measured truly.
static void CountStream(FILE *f, struct Counts *out)
{
    struct Buffer line;
    BufferInit(&line);
    int in_word = 0;
    int c;

    while ((c = fgetc(f)) != EOF) {
        ++out->bytes;
        if (c == '\n') {
            BufferPush(&line, '\0');
            unsigned long line_len = LineLen(line.data);
            if (line_len > out->longest)
                out->longest = line_len;
            ++out->lines;
            line.len = 0;
            in_word = 0;
        } else {
            BufferPush(&line, (char)c);
            if (isspace(c)) {
                in_word = 0;
            } else if (!in_word) {
                in_word = 1;
                ++out->words;
            }
        }
    }

    if (line.len > 0) {
        BufferPush(&line, '\0');
        unsigned long line_len = LineLen(line.data);
        if (line_len > out->longest)
            out->longest = line_len;
    }
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "usage: %s FILE...\n", argv[0]);
        return 1;
    }

    for (int i = 1; i < argc; ++i) {
        FILE *f = fopen(argv[i], "rb");
        if (f == NULL) {
            fprintf(stderr, "%s: cannot open %s\n", argv[0], argv[i]);
            continue;
        }

        struct Counts counts = {0};
        CountStream(f, &counts);

        printf("%lu %lu %lu %lu %s\n", counts.lines, counts.words,
               counts.bytes, counts.longest, argv[i]);
        fclose(f);
    }

    return 0;
}
