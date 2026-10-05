// hashtable.c — the hashtable's implementation: FNV-1a hashing, bucket
// lookup, chained collision handling over the kit's own dynarray.
//
// Lesson 012: one translation unit. static keeps HtHash and HtLookup
// private to this file — internal linkage, invisible to the linker.
#include "hashtable.h"
#include "dynarray.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

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
