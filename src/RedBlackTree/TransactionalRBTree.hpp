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
#include "../STM/EncounterModeTx.hpp"
#include "../STM/CommitModeTx.hpp"
#include "../STM/TLCommitModeTx.hpp"

#define __1 EncounterModeTx<Node<T>*>
#define __2 TLCommitModeTx<Node<T>*>
#define __3 CommitModeTx<Node<T>*>

#define TX_ __2

#define BLACK TransactionalRBTree::B
#define RED   TransactionalRBTree::R

template<class T = int>
class TransactionalRBTree : public AbstractTree<T> {

public:

    TransactionalRBTree() {
        nil = new Node<T>;
        nil->c = BLACK;
        root = nil;
        nil->l = nil->r = nil->p = nil;
    }

    ~TransactionalRBTree() {
        root = empty(root);
        delete nil;
        delete RED;
        delete BLACK;
    }

    void insert(T x) override { insert(new Node<T>(x)); }

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

    void insert(Node<T> *z) {
        using AbortException = typename TX_::AbortException;

        TX_ Tx;
        bool done = false;
        while (!done) {
            try {
                Tx.begin();

                Node<T> *y = nil;
                Node<T> *x = Tx.read(&root);

                while (x != nil) {
                    y = x;
                    if (z->key < x->key) x = Tx.read(&x->l);
                    else x = Tx.read(&x->r);
                }

                z->p = y;

                if (y == nil) {
                    Tx.write(&root, z);
                } 
                else if (z->key < y->key) {
                    Tx.write(&y->l, z);
                } 
                else {
                    Tx.write(&y->r, z);
                } 

                z->l = nil;
                z->r = nil;
                z->c = RED;

                insert_fixup(Tx, z);

                done = Tx.commit();
            }
            catch(AbortException&) {
                Tx.abort();
                done = false;
            }
        }
    }

    void insert_fixup(TX_& Tx, Node<T> *z) {
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
                if (Tx.read(&y->c) == RED) {
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
        /* Node<T> *r = Tx.read(&root->c);
        Tx.write(&r, BLACK); */
        Tx.write(&root->c, BLACK);
    }

    void left_rotate(TX_& Tx, Node<T> *x) {
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

    void right_rotate(TX_& Tx, Node<T> *y) {
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