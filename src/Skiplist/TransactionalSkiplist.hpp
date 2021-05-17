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

    using AbortException = typename EncounterModeTx<SkiplistNode<T>*>::AbortException;

    TransactionalSkiplist() :
        head(new SkiplistNode<T>(-1, MAX_LEVEL)), 
        level(0)
    {}

    ~TransactionalSkiplist() { 
        SkiplistNode<T> *node = head->neighbours[0];
        while (node != NULL) {
            SkiplistNode<T> *old = node;
            node = node->neighbours[0];
            delete old;
        }
        delete head;
    }

    void add(T val) { add(new SkiplistNode<T>(val, get_random_height())); }

    void display() {
        for (int i = level; i >= 0; i--) {
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

    SkiplistNode<T> *head;
    std::atomic<int> level;

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

    void add(SkiplistNode<T> *n) {
        EncounterModeTx<SkiplistNode<T> *> Tx;
        bool done = false;

        while (!done) {
            int old_level = 0;
            try {
                Tx.begin();

                old_level = level;

                SkiplistNode<T> *curr = Tx.read(&head);
                SkiplistNode<T> *update[MAX_LEVEL + 1];
                memset(update, 0, sizeof(SkiplistNode<T>*)*(MAX_LEVEL + 1));

                for (int i = level; i >= 0; i--) {
                    SkiplistNode<T> *next = Tx.read(&curr->neighbours[i]);
                    while (next != NULL && next->value < n->value) {
                        curr = next;
                        next = Tx.read(&next->neighbours[i]);
                    }
                    update[i] = curr;
                }

                curr = Tx.read(&curr->neighbours[0]);

                if (curr == NULL || curr->value != n->value) {
                    int h = n->height;

                    if (h > level) {
                        for (int i = level + 1; i < h + 1; i++)
                            update[i] = Tx.read(&head);

                        level = h;
                    }

                    for (int i = 0; i <= h; i++) {
                        Tx.write(&n->neighbours[i], update[i]->neighbours[i]);
                        Tx.write(&update[i]->neighbours[i], n);
                    }
                }

                done = Tx.commit();
            }
            catch (AbortException& e) {
                Tx.abort();
                done = false;
                level = old_level;
            }
        }
    }

};

#endif