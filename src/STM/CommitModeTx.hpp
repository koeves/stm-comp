/*
 *  Commit-order STM implementation
 *
 *     based on and adapted from the TL2 algorithm described in:
 *         Maurice Herlihy and Nir Shavit. The Art of Multiprocessor Programming
 *         Dice, David & Shalev, Ori & Shavit, Nir. (2006). Transactional Locking II. 194-208. 10.1007/11864219_14. 
 */

#ifndef COMMIT_MODE_TRANSACTION_HPP
#define COMMIT_MODE_TRANSACTION_HPP

#include "Transaction.hpp"
#include "Util.hpp"
#include "Orec.hpp"

#define NUM_LOCKS 1024
#define GRAIN 3

template<class T = uintptr_t>
class CommitModeTx : Transaction<T> {

public:

    inline void begin() override {
        TRACE("ETx " + std::to_string(id) + " STARTED");
        start_time = clock;
    }

    inline void write(T *addr, T val) override {
        writes.insert_or_assign(addr, val);
    }

    inline T read(T *addr) override {
        auto it = writes.find(addr);
        if (it != writes.end())
            return (*it).second;

        std::atomic<uint64_t> *l = get_lock(addr);
        uint64_t pre = *l;
        T val = std::atomic_ref<T>(*addr).load(std::memory_order_acquire);
        uint64_t post = *l;

        if ((pre & 1) || (pre != post) || (pre > start_time))
            throw AbortException();

        reads.push_back(l);

        return val;
    };

    inline bool commit() override {
        if (writes.empty()) {
            reads.clear();
            return true;
        }

        for (auto &l : writes) {
            std::atomic<uint64_t> *lock = get_lock(l.first);
            uint64_t prev = *lock;
            if (((prev & 1) == 0) && (prev <= start_time)) {
                if (!lock->compare_exchange_strong(prev, my_lock))
                    throw AbortException();
                locks.push_back({lock, prev});
            }
            else if (prev != my_lock) {
                throw AbortException();
            }
        }

        uint64_t end_time = ++clock;
        if (end_time != start_time + 1) {
            for (auto l : reads) {
                uint64_t v = *l;
                if (((v & 1) && (v != my_lock)) || (((v & 1) == 0) && (v > start_time)))
                    throw AbortException();
            }
        }

        for (auto w : writes)
            std::atomic_ref<T>(*w.first).store(w.second, std::memory_order_release);

        for (auto l : locks)
            *l.first = end_time;

        writes.clear();
        locks.clear();
        reads.clear();

        TRACE("ETx " + std::to_string(id) + " COMMITTED");
        return true;
    };

    inline void abort() override {
        for (auto l : locks)
            *l.first = l.second;

        reads.clear();
        writes.clear();
        locks.clear();

        TRACE("ETx " + std::to_string(id) + " ABORTED");
    };

    inline int get_id() const { return id; };

    struct AbortException {};

    CommitModeTx() {
        id = id_gen++;
        my_lock = ((id << 1) | 1);
    };

private:

    int id;
    uint64_t my_lock, start_time;
    std::unordered_map<T *, T> writes;
    std::vector<std::pair<std::atomic<uint64_t> *, uint64_t>> locks;
    std::vector<std::atomic<uint64_t>*> reads;

    static inline std::atomic<uint64_t> id_gen {1};
    static inline std::atomic<uint64_t> clock {0};
    static inline std::atomic<uint64_t> lock_table[NUM_LOCKS];

    static inline std::atomic<uint64_t> *get_lock(void *addr) {
        return &lock_table[(((uintptr_t)addr) >> GRAIN) % NUM_LOCKS];
    }

};

#endif
