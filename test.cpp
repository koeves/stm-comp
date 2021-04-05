#include "CommitModeTransaction.hpp"
#include "EncounterModeTransaction.hpp"
#include <thread>
#include <string>
#include <cstdlib>  
#include <ctime> 

unsigned long *TABLE;

#define RAND_ENTRY rand() % 8

void thread_work_commit_mode(int id) {
    CommitModeTransaction CT(std::to_string(id));

    CT.begin();

    for (int i = 0; i < 8; ++i) {
        CT.write(&TABLE[RAND_ENTRY], id);
    }
    
    CT.commit();
}

void thread_work_encounter_mode(int id) {
    EncounterModeTransaction ET(std::to_string(id));

    ET.begin();
    
    for (int i = 0; i < 8; ++i) {
        ET.write(&TABLE[RAND_ENTRY], id);
    }

    ET.commit();
}

int main() {

    TABLE = (unsigned long *)calloc(8, sizeof(long));

    srand((unsigned)time(0));

    std::vector<std::thread> threads(4);

    for (int i = 0; i < 4; i++) {
        threads.at(i) = std::thread(thread_work_encounter_mode, i+1);
    }
    
    for (int i = 0; i < 4; i++) {
        threads.at(i).join();
    }

    for (int i = 0; i < 8; i++) {
        std::cout << TABLE[i] << " ";
    }

    std::cout << std::endl;

    free(TABLE);
    

    return 0;
}