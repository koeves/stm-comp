#ifndef NONBLOCKINGTRANSACTION_HPP
#define NONBLOCKINGTRANSACTION_HPP

#include "Transaction.hpp"

class NonBlockingTransaction : Transaction {
public:

    void begin() override {};

    void write() override {};

    void read() override { std::cout << "nonblocking transaction" << '\n'; };

    void commit() override {};

    void abort() override {};
};

#endif