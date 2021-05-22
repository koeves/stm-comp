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

template<class T = uintptr_t>
class EncounterModeTx : Transaction<T> {

public:

    inline void begin() override {
        TRACE("ETx " + std::to_string(id) + " STARTED");
    };

    inline void write(T *addr, T val) override {
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

        /* save previous value at addr if not already in map */
        prev_values.try_emplace(addr, *addr);

        /* store new value */
        ATOMIC_STORE(T, addr, val);
        writes.push_back({O, O->get_version()});
    };

    inline void write(int *addr, int val) {
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

        /* save previous value at addr if not already in map */
        prev_ints.try_emplace(addr, *addr);

        /* store new value */
        ATOMIC_STORE(int, addr, val);
        writes.push_back({O, O->get_version()});
    };

    inline T read(T *addr) override {
        Orec *O = get_orec(addr);

        /* check if orec for addr is taken and is not held by us */
        if (orecs.count(O) == 0) {
            if (O->is_locked()) {
                /* spin while orec is locked */
                //while(O->is_locked());
                //reads.push_back({O, O->get_version()});

                /* or abort */
                throw AbortException();
            }
        }
        else {
            reads.push_back({O, O->get_version()});
        }

        /* orec is unlocked, read value */
        return ATOMIC_LOAD(T, addr);
    };

    inline int read(int *addr) {
        Orec *O = get_orec(addr);

        if (orecs.count(O) == 0) {
            if (O->is_locked()) 
                throw AbortException();
        }
        else {
            reads.push_back({O, O->get_version()});
        }

        return ATOMIC_LOAD(int, addr);
    };

    inline bool commit() override {
        for (auto r : reads) {
            if (r.first->get_version() != r.second) {
                TRACE("\tETx " + std::to_string(id) + " SAW INCONSISTENT READ");
                throw AbortException();
            }
        }  
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
    static const int NUM_LOCKS = 2048;
    static const int GRAIN     = 3;
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

    inline void unroll_writes() {
        for (auto w : prev_values)
            ATOMIC_STORE(T, w.first, w.second);
        
        for (auto i : prev_ints) 
            ATOMIC_STORE(int, i.first, i.second);
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
