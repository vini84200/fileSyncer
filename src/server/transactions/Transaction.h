//
// Created by vini84200 on 16/08/23.
//

#ifndef FILESYNCERCLIENT_TRANSACTION_H
#define FILESYNCERCLIENT_TRANSACTION_H

#include "../ServerState.h"

class Transaction {
private:
    ServerState **resultStatePtr;
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

public:
    explicit Transaction(ServerState **state);
    bool run();
    void forceRollback();
    bool commit();
};


#endif//FILESYNCERCLIENT_TRANSACTION_H
