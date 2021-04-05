#ifndef DESCRIPTOR_HPP
#define DESCRIPTOR_HPP

#include "Includes.hpp"

std::atomic<uint64_t> id_gen {1};
std::atomic<uint64_t> tx_clock {0};
std::atomic<uint64_t> lock_table[1024];

#endif