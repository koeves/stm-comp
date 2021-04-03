#ifndef TRANSACTION_HPP
#define TRANSACTION_HPP

#include "Descriptor.hpp"

class Transaction {
public:

    virtual void begin() {};

    virtual void write() {};

    virtual int read() { return 2; };

    virtual void commit() {};

    virtual void abort() {};

private:
    Descriptor desc;

};

#endif