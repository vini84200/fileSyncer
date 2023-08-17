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
    TransactionMsg *serialize() override;
    void deserialize(TransactionMsg *msg) override;

protected:
    int sessionID;
    std::string username;
    int getTid();
};


#endif//FILESYNCERCLIENT_CREATESESSIONTRANSACTION_H
