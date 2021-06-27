#ifndef VECTOR_H
#define VECTOR_H

#include <stdio.h>
#include <stdlib.h>
#define VECTOR_INIT_CAPACITY 4

// #define VECTOR_INIT(vec) vector vec; vector_init(&vec)
// #define VECTOR_ADD(vec, item) vector_add(&vec, (void *) item)
// #define VECTOR_SET(vec, id, item) vector_set(&vec, id, (void *) item)
// #define VECTOR_GET(vec, type, id) (type) vector_delete(&vec, id)
// #define VECTOR_DELETE(vec, id) vector_delete(&vec, id)
// #define VECTOR_SIZE(vec) vector_size(&vec)
// #define VECTOR_FREE(vec) vector_free(&vec)

typedef struct vector {
    void **items;
    size_t capacity;
    size_t size;
} vector;

void vector_init(vector *);
size_t vector_size(vector *);
static void vector_resize(vector *, size_t);
void vector_add(vector *, void *);
void *vector_get(vector *, int);
void vector_set(vector *, int, void *);
void vector_delete(vector *, int);
void vector_free(vector *);

#endif