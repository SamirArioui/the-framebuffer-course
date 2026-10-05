// hashtable.h — the hashtable: hashed string keys, chained buckets.
//
// Lesson 012: a header is a contract. Note what is NOT here: dynarray.h.
// The header only mentions struct DynArray through a pointer, so a forward
// declaration is enough — callers that need the full type include
// dynarray.h themselves.
#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <stddef.h> // size_t

struct DynArray; // incomplete type: pointers to it are all this file needs

struct Item {
    char key[16];
    long value;
};

struct HashTable {
    struct DynArray *chains;
    size_t nbuckets;
    size_t len;
};

void HtInit(struct HashTable *ht, size_t nbuckets);
long *HtGet(const struct HashTable *ht, const char *key);
void HtPut(struct HashTable *ht, const char *key, long value);
void HtEntries(const struct HashTable *ht, struct DynArray *out);
void HtFree(struct HashTable *ht);

#endif // HASHTABLE_H
