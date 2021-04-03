#ifndef TRANSACTION_HPP
#define TRANSACTION_HPP

#include <iostream>

class Transaction {
public:

    virtual void begin() {};

    virtual void write() {};

    virtual void read() { std::cout << "generic transaction" << '\n'; };

    virtual void commit() {};

    virtual void abort() {};

};

#endif