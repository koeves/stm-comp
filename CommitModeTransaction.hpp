#ifndef COMMIT_MODE_TRANSACTION_HPP
#define COMMIT_MODE_TRANSACTION_HPP

#include "LockTable.hpp"
#include "Descriptor.hpp"
#include "Includes.hpp"

class CommitModeTransaction {
public:

    inline void begin(/* jmp_buf *b */) {
        /* D.chk = b; */
        D.start_time = tx_clock;

        debug_print("STARTED");
    };

    inline void write(uintptr_t *addr, uintptr_t val) {
        D.writes.insert_or_assign(addr, val);
    };

    inline int read(uintptr_t *addr) {
        /* check if addr is in write set and load it if it is */
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

    inline void commit() {
        if (D.writes.empty()) {
            D.reads.clear();
            return;
        }

        for (auto &l : D.writes) {
            std::atomic<uint64_t> *lock = LT.get_lock(l.first);
            uint64_t prev = *lock;

            if (((prev & 1) == 0) && (prev <= D.start_time)) {
                if (!lock->compare_exchange_strong(prev, D.my_lock))
                    abort();

                D.locks.push_back({lock, prev});
            }
            else if (prev != D.my_lock) {
                abort();
            }
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

        debug_print("COMMITED");
    };

    inline void abort() {
        for (auto l : D.locks)
            *l.first = l.second;

        D.writes.clear();
        D.reads.clear();
        D.locks.clear();

        debug_print("ABORTED");
    };

    CommitModeTransaction(std::string n) : name("COMMIT MODE TRANSACTION " + n) {};

private:
    LockTable LT;
    std::string name;

    inline void debug_print(std::string s) {
        TOUT << name << " " + s << std::endl;
    }

    class Descriptor {
    public:
        jmp_buf *chk;
        uint64_t my_lock;
        uint64_t start_time;
        std::unordered_map< uintptr_t *, uintptr_t > writes;
        std::vector< std::atomic<uint64_t> *> reads;
        std::vector< std::pair< std::atomic<uint64_t> *, uint64_t > > locks;


        Descriptor() : my_lock(((id_gen++) << 1) | 1) {}
    } D;
};

#endif
