// ds-kit.c — the data-structures kit of Part 0: one generic dynarray that
// stores any element type as raw bytes.
//
// Lesson 010: void* — genericity, casting, and its silent failures.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

// -- the callbacks this driver supplies --------------------------------
//
// Every one of them casts: the array is generic now, so the types are the
// caller's job.

static int CmpByKey(const void *pa, const void *pb)
{
    const struct Item *a = (const struct Item *)pa;
    const struct Item *b = (const struct Item *)pb;
    return strcmp(a->key, b->key);
}

static int CmpLong(const void *pa, const void *pb)
{
    const long *a = (const long *)pa;
    const long *b = (const long *)pb;
    return (*a > *b) - (*a < *b);
}

static void PrintItem(const void *pe)
{
    const struct Item *it = (const struct Item *)pe;
    printf("%s %ld\n", it->key, it->value);
}

static void PrintLong(const void *pe)
{
    const long *v = (const long *)pe;
    printf("%ld\n", *v);
}

int main(void)
{
    struct DynArray items;
    DaInit(&items, sizeof(struct Item));

    const char *keys[10] = {
        "pear", "apple", "fig", "banana", "cherry",
        "date", "elder", "grape", "kiwi", "lemon",
    };
    for (int i = 0; i < 10; ++i) {
        struct Item item;
        snprintf(item.key, sizeof item.key, "%s", keys[i]);
        item.value = i;
        DaPush(&items, &item);
    }

    DaSort(&items, CmpByKey);
    printf("items sorted by key:\n");
    DaEach(&items, PrintItem);

    struct DynArray nums;
    DaInit(&nums, sizeof(long));
    long vals[5] = { 50, 30, 10, 40, 20 };
    for (int i = 0; i < 5; ++i)
        DaPush(&nums, &vals[i]);

    DaSort(&nums, CmpLong);
    printf("numbers sorted:\n");
    DaEach(&nums, PrintLong);

    DaFree(&nums);
    DaFree(&items);
    return 0;
}
