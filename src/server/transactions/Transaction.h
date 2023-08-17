//
// Created by vini84200 on 16/08/23.
//

#ifndef FILESYNCERCLIENT_TRANSACTION_H
#define FILESYNCERCLIENT_TRANSACTION_H

#include "../../common/RwLock.h"
#include "../ServerState.h"

enum class TransactionStatus {
    COMMITTED,
    ROLLBACK,
    PREPARED,
    RUNNING,
    NOT_RUNNING
};

class Transaction {
private:
    WriteLock<ServerState> *state_;
    ServerState *original_;
    ServerState *work_state_;
    bool hasRollback;
    bool is_committed_;
    bool has_began_;
    bool is_prepared_;

protected:
    ServerState *getWorkState();

    void begin();
    virtual void execute() = 0;
    void rollback();
    bool prepareCommit();

    Transaction();

public:
    explicit Transaction(ServerState **state);
    bool run();
    void forceRollback();
    bool commit();
    bool isPrepared();
    void setTid(int id);
    int setState(ServerState **state);
    int tid;
    int setState(WriteLock<ServerState> *state);
    TransactionStatus getStatus();
    virtual TransactionMsg *serialize() = 0;
    virtual void deserialize(TransactionMsg *msg) = 0;
};


#endif//FILESYNCERCLIENT_TRANSACTION_H
