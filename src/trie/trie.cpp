/*
    Trie data structure implentation
*/
#include <trie/trie.hpp>

Vertex::Vertex(){
    std::fill(std::begin(next), std::end(next), -1);
    leaf = false;
    mode = 0;
    permission = 0;
}

Trie::Trie(){
    trie = std::vector<Vertex> (1);
}

void Trie::add_string(std::string& s, int permission, int mode, bool reverse){
    int v = 0;
    for(int i=0; i<(int)s.length(); i++){
        int ch = (reverse? s[s.length()-i-1] : s[i]);
        if(trie[v].next[ch] == -1){
            trie[v].next[ch] = trie.size();
            trie.emplace_back();
        }   
        v = trie[v].next[ch];
    }
    trie[v].leaf = true;
    trie[v].mode = mode;
    trie[v].permission = permission;
}

bool Trie::traverse(const std::string& s, int& permission, int& mode, bool reverse) const {
    int v = 0;
    for(int i=0; i<(int)s.size(); i++){
        int ch = (reverse?s[s.size()-i-1]:s[i]);
        printf("asd %d %c\n", i, ch);
        if(trie[v].leaf){
            mode = trie[v].mode;
            permission = trie[v].permission;
            return 1;
        }
        else if(trie[v].next[ch] == -1) return 0;
        else v = trie[v].next[ch];
    }
    if(trie[v].leaf){
        mode = trie[v].mode;
        permission = trie[v].permission;
    }

    return trie[v].leaf;
}

size_t Trie::size(){
    return trie.size()-1;
}