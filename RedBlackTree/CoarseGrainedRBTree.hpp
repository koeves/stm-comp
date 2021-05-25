/* 
 *  Coarse-grained Red-Black Tree datastructure implementation   
 *        
 *        based on the book:
 *              Thomas H. Cormen, Charles E. Leiserson, Ronald L. Rivest, and Clifford Stein. 2009. 
 *              Introduction to Algorithms, Third Edition (3rd. ed.). The MIT Press.    
 */

#ifndef RED_BLACK_TREE_HPP
#define RED_BLACK_TREE_HPP

#include <iostream>
#include <mutex>
#include "AbstractTree.hpp"

#define _BLACK  Node<T>::Color::BLACK 
#define _RED    Node<T>::Color::RED  

template<class T = int>
class CoarseGrainedRBTree : public AbstractTree<T> { 

public:

    CoarseGrainedRBTree() {
        nil = new Node<T>;
        nil->color = _BLACK;
        root = nil;
    }

    CoarseGrainedRBTree(T x) {
        root = new Node<T>(x);
        nil = new Node<T>;
        root->l = root->r = root->p = nil->l = nil->r = nil->p = nil;
        root->color = nil->color = _BLACK;
    }

    ~CoarseGrainedRBTree() {
        root = empty(root);
        delete nil;
    }

    void insert(T x) override { insert(new Node<T>(x)); }

    void remove(T x) override {}

    void print() const override { print_inorder(root); }

private:

    template<class U = int>
    struct Node {
        Node() {
            l = r = p = nullptr;
            key = 0;
            color = Color::RED;
        }

        Node(U x) {
            l = r = p = nullptr;
            key = x;
            color = Color::RED;
        }

        Node<U> *l, *r, *p;
        U key;

        enum class Color { RED, BLACK } color;
    };

    std::mutex my_lock;

    Node<T> *root, *nil;

    Node<T> *empty(Node<T> *t) {
        if (t == nil) return t;

        empty(t->l);
        empty(t->r);
        delete t;

        return nullptr;
    }

    void print_inorder(const Node<T> *t) const {
        if (t == nil) return;

        print_inorder(t->l);
        
        std::cout << ((t == root) ? "ROOT (" :  "     (")
                  << ((t->color == _BLACK) ? "BLACK) " : "RED)   ") << t->key << " -> "
                  << "p: " << ((t->p == nil) ? "NIL" : std::to_string(t->p->key)) << ", " 
                  << "l: " << ((t->l == nil) ? "NIL" : std::to_string(t->l->key)) << ", "
                  << "r: " << ((t->r == nil) ? "NIL" : std::to_string(t->r->key)) << std::endl;

        print_inorder(t->r);
    }

    void insert(Node<T> *z) {
        std::lock_guard<std::mutex> guard(my_lock);

        Node<T> *y = nil;
        Node<T> *x = root;

        while (x != nil) {
            y = x;
            if (z->key < x->key) x = x->l;
            else x = x->r;
        }

        z->p = y;

        if (y == nil) root = z;
        else if (z->key < y->key) y->l = z;
        else y->r = z;

        z->l = z->r = nil;
        z->color = _RED;

        insert_fixup(z);
    }

    void insert_fixup(Node<T> *z) {
        while (z->p->color == _RED) {
            if (z->p == z->p->p->l) {
                Node<T> *y = z->p->p->r;
                if (y->color == _RED) {
                    z->p->color = y->color = _BLACK;
                    z->p->p->color = _RED;
                    z = z->p->p;
                }
                else if (z == z->p->r) {
                    z = z->p;
                    left_rotate(z);
                }
                else {
                    z->p->color = _BLACK;
                    z->p->p->color = _RED;
                    right_rotate(z->p->p);
                }
            }
            else {
                Node<T> *y = z->p->p->l;
                if (y->color == _RED) {
                    z->p->color = y->color = _BLACK;
                    z->p->p->color = _RED;
                    z = z->p->p;
                }
                else if (z == z->p->l) {
                    z = z->p;
                    right_rotate(z);
                }
                else {
                    z->p->color = _BLACK;
                    z->p->p->color = _RED;
                    left_rotate(z->p->p);
                }
            }
        }
        root->color = _BLACK;
    }

    void left_rotate(Node<T> *x) {
        Node<T> *y = x->r;
        x->r = y->l;

        if (y->l != nil)
            y->l->p = x;

        y->p = x->p;

        if (x->p == nil) root = y;
        else if (x == x->p->l) x->p->l = y;
        else x->p->r = y;

        y->l = x;
        x->p = y;
    }

    void right_rotate(Node<T> *y) {
        Node<T> *x = y->l;
        y->l = x->r;

        if (x->r != nil)
            x->r->p = y;

        x->p = y->p;

        if (y->p == nil) root = x;
        else if (y == y->p->r) y->p->r = x;
        else y->p->l = x;

        y->p = x;
        x->r = y;
    }
};

#endif