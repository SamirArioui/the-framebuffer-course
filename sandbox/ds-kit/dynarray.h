// dynarray.h — the generic dynarray: any element type as bytes + a stride.
//
// Lesson 012: a header is a contract between translation units. Everything
// here is what callers may rely on: the struct's shape and the Da* calls.
#ifndef DYNARRAY_H
#define DYNARRAY_H

#include <stddef.h> // size_t

struct DynArray {
    void *data;
    size_t len;
    size_t cap;
    size_t elem_size;
};

void DaInit(struct DynArray *da, size_t elem_size);
void DaPush(struct DynArray *da, const void *elem);
void *DaAt(struct DynArray *da, size_t i);
void DaSort(struct DynArray *da, int (*cmp)(const void *, const void *));
void DaEach(const struct DynArray *da, void (*visit)(const void *));
void DaFree(struct DynArray *da);

#endif // DYNARRAY_H
