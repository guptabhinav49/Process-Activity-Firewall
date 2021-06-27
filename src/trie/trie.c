#include "trie/trie.h"

void trie_v_init(trie_v *v) {
    // free(v->nxt);
    // v = (trie_v *) malloc(sizeof(trie_v *));
    v->nxt = (int *) malloc(sizeof(int)*ALPHABET_SIZE);
    for(short i=0; i<ALPHABET_SIZE; i++) v->nxt[i] = -1;
    v->leaf = false;
}

void trie_add_string(vector *v, const char *s, int n, int reverse){
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
    // printf("idx: %d, size: %lu\n", idx, vector_size(v));
}

void trie_free(vector *v){
    for(int i=vector_size(v)-1; i>=0; i--){
        free(((trie_v *)vector_get(v, i))->nxt);
        // free(((trie_v *)vector_get(v, i)));
    }
    vector_free(v);
}

int trie_traverse(vector *v, const char *s, int n, int reverse){
    if(n <= 0) return 0;

    int idx = 0;
    for(int i = 0; i<n; i++){
        int c = (reverse == 1 ? s[n-i-1] : s[i]);
        // printf("%c %d\n", c, c);
        if(((trie_v *)vector_get(v, idx))->leaf == true) return 1;

        if(((trie_v *)vector_get(v, idx))->nxt[c] == -1){
            return 0;
        }
        idx = ((trie_v *)vector_get(v, idx))->nxt[c];
    }
    if(((trie_v *)vector_get(v, idx))->leaf == true) return 1;

    return 0;
}