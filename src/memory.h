#ifndef MEMORY_H
#define MEMORY_H

#include <stdio.h>
#include <stdlib.h>

// memory.h - custom memory allocation functions

// if this is defined, print something during each malloc or free
// #define APC_LOG_ALLOCS

extern void apc_exit(int exit_code);

void* apc_malloc(size_t size);
void* apc_realloc(void* ptr, size_t size);
void apc_free(void* ptr);

#endif // MEMORY_H
