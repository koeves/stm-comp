#ifndef SEQ_SKIP_HPP
#define SEQ_SKIP_HPP

#include "SkiplistNode.hpp"

template<class T = int>
class SequentialSkiplist {

public:

    SequentialSkiplist() {
        head = new SkiplistNode<T>(1);
    }

    ~SequentialSkiplist() {
        delete head;
    }

    int get_height() {
        return head->get_height();
    }

    int get_count() {
        return count;
    }

    bool contains(T val);

    void add(T val);

    bool remove(T val);


private:

    SkiplistNode<T> *head;
    int count;

    static constexpr double prob = 0.5;



};

#endif