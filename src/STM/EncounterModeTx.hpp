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
#include "../Utilities/Util.hpp"
#include "../Utilities/AtomicRef.hpp"
#include "Orec.hpp"

#define NUM_LOCKS 1024
#define GRAIN 3

template<class T = uintptr_t>
class EncounterModeTx : Transaction<T> {

public:

    inline void begin() override {
        TRACE("ETx " + std::to_string(id) + " STARTED");
    };

    inline void write(T *addr, T val) override {
        /* save previous value at addr if not already in map */
        prev_values.try_emplace(addr, *addr);

        /* acquire orec for addr */
        Orec *O = get_orec(addr);

        if (!O->is_locked()) {
            if (!O->lock(O->get_orec(), id)) {
                TRACE("\tTx " + std::to_string(id) + " COUDLN'T LOCK ADDR ");
                throw AbortException();
            }

            orecs.insert(O);
        }
        else if (O->get_owner() != id) {
            TRACE("\tTx " + std::to_string(id) + " ADDR OWNED BY Tx " + std::to_string(O->get_owner()));
            throw AbortException();
        }

        /* store new value */
#if __GNUC__ > 9
        std::atomic_ref<T>(*addr).store(val, std::memory_order_release);
#else
        AtomicRef<T>(addr).store(val);
        //reinterpret_cast< std::atomic<T>& >(*addr).store(val, std::memory_order_release);
#endif
        writes.push_back({O, O->get_version(id)});
    };

    inline void write(int *addr, int val) {
        /* save previous value at addr if not already in map */
        prev_ints.try_emplace(addr, *addr);

        /* acquire orec for addr */
        Orec *O = get_orec(addr);

        if (!O->is_locked()) {
            if (!O->lock(O->get_orec(), id)) {
                TRACE("\tTx " + std::to_string(id) + " COUDLN'T LOCK ADDR ");
                throw AbortException();
            }

            orecs.insert(O);
        }
        else if (O->get_owner() != id) {
            TRACE("\tTx " + std::to_string(id) + " ADDR OWNED BY Tx " + std::to_string(O->get_owner()));
            throw AbortException();
        }

        /* store new value */
#if __GNUC__ > 9
        std::atomic_ref<int>(*addr).store(val, std::memory_order_release);
#else
        AtomicRef<int>(addr).store(val);
        //reinterpret_cast< std::atomic<int>& >(*addr).store(val, std::memory_order_release);
#endif
        writes.push_back({O, O->get_version(id)});
    };

    inline T read(T *addr) override {
        Orec *O = get_orec(addr);

        /* check if orec for addr is taken and is not held by us */
        if (orecs.count(O) == 0) {
            if (O->is_locked()) {
                /* spin while orec is locked */
                // while(O->is_locked());
                // reads.push_back({O, O->get_version()});

                /* or abort */
                throw AbortException();
            }
        }
        else {
            reads.push_back({O, O->get_version(id)});
        }

        /* orec is unlocked, read value */
#if __GNUC__ > 9
        return std::atomic_ref<T>(*addr).load(std::memory_order_acquire);
#else
        return AtomicRef<T>(addr).load();
        //return reinterpret_cast< std::atomic<T>& >(*addr).load(std::memory_order_acquire);
#endif
    };

    inline int read(int *addr) {
        Orec *O = get_orec(addr);

        if (orecs.count(O) == 0) {
            if (O->is_locked()) {
                throw AbortException();
            }
        }
        else {
            reads.push_back({O, O->get_version(id)});
        }

#if __GNUC__ > 9
        return std::atomic_ref<int>(*addr).load(std::memory_order_acquire);
#else
        return AtomicRef<int>(addr).load();
        //return reinterpret_cast< std::atomic<int>& >(*addr).load(std::memory_order_acquire);
#endif
    };

    inline bool commit() override {  
        if (!validate_read_set())
            throw AbortException();

        clear_and_release();

        TRACE("ETx " + std::to_string(id) + " COMMITTED");

        return true;
    };

    inline void abort() override {
        unroll_writes();
        clear_and_release();

        TRACE("ETx " + std::to_string(id) + " ABORTED");
    };

    inline int get_id() const { return id; };

    struct AbortException {};

    EncounterModeTx() : id(EncounterModeTx::id_gen++) {};

private:

    static inline std::atomic<uint64_t> id_gen {1};
    static inline Orec orec_table[NUM_LOCKS];
    static inline Orec *get_orec(void *addr) {
        return &EncounterModeTx::orec_table[(((uintptr_t)addr) >> GRAIN) % NUM_LOCKS];
    }

    int id;
    std::unordered_map<T*, T> prev_values;
    std::unordered_map<int*, int> prev_ints;
    std::vector<std::pair<Orec *, uint64_t>> reads, writes;
    std::unordered_set<Orec *> orecs;

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
#if __GNUC__ > 9
            std::atomic_ref<T>(*w.first).store(w.second, std::memory_order_release);
#else
            AtomicRef<T>(w.first).store(w.second);
            //reinterpret_cast< std::atomic<T>& >(*w.first).store(w.second, std::memory_order_release);
#endif
        }
        for (auto i : prev_ints) {
#if __GNUC__ > 9
            std::atomic_ref<int>(*i.first).store(i.second, std::memory_order_release);
#else
            AtomicRef<int>(i.first).store(i.second);
            //reinterpret_cast< std::atomic<int>& >(*i.first).store(i.second, std::memory_order_release);
#endif
        }
    }

    inline void clear_and_release() {
        for (Orec *O : orecs) 
            O->unlock();
        
        prev_values.clear();
        prev_ints.clear();
        reads.clear();
        writes.clear();
        orecs.clear();
    }

};

#endif
