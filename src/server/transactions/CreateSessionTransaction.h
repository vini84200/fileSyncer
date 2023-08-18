//
// Created by vini84200 on 8/16/23.
//

#ifndef FILESYNCERCLIENT_CREATESESSIONTRANSACTION_H
#define FILESYNCERCLIENT_CREATESESSIONTRANSACTION_H

#include "Transaction.h"
class CreateSessionTransaction : public Transaction {
public:
    CreateSessionTransaction(int sessionID, std::string username);
    CreateSessionTransaction() = default;
protected:
    void execute() override;

public:
    void *serialize(TransactionMsg *out) override;
    void deserialize(const TransactionMsg *msg) override;
    std::string getTransactionName() override;
    std::string toString() override;

protected:
    int sessionID;
    std::string username;
    int getTid();
};


#endif//FILESYNCERCLIENT_CREATESESSIONTRANSACTION_H
