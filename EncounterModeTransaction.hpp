#ifndef ENCOUNTER_MODE_TRANSACTION_HPP
#define ENCOUNTER_MODE_TRANSACTION_HPP

#include "LockTable.hpp"
#include "Descriptor.hpp"
#include "Includes.hpp"

#ifdef DEBUG
#define TRACE(_x)   do {                                        \
                        TOUT << name << " " << _x << std::endl; \
                    } while(0)
#else
#define TRACE(_x) (void)(_x)
#endif

class EncounterModeTransaction {
public:

    inline void begin(/* jmp_buf *b */) {
        /* D.chk = b; */
        D.start_time = tx_clock;

       TRACE("STARTED");
    };

    inline void write(uintptr_t *addr, uintptr_t val) {
        /* save previous value at addr */
        D.writes.insert_or_assign(addr, *addr);

        /* save new value */
        D.writes.insert_or_assign(addr, val);

        /* acquire lock for addr */
        std::atomic<uint64_t> *lock = LT.get_lock(addr);
        uint64_t prev = *lock;

        if (((prev & 1) == 0) && (prev <= D.start_time)) {
            if (!lock->compare_exchange_strong(prev, D.my_lock))
                abort();

            D.locks.insert_or_assign(lock, prev);
        }
        else if (prev != D.my_lock) {
            abort();
        }

        /* store new value */
        std::atomic_ref<uintptr_t>(*addr).store(val, std::memory_order_release);
    };

    inline int read(uintptr_t *addr) {
        std::atomic<uint64_t> *l = LT.get_lock(addr);

        /* check if lock for addr is taken and is held by us */
        if ((*l & 1) && (D.locks.count(l) == 0)) {
            abort();
            return -1;
        }

        /* else read value */
        uintptr_t val = std::atomic_ref<uintptr_t>(*addr).load(std::memory_order_acquire);

        D.reads.push_back(l);
        return val;
    };

    inline void commit() {
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

        D.writes.clear();
        D.reads.clear();
        D.locks.clear();

        TRACE("COMMITTED");
    };

    inline void abort() {
        for (auto l : D.locks)
            *l.first = l.second;

        D.writes.clear();
        D.reads.clear();
        D.locks.clear();

        TRACE("ABORTED");
    };

    EncounterModeTransaction(std::string n) : name("ENCOUNTER MODE TRANSACTION " + n) {};

private:
    LockTable LT;
    std::string name;

    class Descriptor {
    public:
        jmp_buf *chk;
        uint64_t my_lock;
        uint64_t start_time;
        std::unordered_map< uintptr_t *, uintptr_t > writes;
        std::vector< std::atomic<uint64_t> *> reads;
        std::map< std::atomic<uint64_t> *, uint64_t > locks;

        Descriptor() : my_lock(((id_gen++) << 1) | 1) {}
    } D;

};

#endif
