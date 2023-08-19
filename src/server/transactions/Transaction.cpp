//
// Created by vini84200 on 16/08/23.
//

#include "Transaction.h"
#include <filesystem>
#include <fstream>

Transaction::Transaction() {
    hasRollback    = false;
    is_committed_  = false;
    has_began_     = false;
    is_prepared_   = false;
}

void Transaction::begin() {
    has_began_ = true;
    printf("Transaction %d began (%s)\n", tid, toString().c_str());
    // Create tid~ file
    std::string tid_dir = ServerState::getTidPath();
    {
        std::ofstream tid_file(tid_dir + "~");
        tid_file << tid;
    }
}

bool Transaction::run() {
    if (has_began_) { throw "Transaction already began"; }
    begin();
    try {
        work_state_->setLastTid(tid);
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
        // Switch the tid file to the new one
        std::string tid_dir = ServerState::getTidPath();
        std::filesystem::remove(tid_dir);
        std::filesystem::rename(tid_dir + "~", tid_dir);

        commitHook();
        state_->set(*work_state_);
        printf("Transaction %d committed (%s)\n", tid, toString().c_str());
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
        if (hasRollback) { return;}
        hasRollback     = true;
        // Remove the tid~ file
        std::string tid_dir = ServerState::getTidPath();
        std::filesystem::remove(tid_dir + "~");

        rollbackHook();
        printf("Transaction %d rolled back (%s)\n", tid, toString().c_str());
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
        printf("Transaction %d prepared to commit. (%s)\n", tid, toString().c_str());
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

int Transaction::getTid() {
    return tid;
}

void Transaction::commitHook() {
}

void Transaction::rollbackHook() {
}
