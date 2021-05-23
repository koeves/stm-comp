#ifndef SKIPLIST_NODE_HPP
#define SKIPLIST_NODE_HPP

#include <vector>
#include <memory>
#include <cstdlib>

template<class T = int>
struct SkiplistNode {
    SkiplistNode(int h) : height(h), neighbours(nullptr) {}
    SkiplistNode(T val, int h) {
        value = val;
        height = h;
        neighbours = new SkiplistNode*[h + 1];
        memset(neighbours, 0, sizeof(SkiplistNode*)*(h + 1));
    }

    ~SkiplistNode() { if (neighbours) delete[] neighbours; }

    T value;
    int height;
    SkiplistNode<T> **neighbours;
};


#endif