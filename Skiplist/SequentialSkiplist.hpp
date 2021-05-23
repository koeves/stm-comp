#ifndef SEQ_SKIP_HPP
#define SEQ_SKIP_HPP

#include <random>
#include <iostream>
#include "SkiplistNode.hpp"

template<class T = int>
class SequentialSkiplist {
public:

    SequentialSkiplist() : 
        head(new SkiplistNode<T>(-1, MAX_LEVEL)),  
        level(0)
    {}

    ~SequentialSkiplist() { 
        SkiplistNode<T> *node = head->neighbours[0];
        while (node != NULL) {
            SkiplistNode<T> *old = node;
            node = node->neighbours[0];
            delete old;
        }
        delete head; 
    }

    bool contains(T val);

    void add(T val);

    bool remove(T val);

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
    int level;

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
        int lvl = 0;
        while (r < PROB && lvl < MAX_LEVEL) {
            lvl++;
            r = get_random_prob();
        }
        return lvl;
    }

};

template<class T>
void SequentialSkiplist<T>::add(T val) {
    SkiplistNode<T> *curr = head;
    SkiplistNode<T> *update[MAX_LEVEL + 1];
    memset(update, 0, sizeof(SkiplistNode<T>*)*(MAX_LEVEL + 1));

    for (int i = level; i >= 0; i--) {
        while (curr->neighbours[i] != NULL && curr->neighbours[i]->value < val)
            curr = curr->neighbours[i];
        update[i] = curr;
    }

    curr = curr->neighbours[0];

    if (curr == NULL || curr->value != val) {
        int h = get_random_height();

        if (h > level) {
            for (int i = level + 1; i < h + 1; i++)
                update[i] = head;

            level = h;
        }

        SkiplistNode<T> *n = new SkiplistNode<T>(val, h);

        for (int i = 0; i <= h; i++) {
            n->neighbours[i] = update[i]->neighbours[i];
            update[i]->neighbours[i] = n;
        }
    }
}


#endif