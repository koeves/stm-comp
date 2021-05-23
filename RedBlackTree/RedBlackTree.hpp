/* 
 *  Red-Black Tree datastructure implementation   
 *        
 *        based on the book:
 *              Thomas H. Cormen, Charles E. Leiserson, Ronald L. Rivest, and Clifford Stein. 2009. 
 *              Introduction to Algorithms, Third Edition (3rd. ed.). The MIT Press.    
 */

#ifndef RED_BLACK_TREE_HPP
#define RED_BLACK_TREE_HPP

#include <iostream>
#include "AbstractTree.hpp"

#define _BLACK  Node<T>::Color::BLACK 
#define _RED    Node<T>::Color::RED  

template<class T = int>
class RedBlackTree : public AbstractTree<T> { 

public:

    RedBlackTree() {
        nil = new Node<T>;
        nil->color = _BLACK;
        root = nil;
    }

    RedBlackTree(T x) {
        root = new Node<T>(x);
        nil = new Node<T>;
        root->l = root->r = root->p = nil->l = nil->r = nil->p = nil;
        root->color = nil->color = _BLACK;
    }

    ~RedBlackTree() {
        root = empty(root);
        delete nil;
    }

    void insert(T x) override { insert(new Node<T>(x)); }

    void remove(T x) override { remove(find(root, x)); }

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

    Node<T> *find_min(Node<T> *t) {
        if (t == nil) return t;
        else if (t->l == nil) return t;
        else return find_min(t->l);
    }

    Node<T> *find_max(Node<T> *t) {
        if (t == nil) return t;
        else if (t->r == nil) return t;
        else return find_min(t->r);
    }

    Node<T> *find(Node<T> *t, T x) {
        if(t == nil) return t;
        else if(x < t->key) return find(t->l, x);
        else if(x > t->key) return find(t->r, x);
        else return t;
    }

    void insert(Node<T> *z) {
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

    void remove(Node<T> *z) {
        Node<T> *y = z, *x;
        enum Node<T>::Color y_orig_color = y->color;

        if (z->l == nil) {
            x = z->r;
            transplant(z, z->r);
        }
        else if (z->r == nil) {
            x = z->l;
            transplant(z, z->l);
        }
        else {
            y = find_min(z->r);
            y_orig_color = y->color;
            x = y->r;
            if (y->p == z) {
                x->p = y;
            }
            else {
                transplant(y, y->r);
                y->r = z->r;
                y->r->p = y;
            }
            transplant(z, y);
            y->l = z->l;
            y->l->p = y;
            y->color = z->color;
        }

        if (y_orig_color == _BLACK)
            delete_fixup(x);

        delete z;
    }

    void delete_fixup(Node<T> *x) {
        while (x != root && x->color == _BLACK) {
            if (x == x->p->l) {
                Node<T> *w = x->p->r;
                if (w->color == _RED) {
                    w->color = _BLACK;
                    x->p->color = _RED;
                    left_rotate(x->p);
                    w = x->p->r;
                }

                if (w->l->color == _BLACK && w->r->color == _BLACK) {
                    w->color = _RED;
                    x = x->p;
                }
                else if (w->r->color == _BLACK) {
                    w->l->color = _BLACK;
                    w->color = _RED;
                    right_rotate(w);
                    w = x->p->r;
                }
                else {
                    w->color = x->p->color;
                    x->p->color = _BLACK;
                    w->r->color = _BLACK;
                    left_rotate(x->p);
                    x = root;
                }
            }
            else {
                Node<T> *w = x->p->l;
                if (w->color == _RED) {
                    w->color = _BLACK;
                    x->p->color = _RED;
                    right_rotate(x->p);
                    w = x->p->l;
                }

                if (w->r->color == _BLACK && w->l->color == _BLACK) {
                    w->color = _RED;
                    x = x->p;
                }
                else if (w->l->color == _BLACK) {
                    w->r->color = _BLACK;
                    w->color = _RED;
                    left_rotate(w);
                    w = x->p->l;
                }
                else {
                    w->color = x->p->color;
                    x->p->color = _BLACK;
                    w->l->color = _BLACK;
                    right_rotate(x->p);
                    x = root;
                }
            }
        }
        x->color = _BLACK;
    }

    void transplant(Node<T> *u, Node<T> *v) {
        if (u->p == nil) root = v;
        else if (u == u->p->l) u->p->l = v;
        else u->p->r = v;
        v->p = u->p;
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