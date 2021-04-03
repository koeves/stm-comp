#ifndef BLOCKINGTRANSACTION_HPP
#define BLOCKINGTRANSACTION_HPP

#include "Transaction.hpp"

class BlockingTransaction : Transaction {
public:

    void begin() override {};

    void write() override {};

    void read() override { std::cout << "blocking transaction" << '\n'; };

    void commit() override {};

    void abort() override {};
};

#endif