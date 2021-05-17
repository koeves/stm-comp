#ifndef TX_SKIP_HPP
#define TX_SKIP_HPP

#include <cstring>
#include <random>
#include <iostream>
#include "SkiplistNode.hpp"
#include "../STM/EncounterModeTx.hpp"
#include "../STM/CommitModeTx.hpp"
#include "../STM/Transaction.hpp"

template<class T = int>
class TransactionalSkiplist {
public:

    TransactionalSkiplist() :
        head(new SkiplistNode<T>(-1, MAX_LEVEL)), 
        level(new SkiplistNode<T>(0)),
        l(0)
    {}

    ~TransactionalSkiplist() { 
        SkiplistNode<T> *node = head->neighbours[0];
        while (node != NULL) {
            SkiplistNode<T> *old = node;
            node = node->neighbours[0];
            delete old;
        }
        delete head;
        delete level;
    }

    bool contains(T val);

    void add(T val);

    bool remove(T val);

    void display() {
        for (int i = l; i >= 0; i--) {
            SkiplistNode<T> *node = head->neighbours[i];
            std::cout << "Level " << i << ": ";
            while (node != NULL) {
                std::cout << node->value << " ";
                node = node->neighbours[i];
            }
            std::cout << "\n";
        }
    }

private:

    SkiplistNode<T> *head, *level;
    int l;

    static constexpr double PROB = 0.5;
    static const int MAX_LEVEL = 6;

    double get_random_prob() {
        std::random_device rd;
        std::mt19937 mt(rd());
        std::uniform_real_distribution<double> dist(0., 1.);

        return dist(mt);
    }

    int get_random_height() {
        double r = get_random_prob();
        int level = 0;
        while (r < PROB && level < MAX_LEVEL) {
            level++;
            r = get_random_prob();
        }
        return level;
    }

};

template<class T>
void TransactionalSkiplist<T>::add(T val) {
    EncounterModeTx<SkiplistNode<T> *> Tx;
    bool done = false;

    while (!done) {
        SkiplistNode<T> *n = nullptr;
        try {
            Tx.begin();

            SkiplistNode<T> *curr = Tx.read(&head);
            SkiplistNode<T> *update[MAX_LEVEL + 1];
            memset(update, 0, sizeof(SkiplistNode<T>*)*(MAX_LEVEL + 1));

            for (int i = l; i >= 0; i--) {
                while (Tx.read(&curr->neighbours[i]) != NULL && 
                       curr->neighbours[i]->value < val)
                    curr = Tx.read(&curr->neighbours[i]);
                update[i] = curr;
            }

            curr = Tx.read(&curr->neighbours[0]);

            if (curr == NULL || curr->value != val) {
                int h = get_random_height();

                if (h > l) {
                    for (int i = l + 1; i < h + 1; i++)
                        update[i] = Tx.read(&head);

                    l = h;
                }

                n = new SkiplistNode<T>(val, h);

                for (int i = 0; i <= h; i++) {
                    Tx.write(&n->neighbours[i], update[i]->neighbours[i]);
                    Tx.write(&update[i]->neighbours[i], n);
                }
            }

            done = Tx.commit();
        }
        catch (typename EncounterModeTx<SkiplistNode<T>*>::AbortException &e) {
            Tx.abort();
            done = false;
            if (n) delete n;
        }
    }

    
}

#endif