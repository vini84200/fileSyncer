//
// Created by vini84200 on 8/19/23.
//

#ifndef FILESYNCERCLIENT_REMOVESESSIONTRANSACTION_H
#define FILESYNCERCLIENT_REMOVESESSIONTRANSACTION_H

#include "Transaction.h"

class RemoveSessionTransaction : public Transaction {
public:
    RemoveSessionTransaction(int32_t sessionID);
    RemoveSessionTransaction() = default;
    void *serialize(TransactionMsg *out) override;
    void deserialize(const TransactionMsg *msg) override;
    std::string getTransactionName() override;
    std::string toString() override;

protected:
    void execute() override;
    int32_t sessionID;
};


#endif//FILESYNCERCLIENT_REMOVESESSIONTRANSACTION_H
