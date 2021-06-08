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
        start = std::chrono::steady_clock::now();
        TRACE("ETx "  << id <<  " STARTED");
        assert(reads.empty());
        assert(prev_values.empty());
        assert(prev_ints.empty());
        assert(orecs.empty());
        end = std::chrono::steady_clock::now();
        std::cout << "BEGIN: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << '\n';
    }

    inline void write(T *addr, T val) override {
        start = std::chrono::steady_clock::now();
        /* acquire orec for addr */
        Orec *O = get_orec(addr);

        if (!O->is_locked()) {
            if (!O->lock(O->get_orec(), id)) {
                TRACE("\tTx "  << id <<  " COUDLN'T LOCK ADDR ");
                throw AbortException();
            }

            orecs.insert(O);
        }
        else if (O->get_owner() != id) {
            TRACE("\tTx "  << id <<  " ADDR OWNED BY Tx " << O->get_owner());
            throw AbortException();
        }

        /* save previous value at addr if not already in map */
        prev_values.try_emplace(addr, *addr);

        if (!validate_read_set()) throw AbortException();

        /* store new value */
        ATOMIC_STORE(T, addr, val);

        end = std::chrono::steady_clock::now();
        std::cout << "WRITE: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << '\n';
    }

    inline void write(int *addr, int val) {
        /* acquire orec for addr */
        Orec *O = get_orec(addr);

        if (!O->is_locked()) {
            if (!O->lock(O->get_orec(), id)) {
                TRACE("\tTx "  << id <<  " COUDLN'T LOCK ADDR ");
                throw AbortException();
            }

            orecs.insert(O);
        }
        else if (O->get_owner() != id) {
            TRACE("\tTx "  << id <<  " ADDR OWNED BY Tx " << O->get_owner());
            throw AbortException();
        }

        /* save previous value at addr if not already in map */
        prev_ints.try_emplace(addr, *addr);

        if (!validate_read_set()) throw AbortException();

        /* store new value */
        ATOMIC_STORE(int, addr, val);
    }

    inline T read(T *addr) override {
        start = std::chrono::steady_clock::now();
        Orec *O = get_orec(addr);

        /* check if orec for addr is taken and is not held by us */
        if (orecs.count(O) == 0) {
            if (O->is_locked()) {
                throw AbortException();
            }
        }
        else {
            reads.push_back({O, O->get_version()});
        }

        if (!validate_read_set()) throw AbortException();

        /* orec is unlocked, read value */
        end = std::chrono::steady_clock::now();
        std::cout << "READ: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << '\n';
        return ATOMIC_LOAD(T, addr);
    }

    inline int read(int *addr) {
        Orec *O = get_orec(addr);

        if (orecs.count(O) == 0) {
            if (O->is_locked()) 
                throw AbortException();
        }
        else {
            reads.push_back({O, O->get_version()});
        }

        if (!validate_read_set()) throw AbortException();

        return ATOMIC_LOAD(int, addr);
    }

    inline bool commit() override {
        start = std::chrono::steady_clock::now();
        if (!validate_read_set()) throw AbortException();
        clear_and_release();
        num_retries = 0;

        TRACE("ETx "  << id <<  " COMMITTED");

        end = std::chrono::steady_clock::now();

        TRACE("\tETx "  << id <<  " TOOK " 
            << std::to_string(std::chrono::duration_cast<std::chrono::microseconds>(end - start).count())
            << " ms"
        );
        end = std::chrono::steady_clock::now();
        std::cout << "COMMIT: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << '\n';

        return true;
    }

    inline void abort() override {
        start = std::chrono::steady_clock::now();
        unroll_writes();
        clear_and_release();

        num_retries++;
        end = std::chrono::steady_clock::now();
        std::cout << "ABORT: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << '\n';

    }

    inline int get_id() const { return id; };

    struct AbortException {};

    EncounterModeTx() : 
        id(EncounterModeTx::id_gen++), 
        num_retries(0),
        start(std::chrono::steady_clock::now())
    {};

private:
    static const int NUM_LOCKS = 2048;
    static const int GRAIN     = 3;
    static inline std::atomic<uint64_t> id_gen {1};
    static inline Orec orec_table[NUM_LOCKS];
    static inline Orec *get_orec(void *addr) {
        return &EncounterModeTx::orec_table[(((uintptr_t)addr) >> GRAIN) % NUM_LOCKS];
    }

    int id, num_retries;
    std::unordered_map<T*, T> prev_values;
    std::unordered_map<int*, int> prev_ints;
    std::vector<std::pair<Orec *, uint64_t>> reads;
    std::unordered_set<Orec *> orecs;

    std::chrono::steady_clock::time_point start, end;

    inline void unroll_writes() {
        for (auto w : prev_values) {
            if (w.first) ATOMIC_STORE(T, w.first, w.second);
        }
        for (auto w : prev_ints) {
            if (w.first) ATOMIC_STORE(int, w.first, w.second);
        }
    }

    inline void clear_and_release() {
        for (Orec *O : orecs) 
            O->unlock();
        
        prev_values.clear();
        prev_ints.clear();
        reads.clear();
        orecs.clear();
    }

    inline int random_wait() {
        std::random_device rd;
        std::mt19937 mt(rd());
        std::uniform_real_distribution<> dist(0, 10);

        int w = dist(mt);

        return w;
    }

    inline bool validate_read_set() {
        for (auto r : reads) {
            if (r.first->get_version() != r.second) {
                TRACE("\tTLCTx "  << id <<  " READSET VERSION CHANGED");
                return false;
            }
        }
        return true;
    }

};

#endif
