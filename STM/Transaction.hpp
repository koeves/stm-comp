#ifndef TRANSACTION_HPP
#define TRANSACTION_HPP

#include "Util.hpp"

/* 
 *  Interface of a transaction
 *       STM implementations need to override each method here    
 */

class Transaction {
public:
    virtual void begin() {};

    virtual bool write(uintptr_t *, uintptr_t) = 0;

    virtual uintptr_t read(uintptr_t *) = 0;

    virtual bool commit() = 0;

    virtual void abort() {};

private:
    int id;
};

#endif