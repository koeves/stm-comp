#ifndef LOCK_TABLE_HPP
#define LOCK_TABLE_HPP

#include "Includes.hpp"

#define NUM_LOCKS 1024
#define GRAIN 3

typedef std::atomic<uint64_t> atomic64u_t;

class LockTable {
public:
    inline atomic64u_t *get_lock(void *addr) {
        return &lock_table[(((uintptr_t)addr) >> GRAIN) % NUM_LOCKS];
    }

private:
    atomic64u_t lock_table[NUM_LOCKS];
};

#endif