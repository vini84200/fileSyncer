//
// Created by vini84200 on 8/16/23.
//

#ifndef FILESYNCERCLIENT_TRANSACTIONMANAGER_H
#define FILESYNCERCLIENT_TRANSACTIONMANAGER_H

#include "../common/RwLock.h"
#include "transactions/Transaction.h"

class TransactionManager {
public:
    TransactionManager(bool isCoordinator,
                       SharedData<ServerState> &state);
    bool doTransaction(Transaction &transaction);
    bool isTransactionActive();
    void forceRollback();
    void setIsCoordinator(bool b);

private:
    bool isTransactionActive_;
    bool isTransactionPrepared_;
    bool isTransactionCommitted_;
    bool isCoordinator_;
    Transaction *activeTransaction_;
    SharedData<ServerState> &state_;

private:
    void initTransaction();
    void executeTransactionInNodes();
    bool receiveVotes();
    void commitTransaction();
    void rollbackTransaction();

    void sendVote(bool vote);
    bool receiveConfirmation();
};


#endif//FILESYNCERCLIENT_TRANSACTIONMANAGER_H
