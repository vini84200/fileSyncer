//
// Created by vini84200 on 8/16/23.
//

#ifndef FILESYNCERCLIENT_TRANSACTIONMANAGER_H
#define FILESYNCERCLIENT_TRANSACTIONMANAGER_H

#include "../common/RwLock.h"
#include "transactions/Transaction.h"

class Server;
class TransactionManager {
public:
    TransactionManager(bool isCoordinator,
                       Server *server,
                       SharedData<ServerState> &state);
    /**
     * Executes a transaction, should be called by the coordinator.
     * This method is blocking and will only return when the transaction is
     * finished.
     * @param transaction The transaction to be executed
     * @return True if the transaction was successful, false otherwise
     */
    bool doTransaction(Transaction &transaction);

    /**
     * Receives a new transaction from another server. This will
     * send the vote to the coordinator and wait for the result. And
     * then execute the transaction in this server.
     * @param transaction The transaction to be executed
     * @return True if the transaction was successful, false otherwise
     */
    bool receiveNewTransaction(Transaction& transaction);

    bool isTransactionActive();
    void forceRollback();
    void setIsCoordinator(bool b);
    void addVote(bool vote, int tid);

    void receiveResult(bool res, int32_t t_id);

private:
    bool isTransactionActive_ = false;
    bool isCoordinator_;
    Transaction *activeTransaction_;
    SharedData<ServerState> &state_;
    int lastTid = 0;

private:
    void initTransaction();
    void executeTransactionInNodes();
    void commitTransaction();
    void rollbackTransaction();

    void sendVote(bool vote, int tid);
    void startVoting();
    void addCoordinatorVote(bool vote);
    bool waitEndVoting();

    Server *server;


    int votingTotal;
    int votes;
    pthread_mutex_t votes_mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t votes_cond;
    void sendResult(bool result, int tid);
    bool waitResult();
};


#endif//FILESYNCERCLIENT_TRANSACTIONMANAGER_H
