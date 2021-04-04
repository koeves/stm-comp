#include "Transaction.hpp"
#include "NonblockingTransaction.hpp"
#include "BlockingTransaction.hpp"
#include "Includes.hpp"
#include "LockTable.hpp"

#include <thread>
#include <string>
#include <cstdlib>  
#include <ctime> 

unsigned long *TABLE;

#define RAND_ENTRY rand() % 1024

void thread_work(std::string t) {
    BlockingTransaction BT(t);

    BT.begin();

    for (int i = 0; i < 1000; ++i)
        BT.write(&TABLE[i], i);

    BT.commit();
}

int main() {

    TABLE = (unsigned long *)calloc(1024, sizeof(long));

    srand((unsigned)time(0));

    std::vector<std::thread> threads(4);

    for (int i = 0; i < 4; i++) {
        threads.at(i) = std::thread(thread_work, std::to_string(i));
    }
    
    for (int i = 0; i < 4; i++) {
        threads.at(i).join();
    }

    for (int i = 0; i < 1024; i++) {
        std::cout << TABLE[i] << " ";
    }

    std::cout << std::endl;

    free(TABLE);
    

    return 0;
}