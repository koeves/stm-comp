/*
 *  Commit-time STM implementation
 *
 *     based on and adapted from the commit-time STM implementation of Dice and Shavit:
 *         Dice, D., & Shavit, N. (2006). What Really Makes Transactions Faster. 
 * 
 */

#ifndef TLCM_TRANSACTION_HPP
#define TLCM_TRANSACTION_HPP

#include "Transaction.hpp"
#include "../Utilities/Util.hpp"
#include "../Utilities/AtomicRef.hpp"
#include "Orec.hpp"

template<class T = uintptr_t>
class TLCommitModeTx : Transaction<T> {

public:

    inline void begin() override {
        TRACE("TLCTx " + std::to_string(id) + " STARTED");
    }

    inline void write(T *addr, T val) override {
        writes.insert_or_assign(addr, val);
    }

    inline T read(T *addr) override {
        if (writes.count(addr))
            return writes.at(addr);

        Orec *O = get_orec(addr);

        if (O->is_locked()) {
            TRACE("\tTLTx " + std::to_string(id) + " READ ADDR LOCKED");
            throw AbortException();
        }

        reads.push_back({O, O->get_version()});

        /* orec is unlocked, read value */
        return ATOMIC_LOAD(T, addr);
    }

    inline bool commit() override {  
        if (writes.empty()) {
            reads.clear();
            return true;
        }

        for (auto w : writes) {
            Orec *O = get_orec(w.first);
            bool set = false;
            for (auto r : reads) {
                if (r.first == O) {
                    if (!O->lock(r.second, id)) {
                        TRACE("\tTLCTx " + std::to_string(id) + " READ-WRITE VERSION MISMATCH");
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                        throw AbortException();
                    }
                    orecs.insert(O);
                    set = true;
                    break;
                }
            }
            if (!set) {
                if (!O->lock(O->get_orec(), id)) {
                    TRACE("\tTLCTx " + std::to_string(id) + " COULDN'T LOCK ADDR OWNED BY Tx " + std::to_string(O->get_owner()));
                    throw AbortException();
                }
                orecs.insert(O);
            }
        }

        for (auto r : reads) {
            if (r.first->get_version() != r.second) {
                TRACE("\tTLCTx " + std::to_string(id) + " READSET VERSION CHANGED");
                throw AbortException();
            }
        }

        for (auto w : writes)
            ATOMIC_STORE(T, w.first, w.second);

        TRACE("TLCTx " + std::to_string(id) + " COMMITTED");

        clear_and_release();

        num_retries = 0;

        return true;
    }

    inline void abort() override {
        clear_and_release();
        TRACE("TLCTx " + std::to_string(id) + " ABORTED");
        num_retries++;
        int r = random_wait();
        TRACE("\tTLCTx " + std::to_string(id) + " SLEEPS " + std::to_string(r) + " MS");
        std::this_thread::sleep_for(std::chrono::microseconds(r));
    }

    inline int get_id() const { return id; };

    struct AbortException {};

    TLCommitModeTx() : id(TLCommitModeTx::id_gen++), num_retries(0) {};

private:
    static const int NUM_LOCKS = 2048;
    static const int GRAIN = 3;
    static inline std::atomic<uint64_t> id_gen {1};
    static inline Orec orec_table[NUM_LOCKS];
    static inline Orec *get_orec(void *addr) {
        return &TLCommitModeTx::orec_table[(((uintptr_t)addr) >> GRAIN) % NUM_LOCKS];
    }

    int id, num_retries;
    std::vector<std::pair<Orec *, uint64_t>> reads;
    std::unordered_map<T *, T> writes;
    std::unordered_set<Orec *> orecs;

    inline void clear_and_release() {
        for (Orec *O : orecs) 
            O->unlock();
        
        reads.clear();
        writes.clear();
        orecs.clear();
    }

    inline int random_wait() {
        std::random_device rd;
        std::mt19937 mt(rd());
        std::uniform_real_distribution<> dist(0, 1000);

        int w = dist(mt);

        return w;
    }

};

#endif
