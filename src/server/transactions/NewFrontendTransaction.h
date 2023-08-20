//
// Created by vini84200 on 8/20/23.
//

#ifndef FILESYNCERCLIENT_NEWFRONTENDTRANSACTION_H
#define FILESYNCERCLIENT_NEWFRONTENDTRANSACTION_H

#include "Transaction.h"

class NewFrontendTransaction : public Transaction {
public:
    NewFrontendTransaction() = default;
    NewFrontendTransaction(std::string hostname, int port);

    void serialize(TransactionMsg *out) override;
    void deserialize(const TransactionMsg *msg) override;
    std::string getTransactionName() override;
    std::string toString() override;

protected:
    void execute() override;

    std::string hostname;
    int port;
};


#endif//FILESYNCERCLIENT_NEWFRONTENDTRANSACTION_H
