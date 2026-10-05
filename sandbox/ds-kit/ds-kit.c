// ds-kit.c — the data-structures kit of Part 0: a hashtable over the
// generic dynarray, and a word-frequency driver.
//
// Lesson 011: hashing, buckets, collisions — and lookup that is O(1) on
// average.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>

struct Item {
    char key[16];
    long value;
};

struct DynArray {
    void *data;
    size_t len;
    size_t cap;
    size_t elem_size;
};

void DaInit(struct DynArray *da, size_t elem_size)
{
    da->data = NULL;
    da->len = 0;
    da->cap = 0;
    da->elem_size = elem_size;
}

void DaPush(struct DynArray *da, const void *elem)
{
    if (da->len == da->cap) {
        size_t newcap = da->cap ? da->cap * 2 : 4;
        void *p = realloc(da->data, newcap * da->elem_size);
        if (p == NULL) {
            fprintf(stderr, "DaPush: out of memory\n");
            exit(1);
        }
        da->data = p;
        da->cap = newcap;
    }
    // Byte arithmetic on purpose: void* has no element size to step by.
    char *slot = (char *)da->data + da->len * da->elem_size;
    memcpy(slot, elem, da->elem_size);
    ++da->len;
}

void *DaAt(struct DynArray *da, size_t i)
{
    char *base = da->data;
    return base + i * da->elem_size;
}

void DaSort(struct DynArray *da, int (*cmp)(const void *, const void *))
{
    qsort(da->data, da->len, da->elem_size, cmp);
}

void DaEach(const struct DynArray *da, void (*visit)(const void *))
{
    const char *base = da->data;
    for (size_t i = 0; i < da->len; ++i)
        visit(base + i * da->elem_size);
}

void DaFree(struct DynArray *da)
{
    free(da->data);
    DaInit(da, da->elem_size);
}

// -- the hashtable ------------------------------------------------------
//
// Buckets are an array of chains, and a chain is just a dynarray of
// struct Item. FNV-1a turns a key into a 32-bit hash; the remainder modulo
// nbuckets picks the bucket; the chain handles collisions.

struct HashTable {
    struct DynArray *chains;
    size_t nbuckets;
    size_t len;
};

static uint32_t HtHash(const char *s)
{
    uint32_t h = 2166136261u; // FNV-1a 32-bit offset basis
    while (*s != '\0') {
        h ^= (unsigned char)*s++;
        h *= 16777619u; // FNV-1a 32-bit prime
    }
    return h;
}

void HtInit(struct HashTable *ht, size_t nbuckets)
{
    ht->chains = malloc(nbuckets * sizeof *ht->chains);
    if (ht->chains == NULL) {
        fprintf(stderr, "HtInit: out of memory\n");
        exit(1);
    }
    ht->nbuckets = nbuckets;
    ht->len = 0;
    for (size_t b = 0; b < nbuckets; ++b)
        DaInit(&ht->chains[b], sizeof(struct Item));
}

static struct Item *HtLookup(const struct HashTable *ht, const char *key)
{
    size_t b = HtHash(key) % ht->nbuckets;
    struct DynArray *chain = &ht->chains[b];
    for (size_t i = 0; i < chain->len; ++i) {
        struct Item *it = (struct Item *)DaAt(chain, i);
        if (strcmp(it->key, key) == 0)
            return it;
    }
    return NULL;
}

long *HtGet(const struct HashTable *ht, const char *key)
{
    struct Item *it = HtLookup(ht, key);
    return it != NULL ? &it->value : NULL;
}

void HtPut(struct HashTable *ht, const char *key, long value)
{
    struct Item *it = HtLookup(ht, key);
    if (it != NULL) {
        it->value = value;
        return;
    }
    struct Item item;
    snprintf(item.key, sizeof item.key, "%s", key); // keys longer than 15 are truncated
    item.value = value;
    size_t b = HtHash(key) % ht->nbuckets;
    DaPush(&ht->chains[b], &item);
    ++ht->len;
}

void HtEntries(const struct HashTable *ht, struct DynArray *out)
{
    for (size_t b = 0; b < ht->nbuckets; ++b) {
        struct DynArray *chain = &ht->chains[b];
        for (size_t i = 0; i < chain->len; ++i)
            DaPush(out, DaAt(chain, i));
    }
}

void HtFree(struct HashTable *ht)
{
    for (size_t b = 0; b < ht->nbuckets; ++b)
        DaFree(&ht->chains[b]);
    free(ht->chains);
    ht->chains = NULL;
    ht->nbuckets = 0;
    ht->len = 0;
}

// -- the driver: word frequencies ---------------------------------------

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
