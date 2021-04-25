#ifndef TRANCATION_POOL_HPP
#define TRANSACTION_POOL_HPP

#include "Util.hpp"
#include "Orec.hpp"

#define NUM_LOCKS 1024
#define GRAIN 3

std::atomic<uint64_t> id_gen {1};

Orec orec_table[NUM_LOCKS];

inline Orec *get_orec(void *addr) {
    return &orec_table[(((uintptr_t)addr) >> GRAIN) % NUM_LOCKS];
}

#endif