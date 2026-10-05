// ds-kit.c — the data-structures kit of Part 0: a dynarray of Items that
// grows by realloc, sorts by comparator, and visits by hook.
//
// Lesson 009: function pointers — comparators and callbacks.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Item {
    char key[16];
    long value;
};

struct DynArray {
    struct Item *items;
    size_t len;
    size_t cap;
};

void DaInit(struct DynArray *da)
{
    da->items = NULL;
    da->len = 0;
    da->cap = 0;
}

void DaPush(struct DynArray *da, struct Item item)
{
    if (da->len == da->cap) {
        size_t newcap = da->cap ? da->cap * 2 : 4;
        struct Item *p = realloc(da->items, newcap * sizeof *p);
        if (p == NULL) {
            fprintf(stderr, "DaPush: out of memory\n");
            exit(1);
        }
        da->items = p;
        da->cap = newcap;
    }
    da->items[da->len++] = item;
}

void DaFree(struct DynArray *da)
{
    free(da->items);
    DaInit(da);
}

// -- the function-pointer machinery -------------------------------------
//
// qsort's own comparator type is int (*)(const void *, const void *), so a
// bridge function converts and forwards. Which comparator to forward to
// lives in g_cmp: qsort offers no way to pass extra context along.

static int (*g_cmp)(const struct Item *, const struct Item *);

static int CmpBridge(const void *pa, const void *pb)
{
    return g_cmp((const struct Item *)pa, (const struct Item *)pb);
}

void DaSort(struct DynArray *da, int (*cmp)(const struct Item *, const struct Item *))
{
    g_cmp = cmp;
    qsort(da->items, da->len, sizeof *da->items, CmpBridge);
    g_cmp = NULL;
}

void DaEach(const struct DynArray *da, void (*visit)(const struct Item *))
{
    for (size_t i = 0; i < da->len; ++i)
        visit(&da->items[i]);
}

// -- the callbacks this driver supplies --------------------------------

static int CmpByKey(const struct Item *a, const struct Item *b)
{
    return strcmp(a->key, b->key);
}

static int CmpByValue(const struct Item *a, const struct Item *b)
{
    return (a->value > b->value) - (a->value < b->value);
}

static void PrintItem(const struct Item *it)
{
    printf("%s %ld\n", it->key, it->value);
}

int main(void)
{
    struct DynArray da;
    DaInit(&da);

    const char *keys[10] = {
        "pear", "apple", "fig", "banana", "cherry",
        "date", "elder", "grape", "kiwi", "lemon",
    };
    for (int i = 0; i < 10; ++i) {
        struct Item item;
        snprintf(item.key, sizeof item.key, "%s", keys[i]);
        item.value = i;
        DaPush(&da, item);
    }

    printf("before:\n");
    DaEach(&da, PrintItem);

    DaSort(&da, CmpByKey);
    printf("sorted by key:\n");
    DaEach(&da, PrintItem);

    DaSort(&da, CmpByValue);
    printf("sorted by value:\n");
    DaEach(&da, PrintItem);

    DaFree(&da);
    return 0;
}
