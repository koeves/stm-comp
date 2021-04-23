#ifndef RED_BLACK_TREE_HPP
#define RED_BLACK_TREE_HPP

#include "AbstractTree.hpp"

template<class T = int>
class RedBlackTree : AbstractTree<T> { 
public:
    void insert(T x) override;

    void remove(T x) override;

    void print() override;

private:
    enum class Color { RED, BLACK };
};

#endif