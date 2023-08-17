//
// Created by vini84200 on 8/16/23.
//

#include "TransactionManager.h"
#include "../common/RwLock.h"


TransactionManager::TransactionManager(bool isCoordinator,
                                       SharedData<ServerState> &state)

    : isCoordinator_(isCoordinator),
        state_(state) {

}

bool TransactionManager::doTransaction(Transaction& transaction) {
    if (isTransactionActive_) {
        return false;
    }
    activeTransaction_ = &transaction;
    WriteLock lock(state_);
    activeTransaction_->setState(&lock);
    initTransaction();
    activeTransaction_->run();
    if (isCoordinator_) {
        // Send the transaction to the other servers
        executeTransactionInNodes();
        // Wait for the votes
        if (!receiveVotes()) {
            // Rollback
            rollbackTransaction();
            return false;
        }
        // Commit
        commitTransaction();
    } else {
        // Send the vote
        if (transaction.isPrepared()) {
            // Send the vote
            sendVote(true);
        } else {
            // Send the vote
            sendVote(false);
            // Rollback
            rollbackTransaction();
            return false;
        }
        // Wait for the commit
        if (!receiveConfirmation()) {
            // Rollback
            rollbackTransaction();
            return false;
        }
        // Commit
        commitTransaction();

    }
    return true;
}

void TransactionManager::sendVote(bool vote) {
}

bool TransactionManager::receiveConfirmation() {
    return false; // TODO
}

void TransactionManager::initTransaction() {
    isTransactionActive_ = true;
    isTransactionPrepared_ = false;
    isTransactionCommitted_ = false;
    // Get id for the transaction
    int tid = state_.getData().nextTid();
    // Create the transaction
    activeTransaction_->setTid(tid);
}

void TransactionManager::executeTransactionInNodes() {
    // TODO: Send the transaction to the other servers
}

bool TransactionManager::receiveVotes() {
    return true;
    // TODO: Wait for the votes
}

void TransactionManager::commitTransaction() {
    activeTransaction_->commit();
    isTransactionActive_ = false;
}

void TransactionManager::rollbackTransaction() {
    activeTransaction_->forceRollback();
    isTransactionActive_ = false;
}

void TransactionManager::setIsCoordinator(bool b) {
    if (isTransactionActive_) {
        rollbackTransaction();
    }
    isCoordinator_ = b;
}
