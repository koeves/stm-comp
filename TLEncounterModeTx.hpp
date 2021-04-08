#ifndef ENCOUNTER_MODE_TRANSACTION_HPP
#define ENCOUNTER_MODE_TRANSACTION_HPP

#include "Transaction.hpp"
#include "Util.hpp"
#include "TransactionPool.hpp"
#include "Orec.hpp"

class TLEncounterModeTx : Transaction {
public:

    inline void begin() override {
        TRACE("ETx " + std::to_string(id) + " STARTED");
    };

    inline bool write(uintptr_t *addr, uintptr_t val) override {
        if (is_aborted) return false;

        /* save previous value at addr if not already in map */
        writes.try_emplace(addr, *addr);

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
        std::atomic_ref<uintptr_t>(*addr).store(val, std::memory_order_release);

        return true;
    };

    inline uintptr_t read(uintptr_t *addr) override {
        if (is_aborted) return 0;

        Orec *O = get_orec(addr);

        /* check if orec for addr is taken and is not held by us */
        if ((O->is_locked()) && (orecs.count(O) == 0)) {
            /* spin while orec is locked */
            while(O->is_locked());
        }

        /* orec is unlocked, read value */
        reads.push_back({O, O->get_version()});
        return std::atomic_ref<uintptr_t>(*addr).load(std::memory_order_acquire);
    };

    inline bool commit() override {
        if (is_aborted) return false;

        /* validate read-set */
        if (!validate_read_set()) {
            goto abort;
        }

        clear_and_release();

        TRACE("ETx " + std::to_string(id) + " COMMITTED");

        return true;

    abort:
        abort();
        return false;
    };

    inline void abort() override {
        is_aborted = true;

        unroll_writes();
        clear_and_release();

        TRACE("ETx " + std::to_string(id) + " ABORTED");
    };

    inline int get_id() const { return id; };

    TLEncounterModeTx() : id(id_gen++) {};

private:
    int id;
    bool is_aborted = false;

    std::unordered_map<uintptr_t *, uintptr_t> writes;
    std::vector<std::pair<Orec *, uintptr_t>> reads;
    std::unordered_set<Orec *> orecs;

    inline bool validate_read_set() {
        for (auto R : reads) {
            Orec *O = R.first;

            /* if Orec's version changed in the meanwhile */
            if (O->get_version() != R.second)
                return false;

            /* if we hold the orec but read a different version */
            if (orecs.count(O)) {
                if (O->get_version() != R.second)
                    return false;
            }
        }
        return true;
    }

    inline void unroll_writes() {
        for (auto w : writes) {
            std::atomic_ref<uintptr_t>(*w.first).store(w.second, std::memory_order_release);
        }
    }

    inline void clear_and_release() {
        for (Orec *O : orecs) {
            O->unlock();
        }

        writes.clear();
        reads.clear();
        orecs.clear();
    }

};

#endif
