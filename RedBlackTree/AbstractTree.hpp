/* 
 *  Abstract Tree datastructure interface 
 *        
 */

#ifndef ABSTRACT_TREE_HPP
#define ABSTRACT_TREE_HPP

template<class T = int>
struct AbstractTree {  
    virtual void insert(T x)   = 0;
    virtual void remove(T x)   = 0;
    virtual void print() const = 0;
};

#endif