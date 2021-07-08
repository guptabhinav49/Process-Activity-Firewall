#include <headers.hpp>
#include <cwalk/cwalk.h>

// binary search to find whether the given string exists in the list
int find(std::vector<std::tuple<std::string,int,int> > &fpaths, std::string &name){
    int n = fpaths.size(); 
    int idx = 0, jmp = n/2;
    while(jmp>0){
        if(idx+jmp < n && (std::get<0>(fpaths[idx+jmp]) <= name)) idx += jmp;
        else jmp /=2;
    }
    if(std::get<0>(fpaths[idx]) == name) return idx;
    else return -1;
}

// matching regex
int match_regex(std::vector<std::tuple<std::string,int,int,int> >& arr, std::string &name){
    const char *basename;
    char filename[MAX_PATHLEN];
    size_t l;
    cwk_path_get_basename(name.c_str(), &basename, &l);
    sprintf(filename, "%.*s", (int)l, basename);

    // std::cout << name << std::endl;

    for(int i=0; i<(int)arr.size(); i++){
        if(std::get<3>(arr[i]) == 0){
            if(std::regex_match(name, std::regex(std::get<0>(arr[i])))){
                return i;
            }
        }
        else{
            if(std::regex_match(filename, std::regex(std::get<0>(arr[i])))){
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
    strncat(b, "`", 2);

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


/*
    basic print funtions for the data structures
*/
void out(std::vector<std::tuple<std::string,int,int> > &fpaths){
    std::cout << "ABSOLUTE MATCHING" << std::endl;
    for(auto i : fpaths){
        std::cout << std::get<0>(i) << ' ' << std::get<1>(i) << ' ' << std::get<2>(i) << std::endl;
    }
}

void out(std::vector<std::tuple<std::string,int,int,int> >& fpaths){
    std::cout << "REGEX MATCHING" << std::endl;
    for(auto i : fpaths){
        std::cout << std::get<0>(i) << ' ' << std::get<1>(i) << ' ' << std::get<2>(i) << ' ' << std::get<3>(i) << std::endl;
    }
}