#include "memory.h"

void* apc_malloc(size_t size) {
    void* m = malloc(size);
    if (m == NULL) {
        fputs(" = memory error\n", stdout);
        exit(4);
    }
    return m;
}

void* apc_realloc(void* ptr, size_t size) {
    void* r = realloc(ptr, size);
    if (r == NULL) {
        fputs(" = memory error\n", stdout);
        exit(4);
    }
    return r;
}

void apc_free(void* ptr) {
    free(ptr);
}

