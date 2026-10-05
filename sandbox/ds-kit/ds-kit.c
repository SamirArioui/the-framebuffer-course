// ds-kit.c — the data-structures kit of Part 0, starting with its element
// type and the memory it lives in.
//
// Lesson 007: structs as laid-out memory — sizeof, offsetof, padding.
#include <stdio.h>
#include <stddef.h>   // offsetof
#include <stdalign.h> // alignof (C11)

struct Item {
    char key[16];
    long value;
};

// The same three fields in two different orders.
struct Scattered {
    char tag;
    long score;
    char flag;
};

struct Compact {
    char tag;
    char flag;
    long score;
};

int main(void)
{
    printf("== scalars ==\n");
    printf("sizeof(char) = %zu\n", sizeof(char));
    printf("sizeof(long) = %zu\n", sizeof(long));

    printf("== struct Item ==\n");
    printf("sizeof(struct Item)  = %zu\n", sizeof(struct Item));
    printf("alignof(struct Item) = %zu\n", alignof(struct Item));
    printf("Item.key   offset %zu\n", offsetof(struct Item, key));
    printf("Item.value offset %zu\n", offsetof(struct Item, value));
    // key is 16 bytes, so the gap before value is its offset minus 16.
    printf("padding before value: %zu bytes\n",
           offsetof(struct Item, value) - 16);

    printf("== same fields, two orders ==\n");
    printf("struct Scattered { tag, score, flag }: sizeof %zu\n",
           sizeof(struct Scattered));
    printf("  tag   offset %zu\n", offsetof(struct Scattered, tag));
    printf("  score offset %zu\n", offsetof(struct Scattered, score));
    printf("  flag  offset %zu\n", offsetof(struct Scattered, flag));
    printf("struct Compact { tag, flag, score }: sizeof %zu\n",
           sizeof(struct Compact));
    printf("  tag   offset %zu\n", offsetof(struct Compact, tag));
    printf("  flag  offset %zu\n", offsetof(struct Compact, flag));
    printf("  score offset %zu\n", offsetof(struct Compact, score));
    printf("Scattered[10] = %zu bytes, Compact[10] = %zu bytes\n",
           sizeof(struct Scattered[10]), sizeof(struct Compact[10]));

    return 0;
}
