#include "trie/vector.h"

void vector_init(vector *v){
    v->capacity = VECTOR_INIT_CAPACITY;
    v->size = 0;
    v->items = (void **)malloc(sizeof(void *) * v->capacity);
}

size_t vector_size(vector *v){
    return v->size;
}

static void vector_resize(vector *v, size_t capacity){
    void **items = (void **)realloc(v->items, sizeof(void *) *capacity);
    if(items){
        v->items = items;
        v->capacity = capacity;
    }
}

void vector_add(vector *v, void *item){
    if(v->capacity == v->size) vector_resize(v, v->capacity*2);

    v->items[v->size++] = item;
}

void vector_set(vector *v, int index, void *item){
    if(index >=0 && index < (int)v->size)
        v->items[index] = item;
}

void *vector_get(vector *v, int index){
    if(index >= 0 && index < v->size)
        return v->items[index];
}

void vector_delete(vector *v, int index) {
    if (index < 0 || index >= v->size){
        return;
    }

    v->items[index] = NULL;
    for(int i=0; i<(int)v->size; i++){
        v->items[i] = v->items[i+1];
        v->items[i+1] = NULL;
    }
    v->size--;

    if(v->size > 0 && v->size == v->capacity/4)
        vector_resize(v, v->capacity/2);
}

void vector_free(vector *v){
    free(v->items);
}