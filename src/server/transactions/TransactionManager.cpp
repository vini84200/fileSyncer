//
// Created by vini84200 on 8/16/23.
//

#include "TransactionManager.h"
#include "../../common/utils.h"
#include "../Server.h"

TransactionManager::TransactionManager(bool isCoordinator,
                                       Server *server,
                                       SharedData<ServerState> &state)

    : isCoordinator_(isCoordinator), state_(state), server(server) {
}

bool TransactionManager::doTransaction(Transaction &transaction) {
    if (!isCoordinator_) {
        printf("WARN: Tried to execute a transaction in a "
               "non-coordinator server\n");
        return false;
    }
    WriteLock lock(state_);
    if (isTransactionActive_) { return false; }
    activeTransaction_   = &transaction;
    isTransactionActive_ = true;
    activeTransaction_->setState(&lock);
    initTransaction();
    printf("TransactionManager::doTransaction: lock acquired for "
           "transaction %d\n",
           activeTransaction_->getTid());
    startVoting();
    // Send the transaction to the other servers
    printf("TransactionManager::doTransaction: sending transaction "
           "to other servers\n");
    executeTransactionInNodes();
    // Run the transaction in this server
    printf("TransactionManager::doTransaction: running transaction "
           "in this server\n");
    activeTransaction_->run();
    // Cast coordinator vote
    printf("TransactionManager::doTransaction: casting coordinator "
           "vote\n");
    addCoordinatorVote(activeTransaction_->isPrepared());

    // Wait for the votes
    printf("TransactionManager::doTransaction: waiting for voting to "
           "end\n");
    bool result = waitEndVoting();// This is a blocking call

    // Send the result to the other servers
    printf("TransactionManager::doTransaction: sending result to "
           "other servers\n");
    sendResult(result, activeTransaction_->getTid());

    printf("TransactionManager::doTransaction: result sent\n");
    if (result) {
        // Commit
        printf("TransactionManager::doTransaction: committing "
               "transaction %d\n",
               activeTransaction_->getTid());
        commitTransaction();
        return true;
    } else {
        // Rollback
        printf("TransactionManager::doTransaction: rolling back "
               "transaction %d\n",
               activeTransaction_->getTid());
        rollbackTransaction();
        return false;
    }
}

void TransactionManager::sendVote(bool vote, int tid) {
    // Send a vote to the coordinator
    printf("Sending vote %s for transaction %d\n",
           vote ? "COMMIT" : "ROLLBACK", tid);
    Connection<TransactionOuterMsg, TransactionOuterMsg> c(
            server->getCoordinator().getTransactionConnectionArgs());
    printf("sendVote: connection created\n");
    printf("sendVote: connection status: %d\n",
           c.getConnectionState());

    TransactionOuterMsg msg;
    msg.set_type(TransactionOuterType::TRANSACTION_VOTE);
    msg.mutable_vote()->set_transaction_id(tid);
    msg.mutable_vote()->set_vote(vote ? TransactionVote::COMMIT
                                      : TransactionVote::ROLLBACK);
    c.sendRequest(msg);
    c.setTimout(2000);// 1 second timeout
    auto resp = c.receiveResponse();
    if (!resp.has_value()) {
        printf("WARN: Could not receive ack from coordinator for "
               "vote of transaction %d\n",
               tid);
        server->startElection();
    } else {
        auto header = resp.value().first;
        auto msg    = resp.value().second;
        if (msg.type() != TransactionOuterType::TRANSACTION_OK_ACK) {
            printf("WARN: Received an invalid ack from coordinator "
                   "for "
                   "vote of transaction %d\n",
                   tid);
        }
        // Everything is ok
    }
}

void TransactionManager::initTransaction() {
    isTransactionActive_ = true;
    // Get id for the transaction
    int tid = ++lastTid;
    // Create the transaction
    activeTransaction_->setTid(tid);
}

void TransactionManager::executeTransactionInNodes() {
    std::vector<Replica *> servers = server->getActiveServers();
    std::vector<Connection<TransactionOuterMsg, TransactionOuterMsg>>
            connections;

    TransactionOuterMsg msg;
    activeTransaction_->serialize(msg.mutable_transaction());
    for (auto &replica: servers) {
        Connection c = connections.emplace_back(
                replica->getTransactionConnectionArgs());
        msg.set_type(TransactionOuterType::TRANSACTION);
        printf("Sending transaction %d to server %d with socket %d\n",
               activeTransaction_->getTid(), replica->getId(),
               c.getSocket());
        bool sent = c.sendRequest(msg);
        if (!sent) {
            printf("WARN: Could not send transaction %d to server "
                   "%d\n",
                   activeTransaction_->getTid(), replica->getId());
        } else {
            printf("Transaction %d sent to server %d\n",
                   activeTransaction_->getTid(), replica->getId());
        }
    }
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
    votingTotal  = servers.size() + 1;// +1 for the coordinator
    votes        = 0;
}

void TransactionManager::addCoordinatorVote(bool vote) {
    printf("Adding coordinator vote %s for transaction %d\n",
           vote ? "COMMIT" : "ROLLBACK",
           activeTransaction_->getTid());
    addVote(vote, activeTransaction_->getTid());
}

bool TransactionManager::waitEndVoting() {
    printf("Waiting for voting for transaction %d\n",
           activeTransaction_->getTid());
    pthread_mutex_lock(&votes_mutex);
    while (votes < votingTotal && votes != -1) {
        printf("Waiting for voting for transaction %d (%d/%d)\n",
               activeTransaction_->getTid(), votes, votingTotal);
        pthread_cond_wait(&votes_cond, &votes_mutex);
    }
    bool result = votes == votingTotal;
    pthread_mutex_unlock(&votes_mutex);
    printf("Voting ended with result %s (%d/%d)\n",
           result ? "COMMIT" : "ROLLBACK", votes, votingTotal);
    return result;
}

void TransactionManager::addVote(bool vote, int tid) {
    if (tid != activeTransaction_->getTid()) {
        printf("WARN: Tried to add a vote for a transaction that is "
               "not "
               "the active one\n");
        return;
    }
    printf("Adding vote %s for transaction %d\n",
           vote ? "COMMIT" : "ROLLBACK", tid);
    if (vote) {
        pthread_mutex_lock(&votes_mutex);
        if (votes == -1) {
            // The voting was already finished
            pthread_mutex_unlock(&votes_mutex);
            return;
        }
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
    if (t_id != activeTransaction_->getTid()) {
        printf("WARN: Tried to receive a result for a transaction "
               "that is not "
               "the active one\n");
        return;
    }
    pthread_mutex_lock(&result_mutex);
    result    = res;
    hasResult = true;
    pthread_mutex_unlock(&result_mutex);
    pthread_cond_signal(&result_cond);
}

bool TransactionManager::receiveNewTransaction(
        Transaction &transaction) {
    if (isCoordinator_) {
        printf("WARN: Tried to receive a transaction in a "
               "coordinator server\n");
        return false;
    }

    // Get id for the transaction
    int tid = transaction.getTid();

    WriteLock lock(state_);
    if (isTransactionActive_) { return false; }
    printf("TransactionManager::receiveNewTransaction: lock acquired "
           "for transaction %d\n",
           tid);

    activeTransaction_   = &transaction;
    isTransactionActive_ = true;
    activeTransaction_->setState(&lock);
    printf("TransactionManager::receiveNewTransaction: transaction "
           "state set\n");

    printf("TransactionManager::receiveNewTransaction: running "
           "transaction in this server\n");
    activeTransaction_->run();

    // Cast vote
    printf("TransactionManager::receiveNewTransaction: casting "
           "vote\n");
    sendVote(activeTransaction_->isPrepared(), tid);

    // Wait for the result
    printf("TransactionManager::receiveNewTransaction: waiting for "
           "result\n");
    bool result = waitResult();
    printf("TransactionManager::receiveNewTransaction: result "
           "received\n");
    if (result) {
        // Commit
        printf("TransactionManager::receiveNewTransaction: "
               "committing transaction %d\n",
               tid);
        commitTransaction();
        return true;
    } else {
        // Rollback
        printf("TransactionManager::receiveNewTransaction: rolling "
               "back transaction %d\n",
               tid);
        rollbackTransaction();
        return false;
    }
}

void TransactionManager::sendResult(bool result, int tid) {
    std::vector<Connection<TransactionOuterMsg, TransactionOuterMsg>>
            connections;
    std::vector<Replica *> servers = server->getActiveServers();

    for (auto s: servers) {
        Connection<TransactionOuterMsg, TransactionOuterMsg> &c =
                connections.emplace_back(
                        s->getTransactionConnectionArgs());
        TransactionOuterMsg msg;
        msg.set_type(TransactionOuterType::TRANSACTION_RESULT);
        msg.mutable_result()->set_transaction_id(tid);
        msg.mutable_result()->set_result(
                result ? TransactionVote::COMMIT
                       : TransactionVote::ROLLBACK);

        c.sendRequest(msg);
    }
    // Get the acks
    for (auto &c: connections) {
        c.setTimout(1000);// 1 second timeout
        auto resp = c.receiveResponse();
        if (!resp.has_value()) {
            printf("WARN: Could not receive ack from server for "
                   "commit of transaction %d\n",
                   tid);
            // TODO: Find what server is down and remove it from the list
        } else {
            auto header = resp.value().first;
            auto msg    = resp.value().second;
            if (msg.type() !=
                TransactionOuterType::TRANSACTION_OK_ACK) {
                printf("WARN: Received an invalid ack from server "
                       "for "
                       "commit of transaction %d\n",
                       tid);
            }
            // Everything is ok
        }
    }
}

bool TransactionManager::waitResult() {
    pthread_mutex_lock(&result_mutex);
    while (!hasResult) {
        pthread_cond_wait(&result_cond, &result_mutex);
    }
    bool result1 = result;
    hasResult    = false;
    pthread_mutex_unlock(&result_mutex);
    return result1;
}
