// dynarray.c — the generic dynarray's implementation.
//
// Lesson 012: one translation unit, one part of the kit. The header is
// included first so the compiler checks this file against the contract.
#include "dynarray.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
