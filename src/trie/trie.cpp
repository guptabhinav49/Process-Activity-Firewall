/*
    Trie data structure implentation
*/
#include "trie/trie.h"

void trie_add_string(vector *v, const char *s, int n, int reverse, int mode){
    int idx = 0;
    // puts("here");
    for(int i=0; i<n; i++){
        int c = (reverse == 1 ? s[n-i-1] : s[i]);

        trie_v *emptyV;
        if(((trie_v *)vector_get(v, idx))->nxt[c] == -1){
            // puts("here");
            trie_v *ex = (trie_v *)vector_get(v, idx);
            ex->nxt[c] = vector_size(v);
            emptyV = (trie_v *)malloc(sizeof(trie_v));
            trie_v_init(emptyV);
            vector_add(v, emptyV);
        }
        idx = ((trie_v *)vector_get(v, idx))->nxt[c];
    }
    ((trie_v *)vector_get(v, idx))->leaf = true;
    ((trie_v *)vector_get(v, idx))->mode = mode;
    // printf("idx: %d, size: %lu\n", idx, vector_size(v));
}

int trie_traverse(vector *v, const char *s, int n, int reverse){
    if(n <= 0) return 0;
    // printf("%d\n", ((trie_v *)vector_get(v, 3))->mode);
    // printf("size: %lu\n", vector_size(v));
    int idx = 0;
    for(int i = 0; i<n; i++){
        int c = (reverse == 1 ? s[n-i-1] : s[i]);
        // printf("%c %d\n", c, idx);
        if(((trie_v *)vector_get(v, idx))->leaf == true){ 
            return ((trie_v *)vector_get(v, idx))->mode;
        }
            // puts("here");
        if(((trie_v *)vector_get(v, idx))->nxt[c] == -1){
            return -1;
        }
        idx = ((trie_v *)vector_get(v, idx))->nxt[c];
    }
    if(((trie_v *)vector_get(v, idx))->leaf == true) {
        return ((trie_v *)vector_get(v, idx))->mode;
    }
    return -1;
}