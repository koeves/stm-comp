#ifndef ABSTRACT_TREE_HPP
#define ABSTRACT_TREE_HPP

template<class T = int>
struct AbstractTree {  
    virtual void insert(T x) {}
    virtual void remove(T x) {}
    virtual void print() const {}
};

#endif