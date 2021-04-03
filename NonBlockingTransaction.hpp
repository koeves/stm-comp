#ifndef NONBLOCKINGTRANSACTION_HPP
#define NONBLOCKINGTRANSACTION_HPP

#include "Transaction.hpp"

class NonblockingTransaction : Transaction {
public:

    void begin() override {};

    void write() override {};

    int read() override { return 0; };

    void commit() override {};

    void abort() override {};

private:
    Descriptor desc;
};

#endif