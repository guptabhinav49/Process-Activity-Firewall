#include "headers.h"
#include "cwalk/cwalk.h"

static int compare(const void *a, const void *b){
    return strcmp((char *)a, (char *)b);
}

void sort(char a[][MAX_PATHLEN], int n){
    qsort(a, n, MAX_PATHLEN, compare);
}

// binary search to find whether the given string exists in the list
int find(char arr[][MAX_PATHLEN], const char *name, int n){
    int idx = 0, jmp = n/2;
    while(jmp>0){
        if(idx+jmp < n && strcmp(arr[idx+jmp], name) <= 0) idx += jmp;
        else jmp /=2;
    }
    if(strcmp(arr[idx], name) == 0) return 1;
    else return 0;
}

// matching regex
int match_regex(regex_t *arr, char *type, const char* s, int n){
    const char *basename;
    char filename[MAX_PATHLEN];
    size_t l;
    cwk_path_get_basename(s, &basename, &l);
    sprintf(filename, "%.*s", (int)l, basename);
    // puts(filename);

    for(int i=0; i<n; i++){
        if(type[i] == '0'){
            if(regexec(arr+i, s, 0, NULL, 0) == 0){
                return i;
            }
        }
        else{
            if(regexec(arr+i, filename, 0, NULL, 0) == 0){
                return i;
            }
        }
    }
    return -1;
}

// Implementing the KMP algorithm, for substring matching: https://cp-algorithms.com/string/prefix-function.html
int match(char a[], const char sub[]){
    if(strlen(a) < strlen(sub)) return 0;

    char b[MAX_SUBSTRLEN_TO_CHECK];
    strncpy(b, sub, sizeof(b));
    strncat(b, "`", 1);

    size_t prefix_match[strlen(b)];

    prefix_match[0] = 0;

    for(int i=1; i<strlen(b); i++){
        int j = prefix_match[i-1];

        while(j>0 && b[i] != b[j]) j = prefix_match[j-1];

        if(b[i] == b[j]) j++;
        prefix_match[i] = j;
    }

    size_t last = prefix_match[strlen(b)-1];
    int ans = 0;

    for(int i = 0; i<strlen(a); i++){
        while(last > 0 && a[i] != b[last]) last = prefix_match[last-1];

        if(a[i] == b[last]) last++;

        if(last == strlen(b)-1){
            ans = 1;
            break;
        }
    }
    return ans;
}