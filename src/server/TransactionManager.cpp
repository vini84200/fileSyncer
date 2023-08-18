//
// Created by vini84200 on 8/16/23.
//

#include "TransactionManager.h"
#include "../common/RwLock.h"
#include "../server/Server.h"

TransactionManager::TransactionManager(bool isCoordinator,
                                       Server *server,
                                       SharedData<ServerState> &state)

    : isCoordinator_(isCoordinator), state_(state), server(server) {
}

bool TransactionManager::doTransaction(Transaction &transaction) {
    if (isTransactionActive_) { return false; }
    if (!isCoordinator_) {
        printf("WARN: Tried to execute a transaction in a "
               "non-coordinator server\n");
        return false;
    }
    activeTransaction_ = &transaction;
    WriteLock lock(state_);
    activeTransaction_->setState(&lock);
    initTransaction();
    startVoting();
    // Send the transaction to the other servers
    executeTransactionInNodes();
    // Run the transaction in this server
    activeTransaction_->run();
    // Cast coordinator vote
    addCoordinatorVote(activeTransaction_->isPrepared());

    // Wait for the votes
    bool result = waitEndVoting(); // This is a blocking call

    sendResult(result, activeTransaction_->getTid());

    if (result) {
        // Commit
        commitTransaction();
        return true;
    } else {
        // Rollback
        rollbackTransaction();
        return false;
    }
}

void TransactionManager::sendVote(bool vote, int tid) {
    //
}

void TransactionManager::initTransaction() {
    isTransactionActive_ = true;
    // Get id for the transaction
    int tid = ++lastTid;
    // Create the transaction
    activeTransaction_->setTid(tid);
}

void TransactionManager::executeTransactionInNodes() {
    // TODO: Send the transaction to the other servers
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
    if (isTransactionActive_) { rollbackTransaction(); }
    isCoordinator_ = b;
}

void TransactionManager::startVoting() {
    // Get the list of servers that will vote
    auto servers = server->getActiveServers();
    votingTotal = servers.size();
    votes = 0;
}

void TransactionManager::addCoordinatorVote(bool vote) {
    addVote(vote, activeTransaction_->getTid());
}

bool TransactionManager::waitEndVoting() {
    pthread_mutex_lock(&votes_mutex);
    while (votes < votingTotal && votes != -1) {
        pthread_cond_wait(&votes_cond, &votes_mutex);
    }
    bool result = votes == votingTotal;
    pthread_mutex_unlock(&votes_mutex);
    return result;
}

void TransactionManager::addVote(bool vote, int tid) {
    if (tid != activeTransaction_->getTid()) {
        printf("WARN: Tried to add a vote for a transaction that is not "
               "the active one\n");
        return;
    }
    if (vote) {
        pthread_mutex_lock(&votes_mutex);
        votes++;
        pthread_cond_signal(&votes_cond);
        pthread_mutex_unlock(&votes_mutex);
    } else {
        // The vote was negative, so we can stop the voting
        pthread_mutex_lock(&votes_mutex);
        votes = -1;
        pthread_cond_signal(&votes_cond);
        pthread_mutex_unlock(&votes_mutex);
    }
}

void TransactionManager::receiveResult(bool res, int32_t t_id) {
}

bool TransactionManager::receiveNewTransaction(
        Transaction &transaction) {
    if (isTransactionActive_) { return false; }
    if (isCoordinator_) {
        printf("WARN: Tried to receive a transaction in a "
               "coordinator server\n");
        return false;
    }

    activeTransaction_ = &transaction;
    isTransactionActive_ = true;
    // Get id for the transaction
    int tid = transaction.getTid();

    WriteLock lock(state_);
    activeTransaction_->setState(&lock);
    initTransaction();

    activeTransaction_->run();

    // Cast vote
    sendVote(activeTransaction_->isPrepared(), tid);

    bool result = waitResult();
    if (result) {
        // Commit
        commitTransaction();
        return true;
    } else {
        // Rollback
        rollbackTransaction();
        return false;
    }
}

void TransactionManager::sendResult(bool result, int tid) {
    for (auto s: server->getActiveServers()) {
        Connection conn(s, s->getTransactionPort());
    }
}

bool TransactionManager::waitResult() {
    return false;
}
