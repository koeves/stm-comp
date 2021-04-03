#ifndef DESCRIPTOR_HPP
#define DESCRIPTOR_HPP

#include "Includes.hpp"

std::atomic<uint64_t> id_gen {1};
std::atomic<uint64_t> tx_clock {0};
std::atomic<uint64_t> lock_table[1024];

class Descriptor {

public:
    jmp_buf *chk;
    uint64_t my_lock;
    uint64_t start_time;
    std::unordered_map< uintptr_t *, uintptr_t > writes;
    std::vector< std::atomic<uint64_t> *> reads;
    std::vector< std::pair< std::atomic<uint64_t> *, uint64_t > > locks;


    Descriptor() : my_lock(((id_gen++) << 1) | 1) {}
};

#endif