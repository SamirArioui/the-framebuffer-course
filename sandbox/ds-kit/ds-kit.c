// ds-kit.c — the data-structures kit of Part 0: a dynarray of Items that
// grows by realloc with capacity doubling.
//
// Lesson 008: realloc growth — len, cap, and why doubling wins.
#include <stdio.h>
#include <stdlib.h>

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
        printf("len=%zu cap=%zu\n", da.len, da.cap);
    }

    printf("first=%s last=%s\n", da.items[0].key, da.items[9].key);

    DaFree(&da);
    return 0;
}
