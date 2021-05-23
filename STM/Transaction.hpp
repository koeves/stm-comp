/* 
 *  Transaction interface
 *       each STM variation needs the following methods    
 */

#ifndef TRANSACTION_HPP
#define TRANSACTION_HPP

#include "../Utilities/Util.hpp"

template<class T = uintptr_t>
class Transaction {
public:
    virtual void begin()       = 0;
    virtual void write(T *, T) = 0;
    virtual T    read(T *)     = 0;
    virtual bool commit()      = 0;
    virtual void abort()       = 0;

private:
    int id;
};

#endif