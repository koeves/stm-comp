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

    virtual void write(uintptr_t *, uintptr_t) {};

    virtual int read(uintptr_t *) = 0;

    virtual void commit() {};

    virtual void abort() {};

private:
    int id;
};

#endif