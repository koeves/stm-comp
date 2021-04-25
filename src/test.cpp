#include "STM/TLEncounterModeTx.hpp"
#include "STM/Orec.hpp"
#include "RedBlackTree/RedBlackTree.hpp"
#include "RedBlackTree/AbstractTree.hpp"

#include <thread>
#include <string>
#include <cstdlib>  
#include <ctime> 

unsigned long *TABLE;

#define RAND_ENTRY rand() % 8

void thread_work_encounter_mode() {
    EncounterModeTx<> ET;
    int id = ET.get_id();

    ET.begin();
    
    for (int i = 0; i < 8; ++i) {
        int r = RAND_ENTRY;
        TOUT << "\tTx " << id << " WRITES TO ENTRY " << r << std::endl;

        int entry = ET.read(&TABLE[r]);

        if (entry % 2) {
            if (!ET.write(&TABLE[r], id)) {
                TOUT << "\tTx " << id << " FAILED TO WRITE ENTRY " << r << std::endl;
                goto restart;
            }
        }
        else {
            if (!ET.write(&TABLE[r], id)) {
                TOUT << "\tTx " << id << " FAILED TO WRITE ENTRY " << r << std::endl;
                goto restart;
            }
        }
    }

    if (!ET.commit()) {
        TOUT << "\tTx " << id << " FAILED TO COMMIT, RESTART " << std::endl;
        goto restart;
    }

    return;
    
restart:
    thread_work_encounter_mode();
    return;
}

void showbits( unsigned int x ) {
    int i=0;
    for (i = (sizeof(int) * 8) - 1; i >= 0; i--) {
        putchar(x & (1u << i) ? '1' : '0');
    }
    printf("\n");
}

    Orec O;

void bits(int i) {
/*
    std::cout << ((1 & 1) ? "SET" : "UNSET") << std::endl;
    std::cout << ((0 & 1) ? "SET" : "UNSET") << std::endl;
    std::cout << ((1 ^ 1) ? "SET" : "UNSET") << std::endl;
    std::cout << ((0 ^ 1) ? "SET" : "UNSET") << std::endl;
*/

    O.print();

    O.lock(O.get_orec(), i);
    O.print();

    O.unlock();
    O.print();
}

int main() {

    TABLE = (unsigned long *)calloc(8, sizeof(long));

    for (int i = 0; i < 8; ++i) TABLE[i] = i+1;

    srand((unsigned)time(0));

    std::vector<std::thread> threads(2);

    for (unsigned i = 0; i < threads.size(); i++) {
        threads.at(i) = std::thread(thread_work_encounter_mode);
    }
    
    for (unsigned i = 0; i < threads.size(); i++) {
        threads.at(i).join();
    }

    for (int i = 0; i < 8; i++) {
        std::cout << TABLE[i] << " ";
    }

    std::cout << std::endl;

    free(TABLE);

    /* RedBlackTree<> RBT;
    RBT.insert(14);
    RBT.insert(17);
    RBT.insert(21);
    RBT.insert(26);
    RBT.insert(30);
    RBT.insert(41);
    RBT.insert(48);


    RBT.print();
    std::cout<<std::endl;

    RBT.remove(41);

    RBT.print(); */

    return 0;
}