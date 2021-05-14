/* 
 *  Transaction interface
 *       each STM variation needs the following methods    
 */

#ifndef TRANSACTION_HPP
#define TRANSACTION_HPP

#include "Util.hpp"

template<class T = uintptr_t>
class Transaction {
public:
    virtual void begin()        {};
    virtual void write(T *, T) = 0;
    virtual T    read(T *)     = 0;
    virtual bool commit()      = 0;
    virtual void abort()        {};

private:
    int id;
};

#endif