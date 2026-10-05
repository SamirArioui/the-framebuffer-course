// wordcount.c — count bytes in every file named on the command line.
//
// Lesson 001: argv, file input, and the first gcc command.
#include <stdio.h>

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

        unsigned long bytes = 0;
        int c;
        while ((c = fgetc(f)) != EOF)
            ++bytes;

        printf("%lu %s\n", bytes, argv[i]);
        fclose(f);
    }

    return 0;
}
