//
// Created by vini84200 on 8/16/23.
//

#ifndef FILESYNCERCLIENT_CREATEUSERTRANSACTION_H
#define FILESYNCERCLIENT_CREATEUSERTRANSACTION_H

#include "Transaction.h"

class CreateUserTransaction : public Transaction {
protected:
    void execute() override;

private:

public:
    CreateUserTransaction(
                          const std::string username,
                          const std::string password);
    CreateUserTransaction() = default;
    TransactionMsg *serialize() override;
    void deserialize(const TransactionMsg *msg) override;
    std::string getTransactionName() override;
    std::string toString() override;
    std::string username;
    std::string password;
};


#endif//FILESYNCERCLIENT_CREATEUSERTRANSACTION_H
