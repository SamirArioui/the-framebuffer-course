// main.c — the driver: word frequencies over the file named on argv,
// printed sorted by key.
//
// Lesson 012: the program itself. This is the only file with main.
#include "dynarray.h"
#include "hashtable.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static int CmpByKey(const void *pa, const void *pb)
{
    const struct Item *a = (const struct Item *)pa;
    const struct Item *b = (const struct Item *)pb;
    return strcmp(a->key, b->key);
}

static void PrintItem(const void *pe)
{
    const struct Item *it = (const struct Item *)pe;
    printf("%s %ld\n", it->key, it->value);
}

static void CountWord(struct HashTable *ht, const char *word)
{
    long *p = HtGet(ht, word);
    if (p != NULL)
        ++*p;
    else
        HtPut(ht, word, 1);
}

static void CountWords(struct HashTable *ht, FILE *f)
{
    char word[16];
    size_t n = 0;
    int c;
    while ((c = fgetc(f)) != EOF) {
        if (isalnum((unsigned char)c)) {
            if (n + 1 < sizeof word)
                word[n++] = (char)tolower((unsigned char)c);
        } else if (n > 0) {
            word[n] = '\0';
            CountWord(ht, word);
            n = 0;
        }
    }
    if (n > 0) {
        word[n] = '\0';
        CountWord(ht, word);
    }
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "usage: %s FILE\n", argv[0]);
        return 1;
    }
    FILE *f = fopen(argv[1], "rb");
    if (f == NULL) {
        fprintf(stderr, "%s: cannot open %s\n", argv[0], argv[1]);
        return 1;
    }

    struct HashTable ht;
    HtInit(&ht, 1024);
    CountWords(&ht, f);
    fclose(f);

    struct DynArray entries;
    DaInit(&entries, sizeof(struct Item));
    HtEntries(&ht, &entries);
    DaSort(&entries, CmpByKey);
    DaEach(&entries, PrintItem);

    DaFree(&entries);
    HtFree(&ht);
    return 0;
}
