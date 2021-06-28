#ifndef TRIE_H
#define TRIE_H

#include "trie/vector.h"
#include <stdbool.h>

#define ALPHABET_SIZE 128

typedef struct trie_v {
    int *nxt;
    bool leaf;
    int mode;
} trie_v;

void trie_v_init(trie_v *v);
void trie_add_string(vector *v, const char *s, int n, int reverse, int mode);
void trie_free(vector *v);
int trie_traverse(vector *v, const char *s, int n, int reverse);

#endif