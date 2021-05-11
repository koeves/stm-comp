#ifndef SKIPLIST_NODE_HPP
#define SKIPLIST_NODE_HPP

#include <vector>

template<class T = int>
class SkiplistNode {
public:
    SkiplistNode() {}

    SkiplistNode(int h) {
        neighbours.resize(h);
    }

    SkiplistNode<T> at(int i) { return neighbours.at(i); }

    int get_height() { return neighbours.count(); }

private:

    T value;
    std::vector<SkiplistNode<T>> neighbours;
};


#endif