#ifndef ENCOUNTER_MODE_TRANSACTION_HPP
#define ENCOUNTER_MODE_TRANSACTION_HPP

#include "Transaction.hpp"
#include "Util.hpp"
#include "TransactionPool.hpp"
#include "Orec.hpp"

class TLEncounterModeTx : Transaction {
public:

    inline void begin() override {
        TRACE("ETL " + std::to_string(id) + " STARTED");
    };

    inline void write(uintptr_t *addr, uintptr_t val) override {
        /* save previous value at addr */
        writes.insert_or_assign(addr, *addr);

        /* save new value */
        writes.insert_or_assign(addr, val);

        /* acquire orec for addr */
        Orec *O = get_orec(addr);

        if (!O->is_locked()) {
            if (!O->lock(O->get_orec(), id))
                abort();

            orecs.insert(O);
        }
        else if (O->get_owner() != id) {
            abort();
        }

        /* store new value */
        std::atomic_ref<uintptr_t>(*addr).store(val, std::memory_order_release);
    };

    inline int read(uintptr_t *addr) override {
        Orec *O = get_orec(addr);

        /* check if orec for addr is taken and is not held by us */
        if ((O->is_locked()) && (orecs.count(O) == 0)) {
            abort();
            return -1;
        }

        /* else read value */
        uintptr_t val = std::atomic_ref<uintptr_t>(*addr).load(std::memory_order_acquire);

        reads.insert(O);
        return val;
    };

    inline void commit() override {
        if (writes.empty()) {
            reads.clear();
            return;
        }

        for (auto O : reads) {
            if ((O->is_locked()) || (O->get_owner() != id)) {
                abort();
                return;
            }
        }
        
        writes.clear();
        reads.clear();
        orecs.clear();

        TRACE("ETL " + std::to_string(id) + " COMMITTED");
    };

    inline void abort() override {
        writes.clear();
        reads.clear();
        orecs.clear();

        TRACE("ETL " + std::to_string(id) + " ABORTED");
    };

    TLEncounterModeTx() : id(id_gen++) {};

private:
    int id;

    std::unordered_map< uintptr_t *, uintptr_t > writes;
    std::set<Orec *> reads;
    std::set<Orec *> orecs;

};

#endif
