/*
 *  Encounter-order STM implementation
 *
 *     based on and adapted from the encounter-order STM implementation of Dice and Shavit:
 *         Dice, D., & Shavit, N. (2006). What Really Makes Transactions Faster. 
 * 
 */

#ifndef ENCOUNTER_MODE_TRANSACTION_HPP
#define ENCOUNTER_MODE_TRANSACTION_HPP

#include "Transaction.hpp"
#include "Util.hpp"
#include "TransactionPool.hpp"
#include "Orec.hpp"

#define NUM_LOCKS 1024
#define GRAIN 3

template<class T = uintptr_t>
class EncounterModeTx : Transaction<T> {

public:

    inline void begin() override {
        TRACE("ETx " + std::to_string(id) + " STARTED");
    };

    inline bool write(T *addr, T val) override {
        /* save previous value at addr if not already in map */
        prev_values.try_emplace(addr, *addr);

        /* acquire orec for addr */
        Orec *O = get_orec(addr);

        if (!O->is_locked()) {
            if (!O->lock(O->get_orec(), id)) {
                TRACE("\tTx " + std::to_string(id) + " COUDLN'T LOCK ADDR ");
                abort();
                return false;
            }

            orecs.insert(O);
        }
        else if (O->get_owner() != id) {
            TRACE("\tTx " + std::to_string(id) + " ADDR OWNED BY Tx " + std::to_string(O->get_owner()));
            abort();
            return false;
        }

        /* store new value */
        std::atomic_ref<T>(*addr).store(val, std::memory_order_release);
        writes.push_back({O, O->get_version(id)});

        return true;
    };

    inline T read(T *addr) override {
        Orec *O = get_orec(addr);

        /* check if orec for addr is taken and is not held by us */
        if (orecs.count(O) == 0) {
            if (O->is_locked()) {
                /* spin while orec is locked */

                /* 
                 * TODO: cyclic dependency breaks here
                 * If Tx 1 owns Rec 1 and wants to write Rec 2 while
                 *    Tx 2 owns Rec 2 and wants to write Rec 1
                 */
                while(O->is_locked());
                reads.push_back({O, O->get_version()});
            }
        }
        else {
            reads.push_back({O, O->get_version(id)});
        }

        /* orec is unlocked, read value */
        
        return std::atomic_ref<T>(*addr).load(std::memory_order_acquire);
    };

    inline bool commit() override {
        /* validate read-set */
        /* if (!validate_read_set()) {
            goto abort;
        } */

        clear_and_release();

        TRACE("ETx " + std::to_string(id) + " COMMITTED");

        return true;

    /* abort:
        abort();
        return false; */
    };

    inline void abort() override {
        unroll_writes();
        clear_and_release();

        TRACE("ETx " + std::to_string(id) + " ABORTED");
    };

    inline int get_id() const { return id; };

    EncounterModeTx() : id(id_gen++) {};

private:

    int id;
    std::unordered_map<T *, T> prev_values;
    std::vector<std::pair<Orec *, uint64_t>> reads, writes;
    std::unordered_set<Orec *> orecs;

    static inline std::atomic<uint64_t> id_gen {1};

    static inline Orec orec_table[NUM_LOCKS];

    static inline Orec *get_orec(void *addr) {
        return &orec_table[(((uintptr_t)addr) >> GRAIN) % NUM_LOCKS];
    }

    /* 
     * !!! O(n^2) validation !!!
     * 
     * might not even be necessary for encounter mode 
     * 
     * if orec is both in write and read sets,
     * check if the versions match
     */
    inline bool validate_read_set() {
        for (auto R : reads) {
            for (auto W : writes) {
                if (R.first == W.first) {
                    /* check for version mismatch */
                    if (R.second != W.second)
                        return false;

                    /* we need only check the earliest occurrence */
                    else 
                        break;
                }
            }
        }
        return true;
    }

    inline void unroll_writes() {
        for (auto w : prev_values) {
            std::atomic_ref<T>(*w.first).store(w.second, std::memory_order_release);
        }
    }

    inline void clear_and_release() {
        for (Orec *O : orecs) 
            O->unlock();
        
        prev_values.clear();
        reads.clear();
        writes.clear();
        orecs.clear();
    }

};

#endif
