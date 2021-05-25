#include <thread>
#include <cmath>

#include "../RedBlackTree/TransactionalRBTree.hpp"

TransactionalRBTree<> TRB;

const int N = 2000;

void trb_insert(int id, int n) {
    int a = id * n, b = (id + 1) * n;
    for (int i = a; i < b; ++i)
        TRB.insert(i);
}

int main(int argc, char **argv) {
    int nthreads = atoi(argv[1]);
    int n = std::floor(N/nthreads); 

    std::vector<std::thread> threads(nthreads);
    
    for (unsigned i = 0; i < threads.size(); i++) {
        threads.at(i) = std::thread(trb_insert, i, n);
    }

    for (unsigned i = 0; i < threads.size(); i++) {
        threads.at(i).join();
    }
    return 0;
}