// wordcount.c — count lines, words, bytes, and the longest line in every
// file named on the command line.
//
// Lesson 003: char buffers — strings by hand.
#include <ctype.h>
#include <stdio.h>

struct Counts {
    unsigned long lines, words, bytes, longest;
};

// LineLen is this program's own strlen: it walks a NUL-terminated string
// and returns its length in bytes.
static unsigned long LineLen(const char *s)
{
    unsigned long n = 0;
    while (s[n] != '\0')
        ++n;
    return n;
}

// CountStream reads f to EOF and accumulates counts into *out. Each line is
// collected in a fixed buffer, so the longest line it can report is 255
// bytes; lesson 004 lifts that ceiling.
static void CountStream(FILE *f, struct Counts *out)
{
    char line[256];
    unsigned long len = 0;
    int in_word = 0;
    int c;

    while ((c = fgetc(f)) != EOF) {
        ++out->bytes;
        if (c == '\n') {
            line[len] = '\0';
            unsigned long line_len = LineLen(line);
            if (line_len > out->longest)
                out->longest = line_len;
            ++out->lines;
            len = 0;
            in_word = 0;
        } else {
            if (len < sizeof line - 1)
                line[len++] = (char)c;
            if (isspace(c)) {
                in_word = 0;
            } else if (!in_word) {
                in_word = 1;
                ++out->words;
            }
        }
    }

    if (len > 0) {
        line[len] = '\0';
        unsigned long line_len = LineLen(line);
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
