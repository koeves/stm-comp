#ifndef ABSTRACT_TREE_HPP
#define ABSTRACT_TREE_HPP

template<class T = int>
class AbstractTree {
public:
    virtual void insert(T x);

    virtual void remove(T x);

    virtual void print();
};

#endif