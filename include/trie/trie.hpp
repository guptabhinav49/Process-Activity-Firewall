#ifndef TRIE_HPP
#define TRIE_HPP

#include <vector>
#include <string>
#define ALPHABET_SIZE 128

struct Vertex {
    int next[ALPHABET_SIZE];
    bool leaf;
    int permission, mode;
    Vertex();
};

struct Trie {
    std::vector<Vertex> trie;

    Trie();
    void add_string(std::string&, int, int, bool rev);
    bool traverse(const std::string&, int&, int&, bool rev) const;
    size_t size();
};

#endif