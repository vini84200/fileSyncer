//
// Created by vini84200 on 16/08/23.
//

#include "Transaction.h"

Transaction::Transaction(ServerState **state) {
    work_state_    = new ServerState(**state);
    original_      = *state;
    resultStatePtr = state;
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
    execute();
    if (!hasRollback) { return commit(); }
    return false;
}

bool Transaction::commit() {
    if (!hasRollback) {
        if (!is_prepared_) { throw "Transaction not prepared"; }
        is_committed_ = true;
        // Set the result
        *resultStatePtr = work_state_;
        // Delete the original state
        delete original_;
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
        *resultStatePtr = original_;
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
