//
// Created by vini84200 on 16/08/23.
//

#ifndef FILESYNCERCLIENT_TRANSACTION_H
#define FILESYNCERCLIENT_TRANSACTION_H

#include "../../common/RwLock.h"
#include "../ServerState.h"
#include "proto/message.pb.h"

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

    virtual void commitHook();
    void rollback();
    virtual void rollbackHook();
    bool prepareCommit();

    Transaction();
    int tid;

public:
    bool run();
    void forceRollback();
    bool commit();
    bool isPrepared();
    void setTid(int id);
    int setState(WriteLock<ServerState> *state);
    TransactionStatus getStatus();
    virtual void serialize(TransactionMsg *out)        = 0;
    virtual void deserialize(const TransactionMsg *msg) = 0;
    virtual std::string getTransactionName() = 0;
    virtual std::string toString() = 0;
    int getTid();
};


#endif//FILESYNCERCLIENT_TRANSACTION_H
