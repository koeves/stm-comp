#ifndef BLOCKINGTRANSACTION_HPP
#define BLOCKINGTRANSACTION_HPP

#include "LockTable.hpp"
#include "Descriptor.hpp"
#include "Includes.hpp"
#include <string>

class BlockingTransaction {
public:

    inline void begin(/* jmp_buf *b */) {
        /* D.chk = b; */
        D.start_time = tx_clock;

        TOUT << "THREAD " << id << " STARTED\n";
    };

    void write(uintptr_t *addr, uintptr_t val) {
        D.writes.insert_or_assign(addr, val);

        /* for (auto &l : D.writes) { */
        std::atomic<uint64_t> *lock = LT.get_lock(addr);
        uint64_t prev = *lock;

        if (((prev & 1) == 0) && (prev <= D.start_time)) {
            if (!lock->compare_exchange_strong(prev, D.my_lock))
                abort();

            D.locks.push_back({lock, prev});
        }
        else if (prev != D.my_lock) {
            abort();
        }
        /* } */
    };

    int read(uintptr_t *addr) { 
        auto it = D.writes.find(addr);
        if (it != D.writes.end())
            return (*it).second;

        std::atomic<uint64_t> *l = LT.get_lock(addr);
        uint64_t pre = *l;
        uintptr_t val = std::atomic_ref<uintptr_t>(*addr).load(std::memory_order_acquire);
        uint64_t post = *l;

        if ((pre & 1) || (pre != post) || (pre > D.start_time)) {
            abort();
            return -1;
        }

        D.reads.push_back(l);
        return val;
    };

    void commit() {
        if (D.writes.empty()) {
            D.reads.clear();
            return;
        }

        uint64_t end_time = ++tx_clock;

        if (end_time != D.start_time + 1) {
            for (auto i : D.reads) {
                uint64_t v = *i;

                if ((v & 1) || (v != D.my_lock) || ((v & 1) == 0 && (v > D.start_time))) {
                    abort();
                    return;
                }
            }
        }

        for (auto &w : D.writes)
            std::atomic_ref<uintptr_t>(*w.first).store(w.second, std::memory_order_release);
 
        /* for (auto l : D.locks)
            *l.first = D.end_time */

        D.writes.clear();
        D.reads.clear();
        D.locks.clear();

        TOUT << "THREAD " << id << " COMMITED\n";
    };

    void abort() {
        for (auto l : D.locks)
            *l.first = l.second;

        D.writes.clear();
        D.reads.clear();
        D.locks.clear();

        TOUT << "THREAD " << id << " ABORTED\n";
    };

    BlockingTransaction(int n) : id(n) {};

private:
    Descriptor D;
    LockTable LT;
    int id;
};

#endif