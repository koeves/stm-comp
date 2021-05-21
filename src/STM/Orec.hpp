/* 
 * Ownership Record (ORec) implementation
 * 
 *      last bit is 0 : remaining bits are object's version number
 *                  1 : remaining bits are transaction's ID
 * 
 *      adapted from the proposed implementation in the book:
 *          Maurice Herlihy and Nir Shavit. 2012. 
 *          The Art of Multiprocessor Programming, Revised Reprint (1st. ed.). 
 *          Morgan Kaufmann Publishers Inc., San Francisco, CA, USA.
 */

#ifndef OREC_HPP
#define OREC_HPP

#include "../Utilities/Util.hpp"

class Orec {
public:
    Orec() : rec(1 << 1) {}  /* initial version is 1, lock is unlocked */

    inline uint64_t get_version() {
        if (is_locked()) {
            TRACE("OREC IS LOCKED BY TX " + std::to_string(owner_id));
            return 0; 
        }
        return rec >> 1;
    }

    /* if orec is locked, but Tx is its owner, get old version */
    inline uint64_t get_version(uint64_t id) {
        if (owner_id != id) {
            TRACE("HERE OREC IS LOCKED BY TX " + std::to_string(owner_id));
            return 0; 
        }

        return old_version;
    }

    inline uint64_t get_orec() {
        return rec.load();
    }

    inline int get_owner() {
        if (!is_locked()) {
            TRACE("OREC IS NOT OWNED BY ANY TX");
            return -1; 
        }
        return owner_id;
    }

    inline bool lock(uint64_t exp, uint64_t id) {
        uint64_t old = get_version();
        if (!old) return false;

        if (!rec.compare_exchange_strong(exp, (id << 1) | 1)) {
            TRACE("CANNOT ACQUIRE OREC");
            return false; 
        }

        /* rec holds now the id of the transaction and is locked */

        old_version = old;
        owner_id = id;

        return true;
    }

    inline void unlock() {
        rec.store((old_version + 1) << 1);
    }

    inline bool is_locked() {
        return rec & 1;
    }

    inline void print() {
#ifdef DEBUG
        std::cout << "OREC DETAILS\n" << "\tbits: ";
        print_bits();
        if (is_locked()) 
            std::cout << "\tLOCKED\n" << "\tOWNED BY TX " << owner_id << "\n\n"; 
        else
            std::cout << "\tUNLOCKED\n" << "\tVERSION " << get_version() << "\n\n";
#endif 
    }

private:
    std::atomic<uint64_t> rec;
    uint64_t old_version;
    uint64_t owner_id;

    void print_bits() {
        int i = 0;
        for (i = (sizeof(int) * 8) - 1; i >= 0; i--) {
            putchar(rec & (1u << i) ? '1' : '0');
        }
        printf("\n");
    }
};

#endif