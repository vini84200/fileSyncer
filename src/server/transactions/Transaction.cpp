//
// Created by vini84200 on 16/08/23.
//

#include "Transaction.h"
#include "../../common/RwLock.h"
#include "CreateUserTransaction.h"

Transaction::Transaction() {
    hasRollback    = false;
    is_committed_  = false;
    has_began_     = false;
}

void Transaction::begin() {
    has_began_ = true;
}

bool Transaction::run() {
    if (has_began_) { throw "Transaction already began"; }
    begin();
    try {
        execute();
    } catch (...) {
        rollback();
    }
    if (!hasRollback) { return prepareCommit(); }
    return false;
}

bool Transaction::commit() {
    if (!hasRollback) {
        if (!is_prepared_) { throw "Transaction not prepared"; }
        is_committed_ = true;
        // Set the result
        state_->set(*work_state_);
        // Delete the original state
        return true;
    }
    return false;
}

ServerState *Transaction::getWorkState() {
    return work_state_;
}

void Transaction::forceRollback() {
    if (!is_committed_) {
        rollback();
    } else {
        printf("WARN: Tried to rollback a committed transaction\n");
    }
}

void Transaction::rollback() {
    if (has_began_) {
        hasRollback     = true;
        // The original state is the one that is valid
        // Delete the work state
        delete work_state_;
    } else {
        printf("WARN: Tried to rollback a transaction that has not "
               "began\n");
    }
}

bool Transaction::prepareCommit() {

    if (work_state_->isValid()) {
        is_prepared_ = true;
        return true;
    }

    return false;
}

bool Transaction::isPrepared() {
    return is_prepared_;
}

void Transaction::setTid(int id) {
    tid = id;
}

int Transaction::setState(WriteLock<ServerState> *state) {
    state_ = state;
    original_ = &state->get();
    work_state_ = new ServerState(state->get());
    return 0;
}

TransactionStatus Transaction::getStatus() {
    if (is_committed_) {
        return TransactionStatus::COMMITTED;
    }
    if (hasRollback) {
        return TransactionStatus::ROLLBACK;
    }
    if (is_prepared_) {
        return TransactionStatus::PREPARED;
    }
    if (has_began_) {
        return TransactionStatus::RUNNING;
    }
    return TransactionStatus::NOT_RUNNING;
}