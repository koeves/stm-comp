/* 
 *  Transactional Red-Black Tree datastructure implementation   
 *        
 *        based on the book:
 *              Thomas H. Cormen, Charles E. Leiserson, Ronald L. Rivest, and Clifford Stein. 2009. 
 *              Introduction to Algorithms, Third Edition (3rd. ed.). The MIT Press.    
 */

#ifndef TX_RED_BLACK_TREE_HPP
#define TX_RED_BLACK_TREE_HPP

#include <iostream>
#include "AbstractTree.hpp"
#include "../STM/CommitModeTx.hpp"

#define BLACK TransactionalRBTree::B
#define RED   TransactionalRBTree::R

template<class T = int>
class TransactionalRBTree : public AbstractTree<T> {

public:

    TransactionalRBTree() {
        nil = new Node<T>;
        nil->c = BLACK;
        root = nil;
    }

    TransactionalRBTree(T x) {
        root = new Node<T>(x);
        nil = new Node<T>;
        root->l = root->r = root->p = nil->l = nil->r = nil->p = nil;
        root->c = nil->c = BLACK;
    }

    ~TransactionalRBTree() {
        root = empty(root);
        delete nil;
        delete RED;
        delete BLACK;
    }

    void insert(T x) override { insert(new Node<T>(x)); }

    void remove(T x) override { remove(find(root, x)); }

    void print() const override { print_inorder(root); }

private:

    template<class U = T>
    struct Node {
        Node() {
            l = r = p = nullptr;
            key = 0;
            c = RED;
        }

        Node(U x) {
            l = r = p = nullptr;
            key = x;
            c = RED;
        }

        Node<U> *l, *r, *p, *c;
        U key;
    };

    inline static Node<T> *R = new Node<T>, 
                          *B = new Node<T>;

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
                  << ((t->c == BLACK) ? "BLACK) " : "RED)   ") << t->key << " -> "
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
        else return find_max(t->r);
    }

    Node<T> *find(Node<T> *t, T x) {
        if(t == nil) return t;
        else if(x < t->key) return find(t->l, x);
        else if(x > t->key) return find(t->r, x);
        else return t;
    }

    void insert(Node<T> *z) {
        using AbortException = typename CommitModeTx<Node<T>*>::AbortException;

        CommitModeTx<Node<T>*> Tx;
        bool done = false;
        while (!done) {
            try {
                Tx.begin();

                Node<T> *y = Tx.read(&nil);
                Node<T> *x = Tx.read(&root);

                while (x != Tx.read(&nil)) {
                    y = x;
                    if (z->key < Tx.read(&x->key)) x = Tx.read(&x->l);
                    else x = Tx.read(&x->r);
                }

                Tx.write(&z->p, y);

                if (y == nil) Tx.write(&root, z);
                else if (z->key < Tx.read(&y->key)) Tx.write(&y->l, z);
                else Tx.write(&y->r, z);

                Tx.write(&z->l, nil);
                Tx.write(&z->r, nil);
                Tx.write(&z->c, RED);

                insert_fixup(Tx, z);

                done = Tx.commit();
            }
            catch(AbortException&) {
                Tx.abort();
                done = false;
            }
        }
    }

    void insert_fixup(CommitModeTx<Node<T>*>& Tx, Node<T> *z) {
        while (Tx.read(&z->p->c) == RED) {
            if (Tx.read(&z->p) == Tx.read(&z->p->p->l)) {
                Node<T> *y = Tx.read(&z->p->p->r);
                if (y->c == RED) {
                    Tx.write(&z->p->c, BLACK);
                    Tx.write(&y->c, BLACK);
                    Tx.write(&z->p->p->c, RED);
                    z = Tx.read(&z->p->p);
                }
                else if (z == Tx.read(&z->p->r)) {
                    z = Tx.read(&z->p);
                    left_rotate(Tx, z);
                }
                else {
                    Tx.write(&z->p->c, BLACK);
                    Tx.write(&z->p->p->c, RED);
                    right_rotate(Tx, z->p->p);
                }
            }
            else {
                Node<T> *y = Tx.read(&z->p->p->l);
                if (y->c == RED) {
                    Tx.write(&z->p->c, BLACK);
                    Tx.write(&y->c, BLACK);
                    Tx.write(&z->p->p->c, RED);
                    z = Tx.read(&z->p->p);
                }
                else if (z == Tx.read(&z->p->l)) {
                    z = Tx.read(&z->p);
                    right_rotate(Tx, z);
                }
                else {
                    Tx.write(&z->p->c, BLACK);
                    Tx.write(&z->p->p->c, RED);
                    left_rotate(Tx, z->p->p);
                }
            }
        }
        Tx.write(&root->c, BLACK);
    }

    void remove(Node<T> *z) {
        CommitModeTx<Node<T>*> Tx;

        Node<T> *y = z, *x;
        Node<T> *y_orig_color = y->c;

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
            y_orig_color = y->c;
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
            y->c = z->c;
        }

        if (y_orig_color == BLACK)
            delete_fixup(Tx, x);

        delete z;
    }

    void delete_fixup(CommitModeTx<Node<T>*>& Tx, Node<T> *x) {
        while (x != root && x->c == BLACK) {
            if (x == x->p->l) {
                Node<T> *w = x->p->r;
                if (w->c == RED) {
                    w->c = BLACK;
                    x->p->c = RED;
                    left_rotate(Tx, x->p);
                    w = x->p->r;
                }

                if (w->l->c == BLACK && w->r->c == BLACK) {
                    w->c = RED;
                    x = x->p;
                }
                else if (w->r->c == BLACK) {
                    w->l->c = BLACK;
                    w->c = RED;
                    right_rotate(Tx, w);
                    w = x->p->r;
                }
                else {
                    w->c = x->p->c;
                    x->p->c = BLACK;
                    w->r->c = BLACK;
                    left_rotate(Tx, x->p);
                    x = root;
                }
            }
            else {
                Node<T> *w = x->p->l;
                if (w->c == RED) {
                    w->c = BLACK;
                    x->p->c = RED;
                    right_rotate(Tx, x->p);
                    w = x->p->l;
                }

                if (w->r->c == BLACK && w->l->c == BLACK) {
                    w->c = RED;
                    x = x->p;
                }
                else if (w->l->c == BLACK) {
                    w->r->c = BLACK;
                    w->c = RED;
                    left_rotate(Tx, w);
                    w = x->p->l;
                }
                else {
                    w->c = x->p->c;
                    x->p->c = BLACK;
                    w->l->c = BLACK;
                    right_rotate(Tx, x->p);
                    x = root;
                }
            }
        }
        x->c = BLACK;
    }

    void transplant(Node<T> *u, Node<T> *v) {
        if (u->p == nil) root = v;
        else if (u == u->p->l) u->p->l = v;
        else u->p->r = v;
        v->p = u->p;
    }

    void left_rotate(CommitModeTx<Node<T>*>& Tx, Node<T> *x) {
        Node<T> *y = Tx.read(&x->r);
        Tx.write(&x->r, y->l);

        if (Tx.read(&y->l) != nil)
            Tx.write(&y->l->p, x);

        Tx.write(&y->p, x->p);

        if (Tx.read(&x->p) == nil) Tx.write(&root, y);
        else if (x == Tx.read(&x->p->l)) Tx.write(&x->p->l, y);
        else Tx.write(&x->p->r, y);

        Tx.write(&y->l, x);
        Tx.write(&x->p, y);
    }

    void right_rotate(CommitModeTx<Node<T>*>& Tx, Node<T> *y) {
        Node<T> *x = Tx.read(&y->l);
        Tx.write(&y->l, x->r);

        if (Tx.read(&x->r) != nil)
            Tx.write(&x->r->p, y);

        Tx.write(&x->p, y->p);

        if (Tx.read(&y->p) == nil) Tx.write(&root, x);
        else if (y == Tx.read(&y->p->r)) Tx.write(&y->p->r, x);
        else Tx.write(&y->p->l, x);

        Tx.write(&y->p, x);
        Tx.write(&x->r, y);
    }
};

#endif